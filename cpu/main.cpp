/** Minicomputer
 * industrial grade digital synthesizer
 *
 * Copyright 2007,2008 Malte Steiner
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
// a way to compile it was:
//  gcc -o synthesizer synth2.c -ljack -ffast-math -O3 -march=k8 -mtune=k8 -funit-at-a-time -fpeel-loops -ftracer -funswitch-loops -llo -lasound

#include <jack/jack.h>
//#include <jack/midiport.h> // later we use the jack midi ports to, but not this time
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <lo/lo.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include "cpu.h"
// some common definitions
#include "../common.h"

jack_port_t *port[_MULTITEMP + 4];  // _multitemp * ports + 2 mix and 2 aux
char jackName[64] = "Minicomputer";  // signifier for audio and midiconnections, to be filled with OSC port number
snd_seq_t *open_seq();
snd_seq_t *seq_handle;
int npfd;
struct pollfd *pfd;
/* a flag which will be set by our signal handler when
 * it's time to exit */
int quit = 0;
jack_port_t *inbuf;
jack_client_t *client;

jack_nframes_t bufsize;
int done = 0;

// void midi_action(snd_seq_t *seq_handle);

/** \brief function to create Alsa Midiport
 *
 * \return handle on alsa seq
 */
snd_seq_t *open_seq()
{

    snd_seq_t *seq_handle;
    int portid;
// switch for blocking behaviour for experimenting which one is better
#ifdef _MIDIBLOCK
    if (snd_seq_open(&seq_handle, "hw", SND_SEQ_OPEN_INPUT, 0) < 0)
#else
    // open Alsa for input, nonblocking mode so it returns
    if (snd_seq_open(&seq_handle, "hw", SND_SEQ_OPEN_INPUT, SND_SEQ_NONBLOCK) < 0)
#endif
    {
        fprintf(stderr, "Error opening ALSA sequencer.\n");
        exit(1);
    }
    snd_seq_set_client_name(seq_handle, jackName);
    if ((portid = snd_seq_create_simple_port(seq_handle, jackName, SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
                                             SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
        fprintf(stderr, "Error creating sequencer port.\n");
        exit(1);
    }
    // filter out mididata which is not processed anyway, thanks to Karsten Wiese for the hint
    snd_seq_client_info_t *info;


    snd_seq_client_info_malloc(&info);
    if (snd_seq_get_client_info(seq_handle, info) != 0) {

        fprintf(stderr, "Error getting info for eventfiltersetup.\n");
        exit(1);
    }
    // its a bit strange, you set what you want to get, not what you want filtered out...
    // actually Karsten told me so but I got misguided by the term filter

    // snd_seq_client_info_event_filter_add	( info, SND_SEQ_EVENT_SYSEX);
    // snd_seq_client_info_event_filter_add	( info, SND_SEQ_EVENT_TICK);

    snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_NOTEON);
    snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_NOTEOFF);
    snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_PITCHBEND);
    snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_CONTROLLER);
    snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_CHANPRESS);

    // snd_seq_client_info_event_filter_add	( info, SND_SEQ_EVENT_CLOCK);
    // snd_seq_client_info_event_filter_add	( info, SND_SEQ_EVENT_SONGPOS);
    // snd_seq_client_info_event_filter_add	( info, SND_SEQ_EVENT_QFRAME);
    if (snd_seq_set_client_info(seq_handle, info) != 0) {

        fprintf(stderr, "Error setting info for eventfiltersetup.\n");
        exit(1);
    }
    snd_seq_client_info_free(info);

    return (seq_handle);
}

// some forward declarations
static inline void error(int num, const char *m, const char *path);
static inline int quit_handler(const char *path, const char *types, lo_arg **argv,
                               int argc, void *data, void *user_data);


/** @brief the audio processing function from jack
 *
 * this is the heart of the client. the process callback.
 * this will be called by jack every process cycle.
 * jack provides us with a buffer for every output port,
 * which we can happily write into.
 *
 * @param nframes
 * @param *arg pointer to additional arguments
 * @return integer 0 when everything is ok
 */
int process(jack_nframes_t nframes, void *arg)
{
    float *bufferMixLeft = (float *)jack_port_get_buffer(port[8], nframes);
    float *bufferMixRight = (float *)jack_port_get_buffer(port[9], nframes);
    float *bufferAux1 = (float *)jack_port_get_buffer(port[10], nframes);
    float *bufferAux2 = (float *)jack_port_get_buffer(port[11], nframes);

    float *bufferVoices[_MULTITEMP];
    for (unsigned currentvoice = 0; currentvoice < _MULTITEMP; ++currentvoice)  // for each voice
        bufferVoices[currentvoice] = (float *)jack_port_get_buffer(port[currentvoice], nframes);

    cpuProcess(nframes,
               bufferMixLeft, bufferMixRight,
               bufferAux1, bufferAux2,
               bufferVoices);
    return 0;  // thanks to Sean Bolton who was the first pointing to a bug when I returned 1
}


