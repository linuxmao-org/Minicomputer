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
#include <jack/midiport.h>
#include <string>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include "cpu.h"
// some common definitions
#include "../common.h"

static jack_client_t *client;
static jack_port_t *port[_MULTITEMP + 4];  // _multitemp * ports + 2 mix and 2 aux
static jack_port_t *aoutport[2];
static jack_port_t *midiport;

#ifdef MINICOMPUTER_OSC
#include <lo/lo.h>
static lo_server_thread oscServer = nullptr;
std::string oscServerUrl;
#endif

// void midi_action(snd_seq_t *seq_handle);


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
    void *bufferMidi = jack_port_get_buffer(midiport, nframes);
    jack_midi_event_t midiEvent;
    for (uint32_t midiIndex = 0; jack_midi_event_get(&midiEvent, bufferMidi, midiIndex) == 0; ++midiIndex) {
        cpuHandleMidi(midiEvent.buffer, midiEvent.size);
    }

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


/** @brief the classic c main function
 *
 * @param argc the amount of arguments we get from the commandline
 * @param pointer to array of the arguments
 * @return int the result, should be 0 if program terminates nicely
 */
int cpuStart()
{
    printf("minicomputer version %s\n", _VERSION);

    const char jackName[] = "Minicomputer";  // signifier for audio and midiconnections

    /* naturally we need to become a jack client */
    client = jack_client_open(jackName, JackNoStartServer, nullptr);
    if (!client) {
        printf("couldn't connect to jack server, is it running?\n");
        return 1;
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
    aoutport[0] = port[8];
    port[9] = jack_port_register(client, "mix out right", JACK_DEFAULT_AUDIO_TYPE,
                                 JackPortIsOutput | JackPortIsTerminal, 0);
    aoutport[1] = port[9];

    midiport = jack_port_register(client, "midi", JACK_DEFAULT_MIDI_TYPE,
                                  JackPortIsInput | JackPortIsTerminal, 0);

    // inbuf = jack_port_register(client, "in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    /* jack is callback based. That means we register
     * a callback function (see process() above)
     * which will then get called by jack once per process cycle */
    jack_set_process_callback(client, process, 0);
    //jack_nframes_t bufsize = jack_get_buffer_size(client);

    cpuInit((float)jack_get_sample_rate(client));

    /* tell jack that we are ready to do our thing */
    if (jack_activate(client) != 0) {
        jack_client_close(client);
        return 1;
    }

    return 0;
}

void cpuStop()
{
#ifdef MINICOMPUTER_OSC
    if (oscServer) {
        lo_server_thread_free(oscServer);
        oscServer = nullptr;
    }
#endif

    /* so we shall quit, eh? ok, cleanup time. otherwise
     * jack would probably produce an xrun
     * on shutdown */
    jack_deactivate(client);

    /* shutdown cont. */
    jack_client_close(client);

    /* done !! */
    printf("close minicomputer\n");
    fflush(stdout);
}

void cpuConnectSoundOutputs()
{
    const char **ports = jack_get_ports(client, nullptr, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput);
    if (!ports)
        return;

    // identify the default device (the first one)
    const char *dev = nullptr;
    size_t devlen = 0;

    if (ports[0]) {
        if (const char *separator = strchr(ports[0], ':')) {
            dev = ports[0];
            devlen = separator - dev;
        }
    }

    if (dev) {
        unsigned nports = 0;
        for (const char **p = ports, *port; nports < 2 && (port = *p); ++p) {
            bool is_of_device = strlen(port) > devlen && port[devlen] == ':' && !memcmp(port, dev, devlen);
            if (is_of_device)
                jack_connect(client, jack_port_name(aoutport[nports++]), port);
        }
    }

    jack_free(ports);
}

#ifdef MINICOMPUTER_OSC

static void oscError(int num, const char *msg, const char *path)
{
    fprintf(stderr, "OSC server error %d in path %s: %s\n", num, path, msg);
}

int cpuListenOsc(unsigned port)
{
    assert(!oscServer);

    oscServer = lo_server_thread_new(port ? std::to_string(port).c_str() : nullptr, &oscError);
    if (!oscServer)
        return -1;

    lo_server_thread_add_method(
        oscServer, "/Minicomputer/choice", "iii",
        +[](const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
             { return cpuReceiveChoice(argv[0]->i, argv[1]->i, argv[2]->i); }, nullptr);

    lo_server_thread_add_method(
        oscServer, "/Minicomputer", "iif",
        +[](const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
             { return cpuReceiveParameter(argv[0]->i, argv[1]->i, argv[2]->f); }, nullptr);

    if (lo_server_thread_start(oscServer) != 0) {
        lo_server_thread_free(oscServer);
        oscServer = nullptr;
    }

    char *url = lo_server_thread_get_url(oscServer);
    oscServerUrl.assign(url);
    free(url);

    return lo_server_thread_get_port(oscServer);
}

const char *cpuGetOscUrl()
{
    return oscServerUrl.c_str();
}
#endif
