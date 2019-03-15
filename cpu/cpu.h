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

#ifndef CPU_H_
#define CPU_H_

#include <alsa/asoundlib.h>

// defines
#define _MODCOUNT 32
#define _WAVECOUNT 32
#define _CHOICEMAX 16
#define _MULTITEMP 8
#define TableSize 4096
#define tabM 4095
#define tabF 4096.f

int cpuStart();
void cpuStop();

void cpuInit(float sampleRate);

void cpuProcess(unsigned nframes,
                float *bufferMixLeft, float *bufferMixRight,
                float *bufferAux1, float *bufferAux2,
                float **bufferVoices);

void cpuHandleMidi(snd_seq_event_t *ev);

int cpuReceiveChoice(int voice, int i, int value);
int cpuReceiveParameter(int voice, int i, float value);

#endif
