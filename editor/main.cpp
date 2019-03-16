/** Minicomputer
 * industrial grade digital synthesizer
 * editorsoftware
 * Copyright 2007,2008 Malte Steiner
 * This file is part of Minicomputer, which is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Minicomputer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <vector>
// thanks to Leslie P. Polzer pointing me out to include cstring and more for gcc 4.3 onwards
#include <cstdio>
#include <cstdlib>
//#include <unistd.h>
#include <cstring>
#include <cassert>
#include <pthread.h>
#include <jack/ringbuffer.h>
#include "../common.h"
#include "../cpu/cpu.h"
#include "Memory.h"
#include "syntheditor.h"
#include "communicate.h"
#include "utility.h"
#include "i18n.h"
static bool transmit = false;
// some common definitions

Memory Speicher;
UserInterface Schaltbrett;
static jack_ringbuffer_t *ringbuffer = nullptr;
static const double timeoutCpuRequests = 0.1;

/*
Fl_Double_Window* make_window() {
  Fl_Double_Window* w;
  { Fl_Double_Window* o = new Fl_Double_Window(475, 330);
    w = o;

     { Fl_Dial* knob1 = new Fl_Dial(25, 25, 25, 25,"f1");
      knob1->callback((Fl_Callback*)callback);
}
    {Fl_Dial* o =new Fl_Dial(65, 25, 25, 25, "f2");
    o->callback((Fl_Callback*)callback);
}
   {Fl_Dial* o = new Fl_Dial(105, 25, 25, 25, "f3");
o->callback((Fl_Callback*)callback);
}
    o->end();


}
  return w;
}*/
/*
void reloadSoundNames()
{
  int i;
  for (i = 0;i<8;++i)
  {
    Schaltbrett.soundchoice[i]->clear();
  }

  Speicher.load();

  for (i=0;i<512;i++)
  {
    Schaltbrett.soundchoice[0]->add(Speicher.getName(0,i).c_str());
    Schaltbrett.soundchoice[1]->add(Speicher.getName(0,i).c_str());
    Schaltbrett.soundchoice[2]->add(Speicher.getName(0,i).c_str());
    Schaltbrett.soundchoice[3]->add(Speicher.getName(0,i).c_str());
    Schaltbrett.soundchoice[4]->add(Speicher.getName(0,i).c_str());
    Schaltbrett.soundchoice[5]->add(Speicher.getName(0,i).c_str());
    Schaltbrett.soundchoice[6]->add(Speicher.getName(0,i).c_str());
    Schaltbrett.soundchoice[7]->add(Speicher.getName(0,i).c_str());
  }
}*/

static void cbEditorMultiChange(unsigned value)
{
    jack_ringbuffer_t *rb = ringbuffer;
    unsigned char msg[2] = {1, (unsigned char)value};
    if (jack_ringbuffer_write_space(rb) >= sizeof(msg))
        jack_ringbuffer_write(rb, (char *)msg, sizeof(msg));
}

static void cbEditorProgramChange(unsigned channel, unsigned value)
{
    jack_ringbuffer_t *rb = ringbuffer;
    unsigned char msg[3] = {2, (unsigned char)channel, (unsigned char)value};
    if (jack_ringbuffer_write_space(rb) >= sizeof(msg))
        jack_ringbuffer_write(rb, (char *)msg, sizeof(msg));
}

static void handleCpuRequests(void *)
{
    jack_ringbuffer_t *rb = ringbuffer;

    unsigned char buf[3];
    size_t count;

    while ((count = jack_ringbuffer_peek(rb, (char *)buf, sizeof(buf))) > 0) {
        switch (buf[0]) {
        case 1: // change multi
            if (count < 2) break;
            Schaltbrett.changeMulti(buf[1]);
            jack_ringbuffer_read_advance(rb, 2);
            break;

        case 2: // change program
            if (count < 3) break;
            Schaltbrett.changeSound(buf[1], buf[2]);
            jack_ringbuffer_read_advance(rb, 3);
            break;

        default:
            assert(false);
        }
    }

    Fl::repeat_timeout(timeoutCpuRequests, &handleCpuRequests);
}

static void usage()
{
    printf("usage: minicomputer [-port osc-port] [-bg color] [-fg color]\n");
}

/** @brief the main routine
 *
 * @param argc the amount of arguments
 * @param pointer to array with arguments
 * @return integer, 0 when terminated correctly
 */