/** @brief the signal handler
 *
 * its used here only for quitting
 * @param the signal
 */
void signalled(int signal)
{
    quit = 1;
}

/** @brief handling the midi messages in an extra thread
 *
 * @param pointer/handle of alsa midi
 */
static void *midiprocessor(void *handle)
{
    struct sched_param param;
    int policy;
    snd_seq_t *seq_handle = (snd_seq_t *)handle;
    pthread_getschedparam(pthread_self(), &policy, &param);

    policy = SCHED_FIFO;
    param.sched_priority = 95;

    pthread_setschedparam(pthread_self(), policy, &param);

    /*
        if (poll(pfd, npfd, 100000) > 0)
        {
          midi_action(seq_handle);
        } */

    snd_seq_event_t *ev;
#ifdef _DEBUG
    printf("start\n");
    fflush(stdout);
#endif

#ifdef _MIDIBLOCK
    do {
#else
    while (quit == 0) {
#endif
        while ((snd_seq_event_input(seq_handle, &ev)) && (quit == 0)) {
            if (ev != NULL) {
                cpuHandleMidi(ev);
                snd_seq_free_event(ev);
                usleep(10);  // absolutly necessary, otherwise stream of mididata would block the whole computer, sleep for 1ms == 1000 microseconds
            }  // end of if
#ifdef _MIDIBLOCK
            usleep(1000);  // absolutly necessary, otherwise stream of mididata would block the whole computer, sleep for 1ms == 1000 microseconds
        }
    } while ((quit == 0) && (done == 0));  // doing it as long we are running was (snd_seq_event_input_pending(seq_handle, 0) > 0);
#else
            usleep(100);  // absolutly necessary, otherwise stream of mididata would block the whole computer, sleep for 1ms == 1000 microseconds
        }  // end of first while, emptying the seqdata queue

        usleep(2000);  // absolutly necessary, otherwise this thread would block the whole computer, sleep for 2ms == 2000 microseconds
    }  // end of while(quit==0)
#endif
    printf("midi thread stopped\n");
    fflush(stdout);
    return 0;  // its insisited on this although it should be a void function
}  // end of midiprocessor

/** @brief the classic c main function
 *
 * @param argc the amount of arguments we get from the commandline
 * @param pointer to array of the arguments
 * @return int the result, should be 0 if program terminates nicely
 */