int main(int argc, char **argv)
{
#ifdef MINICOMPUTER_I18N
    /* setup i18n */
    setlocale(LC_ALL, "");
    bindtextdomain("minicomputer", MINICOMPUTER_LOCALEDIR);
    textdomain("minicomputer");
#endif

    printf("minieditor version %s\n", _VERSION);

    // check color settings in arguments and add some if missing
    bool needcolor = true;  // true means user didnt give some so I need to take care myself

#ifdef MINICOMPUTER_OSC
    int oscport = 0;
#endif

    std::vector<char *> fl_argv;
    fl_argv.push_back(argv[0]);

    for (int argi = 1; argi < argc;) {
        char *arg = argv[argi++];
        if (!strcmp(arg, "-fg") || !strcmp(arg, "-bg")) {
            /* pass it to FLTK */
            needcolor = false;
            fl_argv.push_back(arg);
            arg = argv[argi++];
            if (!arg) { usage(); return 1; }
            fl_argv.push_back(arg);
        }
        else if (!strcmp(arg, "-port")) {
#ifdef MINICOMPUTER_OSC
            arg = argv[argi++];
            if (!arg) { usage(); return 1; }
            oscport = atoi(arg);;
#endif
        }
        else {
            usage();
            return 1;
        }
    }

    ringbuffer = jack_ringbuffer_create(1024);

    // ------------------------ create gui --------------
    Fenster *w = Schaltbrett.make_window();
    //
    // for (int i = 0;i<8;++i)
    //{
    // printf("bei %i\n",i);
    // fflush(stdout);
    // Schaltbrett.soundchoice[i]->clear();
    //}
    Speicher.load();
    // printf("und load...\n");
    /*
    for (int i=0;i<512;i++)
    {
      Schaltbrett.soundchoice[0]->add(Speicher.getName(0,i).c_str());
      Schaltbrett.soundchoice[1]->add(Speicher.getName(0,i).c_str());
      Schaltbrett.soundchoice[2]->add(Speicher.getName(0,i).c_str());
      Schaltbrett.soundchoice[3]->add(Speicher.getName(0,i).c_str());
      Schaltbrett.soundchoice[4]->add(Speicher.getName(0,i).c_str());
      Schaltbrett.soundchoice[5]->add(Speicher.getName(0,i).c_str());
      Schaltbrett.soundchoice[6]->add(Speicher.getName(0,i).c_str());
      Schaltbrett.soundchoice[7]->add(Speicher.getName(0,i).c_str());
    }*/
    Speicher.loadMulti();
    /*for (int i=0;i<128;i++)
    {
      Schaltbrett.multichoice->add(Speicher.multis[i].name);
    }*/
    // printf("weiter...\n");

    //------------------------- start engine -----------------------------
    cpuMultiChangeHook = &cbEditorMultiChange;
    cpuProgramChangeHook = &cbEditorProgramChange;

    if (cpuStart() != 0)
        return 1;

#ifdef MINICOMPUTER_OSC
    if (oscport >= 0)
        oscport = cpuListenOsc(oscport);
#endif

    if (needcolor)  // add the arguments in case they are needed
    {
        fl_argv.push_back((char *)"-bg");
        fl_argv.push_back((char *)"grey");
        fl_argv.push_back((char *)"-fg");
        fl_argv.push_back((char *)"black");
    }
    Fl::lock();

    fl_argv.push_back(nullptr);
    w->show((int)(fl_argv.size() - 1), fl_argv.data());

#ifdef MINICOMPUTER_OSC
    if (oscport > 0) {
        std::string caption = astrprintf("%s - %s", w->label(), cpuGetOscUrl());
        w->copy_label(caption.c_str());
    }
#endif

    /* an address to send messages to. sometimes it is better to let the server
     * pick a port number for you by passing nullptr as the last argument */

    Schaltbrett.changeMulti(0);  //load the first multi at startup

    Fl::add_timeout(timeoutCpuRequests, &handleCpuRequests);

    // lo_send(t, "/a/b/c/d", "f",10.f);
    int result = Fl::run();
    cpuStop();
    jack_ringbuffer_free(ringbuffer);
    return result;
}

void enableTransmit(bool enable)
{
    transmit = enable;
}

void sendParameter(int voicenumber, int parameter, float value)
{
    if (!transmit) return;
    //lo_send(t, "/Minicomputer", "iif", voicenumber, parameter, value);
    cpuReceiveParameter(voicenumber, parameter, value);
}

void sendChoice(int voicenumber, int choice, int value)
{
    if (!transmit) return;
    //lo_send(t, "/Minicomputer/choice", "iii", voicenumber, choice, value);
    cpuReceiveChoice(voicenumber, choice, value);
}