int main(int argc, char **argv)
{
    printf("minicomputer version %s\n", _VERSION);
    // ------------------------ decide the oscport number -------------------------
    char OscPort[] = _OSCPORT;  // default value for OSC port
    char *oport = OscPort;  // pointer of the OSC port string
    int i;
    // process the arguments
    if (argc > 1) {
        for (i = 0; i < argc; ++i) {
            if (strcmp(argv[i], "-port") == 0)  // got a OSC port argument
            {
                ++i;  // looking for the next entry
                if (i < argc) {
                    int tport = atoi(argv[i]);
                    if (tport > 0)
                        oport = argv[i];  // overwrite the default for the OSCPort
                }
                else
                    break;  // we are through
            }
        }
    }


    printf("osc port %s\n", oport);
    sprintf(jackName, "Minicomputer%s", oport);  // store globally a unique name

    // ------------------------ midi init ---------------------------------
    pthread_t midithread;
    seq_handle = open_seq();
    npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
    pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
    snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);

    // create the thread and tell it to use Midi::work as thread function
    int err = pthread_create(&midithread, NULL, midiprocessor, seq_handle);

    // ------------------------ OSC Init ------------------------------------
    /* start a new server on port definied where oport points to */
    lo_server_thread st = lo_server_thread_new(oport, error);

    cpuInstallOscMethods(st);

    /* add method that will match the path Minicomputer/quit with one integer */
    lo_server_thread_add_method(st, "/Minicomputer/quit", "i", quit_handler, NULL);

    lo_server_thread_start(st);

    /* setup our signal handler signalled() above, so
     * we can exit cleanly (see end of main()) */
    signal(SIGINT, signalled);

    /* naturally we need to become a jack client
     * prefered with a unique name, so lets add the OSC port to it*/
    client = jack_client_open(jackName, JackNoStartServer, NULL);
    if (!client) {
        printf("couldn't connect to jack server. Either it's not running or "
               "the client name is already taken\n");
        exit(1);
    }

    /* we register the output ports and tell jack these are
     * terminal ports which means we don't
     * have any input ports from which we could somhow
     * feed our output */
    port[0] = jack_port_register(client, "output1", JACK_DEFAULT_AUDIO_TYPE,
                                 JackPortIsOutput | JackPortIsTerminal, 0);
    port[1] = jack_port_register(client, "output2", JACK_DEFAULT_AUDIO_TYPE,
                                 JackPortIsOutput | JackPortIsTerminal, 0);
    port[2] = jack_port_register(client, "output3", JACK_DEFAULT_AUDIO_TYPE,
                                 JackPortIsOutput | JackPortIsTerminal, 0);
    port[3] = jack_port_register(client, "output4", JACK_DEFAULT_AUDIO_TYPE,
                                 JackPortIsOutput | JackPortIsTerminal, 0);
    port[4] = jack_port_register(client, "output5", JACK_DEFAULT_AUDIO_TYPE,
                                 JackPortIsOutput | JackPortIsTerminal, 0);
    port[5] = jack_port_register(client, "output6", JACK_DEFAULT_AUDIO_TYPE,
                                 JackPortIsOutput | JackPortIsTerminal, 0);
    port[6] = jack_port_register(client, "output7", JACK_DEFAULT_AUDIO_TYPE,
                                 JackPortIsOutput | JackPortIsTerminal, 0);
    port[7] = jack_port_register(client, "output8", JACK_DEFAULT_AUDIO_TYPE,
                                 JackPortIsOutput | JackPortIsTerminal, 0);

    port[10] = jack_port_register(client, "aux out 1", JACK_DEFAULT_AUDIO_TYPE,
                                  JackPortIsOutput | JackPortIsTerminal, 0);
    port[11] = jack_port_register(client, "aux out 2", JACK_DEFAULT_AUDIO_TYPE,
                                  JackPortIsOutput | JackPortIsTerminal, 0);

    // would like to create mix ports last because qjackctrl tend to connect automatic the last ports
    port[8] = jack_port_register(client, "mix out left", JACK_DEFAULT_AUDIO_TYPE,
                                 JackPortIsOutput | JackPortIsTerminal, 0);
    port[9] = jack_port_register(client, "mix out right", JACK_DEFAULT_AUDIO_TYPE,
                                 JackPortIsOutput | JackPortIsTerminal, 0);

    // inbuf = jack_port_register(client, "in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    /* jack is callback based. That means we register
     * a callback function (see process() above)
     * which will then get called by jack once per process cycle */
    jack_set_process_callback(client, process, 0);
    bufsize = jack_get_buffer_size(client);

    cpuInit((float)jack_get_sample_rate(client));

    /* tell jack that we are ready to do our thing */
    jack_activate(client);

    /* wait until this app receives a SIGINT (i.e. press
     * ctrl-c in the terminal) see signalled() above */
    while (quit == 0) {
        // operate midi
        /* let's not waste cycles by busy waiting */
        sleep(1);
        // printf("quit:%i %i\n",quit,done);
    }
    /* so we shall quit, eh? ok, cleanup time. otherwise
     * jack would probably produce an xrun
     * on shutdown */
    jack_deactivate(client);

    /* shutdown cont. */
    jack_client_close(client);
#ifndef _MIDIBLOCK
    printf("wait for midithread\n");
    fflush(stdout);
    /* waiting for the midi thread to shutdown carefully */
    pthread_join(midithread, NULL);
#endif
    /* release Alsa Midi connection */
    snd_seq_close(seq_handle);

    /* done !! */
    printf("close minicomputer\n");
    fflush(stdout);
    return 0;
}
// ******************************************** OSC handling for editors ***********************
//!\name OSC routines
//!{
/** @brief OSC error handler
 *
 * @param num errornumber
 * @param pointer msg errormessage
 * @param pointer path where it occured
 */
static inline void error(int num, const char *msg, const char *path)
{
    printf("liblo server error %d in path %s: %s\n", num, path, msg);
    fflush(stdout);
}

/** message handler for quit messages
 *
 * @param pointer path osc path
 * @param pointer types
 * @param argv pointer to array of arguments
 * @param argc amount of arguments
 * @param pointer data
 * @param pointer user_data
 * @return int 0 if everything is ok, 1 means message is not fully handled
 */
static inline int quit_handler(const char *path, const char *types, lo_arg **argv,
                               int argc, void *data, void *user_data)
{
    done = 1;
    quit = 1;
    printf("about to shutdown minicomputer core\n");
    fflush(stdout);
    return 0;
}
//!}
