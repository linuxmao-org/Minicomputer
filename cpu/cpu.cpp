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

#include <math.h>
#include "cpu.h"
#include "../common.h"

// variables
float delayBuffer[_MULTITEMP][96000] __attribute__((aligned(16)));
float table[_WAVECOUNT][TableSize] __attribute__((aligned(16)));
float parameter[_MULTITEMP][_PARACOUNT] __attribute__((aligned(16)));
float modulator[_MULTITEMP][_MODCOUNT] __attribute__((aligned(16)));
float midi2freq[128], midif[_MULTITEMP] __attribute__((aligned(16)));
float EG[_MULTITEMP][8][8] __attribute__((aligned(16)));  // 7 8
float EGFaktor[_MULTITEMP][8] __attribute__((aligned(16)));
float phase[_MULTITEMP][4] __attribute__((aligned(16)));  //=0.f;
unsigned int choice[_MULTITEMP][_CHOICEMAX] __attribute__((aligned(16)));
int EGrepeat[_MULTITEMP][8] __attribute__((aligned(16)));
unsigned int EGtrigger[_MULTITEMP][8] __attribute__((aligned(16)));
unsigned int EGstate[_MULTITEMP][8] __attribute__((aligned(16)));
float high[_MULTITEMP][4], band[_MULTITEMP][4], low[_MULTITEMP][4],
    f[_MULTITEMP][4], q[_MULTITEMP][4], v[_MULTITEMP][4], faktor[_MULTITEMP][4];
unsigned int lastnote[_MULTITEMP];
int delayI[_MULTITEMP], delayJ[_MULTITEMP];

float temp = 0.f, lfo;
float sampleRate = 48000.0f;  // only default, going to be overriden by the actual, taken from jack
float tabX = 4096.f / 48000.0f;
float srate = 3.145f / 48000.f;
float srDivisor = 1.f / 48000.f * 100000.f;
int delayBufferSize = 0, maxDelayBufferSize = 0, maxDelayTime = 0;
static const float anti_denormal = 1e-20;  // magic number to get rid of denormalizing

// I experiment with optimization
#ifdef _VECTOR
typedef float v4sf __attribute__((vector_size(16), aligned(16)));  //((mode(V4SF))); // vector of four single floats
union f4vector {
    v4sf v;  // __attribute__((aligned (16)));
    float f[4];  // __attribute__((aligned (16)));
};
#endif

/* inlined manually
static inline float Oscillator(float frequency,int wave,float *phase)
{
    int i = (int) *phase ;// float to int, cost some cycles

    i=i&tabM;//i%=TableSize;
    //if (i>tabM) i=tabM;
    if (i<0) i=tabM;
    *phase += tabX * frequency;

    if(*phase >= tabF)
    {
         *phase -= tabF;
        // if (*phase>=tabF) *phase = 0; //just in case of extreme fm
    }


        if(*phase < 0.f)
                {
                    *phase += tabF;
            //	if(*phase < 0.f) *phase = tabF-1;
                }
        return table[wave][i] ;
}
*/
// inline float filter (float input,float f, float q)
//{
//
//
//}
/**
 * start the envelope generator
 * called by a note on event for that voice
 * @param the voice number
 * @param the number of envelope generator
 */

static inline void egStart(const unsigned int voice, const unsigned int number)
{
    EGtrigger[voice][number] = 1;
    EG[voice][number][0] = 1.f;  // triggerd
    EG[voice][number][5] = 1.f;  // target
    EG[voice][number][7] = 0.0f;  // state
    EGstate[voice][number] = 0;  // state
    EGFaktor[voice][number] = 0.f;
    // printf("start %i", voice);
}
/**
 * set the envelope to release mode
 * should be called by a related noteoff event
 * @param the voice number
 * @param the number of envelope generator
 */
static inline void egStop(const unsigned int voice, const unsigned int number)
{
    // if (EGrepeat[voice][number] == 0)
    EGtrigger[voice][number] = 0;  // triggerd
    EGstate[voice][number] = 0;  // target
    // printf("stop %i", voice);
}
/**
 * calculate the envelope, done in audiorate to avoide zippernoise
 * @param the voice number
 * @param the number of envelope generator
 */
static inline float egCalc(const unsigned int voice, const unsigned int number)
{
    /* EG[x] x:
     * 0 = trigger
     * 1 = attack
     * 2 = decay
     * 3 = sustain
     * 4 = release
     * 5 = target
     * 6 = state
     */
    if (EGtrigger[voice][number] != 1) {
        int i = EGstate[voice][number];
        if (i == 1) {  // attack
            if (EGFaktor[voice][number] < 1.00f)
                EGFaktor[voice][number] += 0.002f;

            EG[voice][number][6] +=
                EG[voice][number][1] * srDivisor * EGFaktor[voice][number];

            if (EG[voice][number][6] >= 1.0f)  // Attackphase is finished
            {
                EG[voice][number][6] = 1.0f;
                EGstate[voice][number] = 2;
                EGFaktor[voice][number] = 1.f;  // triggerd
            }
        }
        else if (i == 2) {  // decay
            if (EG[voice][number][6] > EG[voice][number][3]) {
                EG[voice][number][6] -=
                    EG[voice][number][2] * srDivisor * EGFaktor[voice][number];
            }
            else {
                if (EGrepeat[voice][number] == 0) {
                    EGstate[voice][number] = 3;  // stay on sustain
                }
                else {
                    EGFaktor[voice][number] = 1.f;  // triggerd
                    egStop(voice, number);  // continue to release
                }
            }
            // what happens if sustain = 0? envelope should go in stop mode when decay reached ground
            if (EG[voice][number][6] < 0.0f) {
                EG[voice][number][6] = 0.0f;
                if (EGrepeat[voice][number] == 0) {
                    EGstate[voice][number] = 4;  // released
                }
                else {
                    egStart(voice, number);  // repeat
                }
            }

        }  // end of decay
        else if ((i == 0) && (EG[voice][number][6] > 0.0f)) {
            /* release */

            if (EGFaktor[voice][number] > 0.025f)
                EGFaktor[voice][number] -= 0.002f;
            EG[voice][number][6] -= EG[voice][number][4] * srDivisor *
                                    EGFaktor[voice][number];  //*EG[number][6];

            if (EG[voice][number][6] < 0.0f) {
                EG[voice][number][6] = 0.0f;
                if (EGrepeat[voice][number] == 0) {
                    EGstate[voice][number] = 4;  // released
                }
                else {
                    egStart(voice, number);  // repeat
                }
            }
        }
        //		if (EG[number][5] == 1.0f){
        //		    /* attack */
        //
        //		    EG[number][6] += EG[number][1]*(1.0f - EG[number][6]);
        //		    EG[number][5] = (EG[number][6] > 1.0f) ? EG[number][3] : 1.0f;
        //		}
        //		else if ((EG[number][5] == 0.0f) && (EG[number][6]>0.0f))
        //		{
        //		    /* release */
        //
        //		    EG[number][6] -= EG[number][4];//*EG[number][6];
        //		    if (EG[number][6]<0.0f) EG[number][6]=0.0f;
        //		}
        //		else{
        //		    /* decay */
        //		    EG[number][6] -= EG[number][2]*(EG[number][6]-EG[number][3]);
        //		}
    }
    else {
        //	if (EGtrigger[voice][number] == 1) // declick ramp down processing
        //	{

        /*	EG[voice][number][6] -= 0.005f;
            if (EG[voice][number][6]<0.0f)
            {
                if  (EG[voice][number][7]< EG[voice][number][6])
                {
                 EG[voice][number][7] += EG[voice][number][1]*(1.0f - EG[voice][number][7]);
                }
                else
                {*/
        EGtrigger[voice][number] = 0;
        //	EGstate[voice][number] = 1;
        /*	    }

            }*/
        //	}
        //	else if (EG[voice][number][0] == 2.f) // actual start
        //	{
        //		EG[voice][number][0] = 0.f;

        EG[voice][number][0] = 1.f;  // triggerd
        EGstate[voice][number] = 1;  // target
                                     // EG[voice][number][6] = 0.0f;// state
        //	}
    }
    return EG[voice][number][6];
}
// float d0,d1,d2,c1;

/** @brief initialization
 *
 * preparing for instance the waveforms
 */
void init()
{
    unsigned int i, k;
    for (k = 0; k < _MULTITEMP; k++)  // k is here the voice number
    {
        for (i = 0; i < 8; i++)  // i is the number of envelope
        {
            EG[k][i][1] = 0.01f;
            EG[k][i][2] = 0.01f;
            EG[k][i][3] = 1.0f;
            EG[k][i][4] = 0.0001f;
            EGtrigger[k][i] = 0;

            EGrepeat[k][i] = 0;
            EGstate[k][i] = 4;  // released
        }
        parameter[k][30] = 100.f;
        parameter[k][31] = 0.5f;
        parameter[k][33] = 100.f;
        parameter[k][34] = 0.5f;
        parameter[k][40] = 100.f;
        parameter[k][41] = 0.5f;
        parameter[k][43] = 100.f;
        parameter[k][44] = 0.5f;
        parameter[k][50] = 100.f;
        parameter[k][51] = 0.5f;
        parameter[k][53] = 100.f;
        parameter[k][54] = 0.5f;
        modulator[k][0] = 0.f;  // the none modulator, doing nothing
        for (i = 0; i < 3; ++i) {
            low[k][i] = 0;
            high[k][i] = 0;
        }
    }
    float PI = 3.145;
    float increment = (float)(PI * 2) / (float)TableSize;
    float x = 0.0f;
    float tri = -0.9f;
    // calculate wavetables
    for (i = 0; i < TableSize; i++) {
        table[0][i] = (float)((float)sin(x + ((float)2.0f * (float)PI)));
        x += increment;
        table[1][i] = (float)i / tabF * 2.f - 1.f;  // ramp up

        table[2][i] = 0.9f - (i / tabF * 1.8f - 0.5f);  // tabF-((float)i/tabF*2.f-1.f);//ramp down

        if (i < TableSize / 2) {
            tri += (float)1.f / TableSize * 3.f;
            table[3][i] = tri;
            table[4][i] = 0.9f;
        }
        else {
            tri -= (float)1.f / TableSize * 3.f;
            table[3][i] = tri;
            table[4][i] = -0.9f;
        }
        table[5][i] = 0.f;
        table[6][i] = 0.f;
        if (i % 2 == 0)
            table[7][i] = 0.9f;
        else
            table[7][i] = -0.9f;

        table[8][i] =
            (float)(((float)sin(x + ((float)2.0f * (float)PI))) +
                    ((float)sin(x * 2.f + ((float)2.0f * (float)PI))) +
                    ((float)sin(x * 3.f + ((float)2.0f * (float)PI))) +
                    ((float)sin(x * 4.f + ((float)2.0f * (float)PI))) * 0.9f +
                    ((float)sin(x * 5.f + ((float)2.0f * (float)PI))) * 0.8f +
                    ((float)sin(x * 6.f + ((float)2.0f * (float)PI))) * 0.7f +
                    ((float)sin(x * 7.f + ((float)2.0f * (float)PI))) * 0.6f +
                    ((float)sin(x * 8.f + ((float)2.0f * (float)PI))) * 0.5f) /
            8.0f;

        table[9][i] =
            (float)(((float)sin(x + ((float)2.0f * (float)PI))) +
                    ((float)sin(x * 3.f + ((float)2.0f * (float)PI))) +
                    ((float)sin(x * 5.f + ((float)2.0f * (float)PI))) +
                    ((float)sin(x * 7.f + ((float)2.0f * (float)PI))) * 0.9f +
                    ((float)sin(x * 9.f + ((float)2.0f * (float)PI))) * 0.8f +
                    ((float)sin(x * 11.f + ((float)2.0f * (float)PI))) * 0.7f +
                    ((float)sin(x * 13.f + ((float)2.0f * (float)PI))) * 0.6f +
                    ((float)sin(x * 15.f + ((float)2.0f * (float)PI))) * 0.5f) /
            8.0f;

        table[10][i] =
            (float)(sin((double)i / (double)TableSize + (sin((double)i * 4)) / 2)) * 0.5;
        table[11][i] =
            (float)(sin((double)i / (double)TableSize * (sin((double)i * 6) / 4))) * 2.;
        table[12][i] = (float)(sin((double)i * (sin((double)i * 1.3) / 50)));
        table[13][i] = (float)(sin((double)i * (sin((double)i * 1.3) / 5)));
        table[14][i] = (float)sin((double)i * 0.5 * (cos((double)i * 4) / 50));
        table[15][i] = (float)sin((double)i * 0.5 + (sin((double)i * 14) / 2));
        table[16][i] = (float)cos((double)i * 2 * (sin((double)i * 34) / 400));
        table[17][i] = (float)cos((double)i * 4 * ((double)table[7][i] / 150));

        // printf("%f ",table[17][i]);
    }

    table[5][0] = -0.9f;
    table[5][1] = 0.9f;

    table[6][0] = -0.2f;
    table[6][1] = -0.6f;
    table[6][2] = -0.9f;
    table[6][3] = -0.6f;
    table[6][4] = -0.2f;
    table[6][5] = 0.2f;
    table[6][6] = 0.6f;
    table[6][7] = 0.9f;
    table[6][8] = 0.6f;
    table[6][9] = 0.2f;
    /*
        float pi = 3.145f;
        float oscfreq = 1000.0; //%Oscillator frequency in Hz
        c1 = 2 * cos(2 * pi * oscfreq / Fs);
        //Initialize the unit delays
        d1 = sin(2 * pi * oscfreq / Fs);
        d2 = 0;*/


    // miditable for notes to frequency
    for (i = 0; i < 128; ++i)
        midi2freq[i] = 8.1758f * pow(2, (i / 12.f));

}  // end of initialization

void cpuInit(float sampleRate)
{
    init();

    // handling the sampling frequency
    ::sampleRate = sampleRate;
    tabX = 4096.f / sampleRate;
    srate = 3.145f / sampleRate;
    srDivisor = 1.f / sampleRate * 100000.f;
    // depending on it the delaybuffer
    maxDelayTime = (int)sampleRate;
    delayBufferSize = maxDelayTime * 2;
    // generate the delaybuffers for each voice
    int k;
    for (k = 0; k < _MULTITEMP; ++k) {
        // float dbuffer[delayBufferSize];
        // delayBuffer[k]=dbuffer;
        delayI[k] = 0;
        delayJ[k] = 0;
    }
#ifdef _DEBUG
    printf("bsize:%d %d\n", delayBufferSize, maxDelayTime);
#endif
}

void cpuProcess(unsigned nframes,
                float *bufferMixLeft, float *bufferMixRight,
                float *bufferAux1, float *bufferAux2,
                float **bufferVoices)
{
    float tf, tf1, tf2, tf3, ta1, ta2, ta3, morph, mo, mf, result, tdelay, clib1, clib2;
    float osc1, osc2, delayMod;

    // an union for a nice float to int casting trick which should be fast
    typedef union {
        int i;
        float f;
    } INTORFLOAT;
    INTORFLOAT P1 __attribute__((aligned(16)));
    INTORFLOAT P2 __attribute__((aligned(16)));
    INTORFLOAT P3 __attribute__((aligned(16)));
    INTORFLOAT bias;  // the magic number
    bias.i = (23 + 127) << 23;  // generating the magic number

    int iP1 = 0, iP2 = 0, iP3 = 0;

#ifdef _VECTOR
    union f4vector g __attribute__((aligned(16)));
    union f4vector h __attribute__((aligned(16)));
    union f4vector i __attribute__((aligned(16)));
    union f4vector j __attribute__((aligned(16)));
    g.f[1] = 2.f;
    g.f[2] = 2.f;
    g.f[3] = 2.f;  // first entry differs always
    // g.f[1] = 2.f*srate; g.f[2] = 2.f*srate; g.f[3] = 2.f*srate; // first entry differs always
    i.f[0] = 1.f;
    i.f[1] = srate;
    i.f[2] = srate;
    i.f[3] = srate;
    h.f[0] = 1.f;
    h.f[1] = 0.1472725f;
    h.f[2] = 0.1472725f;
    h.f[3] = 0.1472725f;
#endif
    // functions for including JACK Midi later, commented out for now
    /*jack_midi_port_info_t* info;
        void* buf;
        jack_midi_event_t ev;


        buf = jack_port_get_buffer(inbuf, bufsize);
        info = jack_midi_port_get_info(buf, bufsize);
        for(index=0; index<info->event_count; ++index)
        {
            jack_midi_event_get(&ev, buf, index, nframes);
        }
    */

    /* main loop */
    register unsigned int index;
    for (index = 0; index < nframes; ++index) {
        /* this function returns a pointer to the buffer where
         * we can write our frames samples */


        bufferMixLeft[index] = 0.f;
        bufferMixRight[index] = 0.f;
        bufferAux1[index] = 0.f;
        bufferAux2[index] = 0.f;
        /*
         * I dont know if its better to separate the calculation blocks, so I
         * try it first calculating the envelopes
         */
        register unsigned int currentvoice;
        for (currentvoice = 0; currentvoice < _MULTITEMP; ++currentvoice)  // for each voice
        {
            //	float *buffer = bufferVoices[currentvoice];
            //		buffer[index]=0.0f;

            // calc the modulators
            float *mod = modulator[currentvoice];
            mod[8] = 1.f - egCalc(currentvoice, 1);
            mod[9] = 1.f - egCalc(currentvoice, 2);
            mod[10] = 1.f - egCalc(currentvoice, 3);
            mod[11] = 1.f - egCalc(currentvoice, 4);
            mod[12] = 1.f - egCalc(currentvoice, 5);
            mod[13] = 1.f - egCalc(currentvoice, 6);
            //}
            /**
             * calc the main audio signal
             */
            // for (currentvoice=0;currentvoice<_MULTITEMP;++currentvoice) // for each voice
            //{
            // get the parameter settings
            float *param = parameter[currentvoice];
            // casting floats to int for indexing the 3 oscillator wavetables with custom typecaster
            P1.f = phase[currentvoice][1];
            P2.f = phase[currentvoice][2];
            P3.f = phase[currentvoice][3];
            P1.f += bias.f;
            P2.f += bias.f;
            P3.f += bias.f;
            P1.i -= bias.i;
            P2.i -= bias.i;
            P3.i -= bias.i;
            iP1 = P1.i & tabM;  // i%=TableSize;
            iP2 = P2.i & tabM;  // i%=TableSize;
            iP3 = P3.i & tabM;  // i%=TableSize;
            /*
            int iP1 = (int) phase[currentvoice][1];// float to int, cost some cycles
            int iP2 = (int) phase[currentvoice][2];// hopefully this got optimized by compiler
            int iP3 = (int) phase[currentvoice][3];// hopefully this got optimized by compiler

                iP1=iP1&tabM;//i%=TableSize;
                iP2=iP2&tabM;//i%=TableSize;
                iP3=iP3&tabM;//i%=TableSize;
                */
            // if (i>tabM) i=tabM;

            if (iP1 < 0)
                iP1 = tabM;
            if (iP2 < 0)
                iP2 = tabM;
            if (iP3 < 0)
                iP3 = tabM;

            // create the next oscillator phase step for osc 3
            phase[currentvoice][3] += tabX * param[90];
#ifdef _PREFETCH
            __builtin_prefetch(&param[1], 0, 0);
            __builtin_prefetch(&param[2], 0, 1);
            __builtin_prefetch(&param[3], 0, 0);
            __builtin_prefetch(&param[4], 0, 0);
            __builtin_prefetch(&param[5], 0, 0);
            __builtin_prefetch(&param[7], 0, 0);
            __builtin_prefetch(&param[11], 0, 0);
#endif

            if (phase[currentvoice][3] >= tabF) {
                phase[currentvoice][3] -= tabF;
                // if (*phase>=tabF) *phase = 0; //just in case of extreme fm
            }
            if (phase[currentvoice][3] < 0.f) {
                phase[currentvoice][3] += tabF;
                //	if(*phase < 0.f) *phase = tabF-1;
            }

            unsigned int *choi = choice[currentvoice];
            // modulator [currentvoice][14]=Oscillator(parameter[currentvoice][90],choice[currentvoice][12],&phase[currentvoice][3]);
            // write the oscillator 3 output to modulators
            mod[14] = table[choi[12]][iP3];

            // --------------- calculate the parameters and modulations of main oscillators 1 and 2
            tf = param[1];
            tf *= param[2];
            ta1 = param[9];
            ta1 *= mod[choi[2]];  // osc1 first ampmod

#ifdef _PREFETCH
            __builtin_prefetch(&phase[currentvoice][1], 0, 2);
            __builtin_prefetch(&phase[currentvoice][2], 0, 2);
#endif

            // tf+=(midif[currentvoice]*parameter[currentvoice][2]*parameter[currentvoice][3]);
            tf += (midif[currentvoice] * (1.0f - param[2]) * param[3]);
            ta1 += param[11] * mod[choi[3]];  // osc1 second ampmod
            tf += (param[4] * param[5]) * mod[choi[0]];
            tf += param[7] * mod[choi[1]];
            // tf/=3.f;
            // ta/=2.f;
            // static inline float Oscillator(float frequency,int wave,float *phase)
            //{
            /*   int iP1 = (int) phase[currentvoice][1];// float to int, cost some cycles
               int iP2 = (int) phase[currentvoice][2];// hopefully this got optimized by compiler

               iP1=iP1&tabM;//i%=TableSize;
               iP2=iP2&tabM;//i%=TableSize;*/
            // if (i>tabM) i=tabM;

            // generate phase of oscillator 1
            phase[currentvoice][1] += tabX * tf;

            //	if (iP1<0) iP1=tabM;
            //	if (iP2<0) iP2=tabM;

            if (phase[currentvoice][1] >= tabF) {
                phase[currentvoice][1] -= tabF;
                // if (param[115]>0.f) phase[currentvoice][2]= 0; // sync osc2 to 1
                // branchless sync:
                phase[currentvoice][2] -= phase[currentvoice][2] * param[115];

                // if (*phase>=tabF) *phase = 0; //just in case of extreme fm
            }

#ifdef _PREFETCH
            __builtin_prefetch(&param[15], 0, 0);
            __builtin_prefetch(&param[16], 0, 0);
            __builtin_prefetch(&param[17], 0, 0);
            __builtin_prefetch(&param[18], 0, 0);
            __builtin_prefetch(&param[19], 0, 0);
            __builtin_prefetch(&param[23], 0, 0);
            __builtin_prefetch(&param[25], 0, 0);
            __builtin_prefetch(&choice[currentvoice][6], 0, 0);
            __builtin_prefetch(&choice[currentvoice][7], 0, 0);
            __builtin_prefetch(&choice[currentvoice][8], 0, 0);
            __builtin_prefetch(&choice[currentvoice][9], 0, 0);
#endif

            if (phase[currentvoice][1] < 0.f) {
                phase[currentvoice][1] += tabF;
                //	if(*phase < 0.f) *phase = tabF-1;
            }
            osc1 = table[choi[4]][iP1];
            //}
            // osc1 = Oscillator(tf,choice[currentvoice][4],&phase[currentvoice][1]);
            mod[3] = osc1 * (param[13] * (1.f + ta1));  //+parameter[currentvoice][13]*ta1);

            // ------------------------ calculate oscillator 2
            // --------------------- first the modulations and frequencys
            tf2 = param[16];
            tf2 *= param[17];
            ta2 = param[23];
            ta2 *= mod[choi[8]];  // osc2 first amp mod
            // tf2+=(midif[currentvoice]*parameter[currentvoice][17]*parameter[currentvoice][18]);
            tf2 += (midif[currentvoice] * (1.0f - param[17]) * param[18]);
            ta3 = param[25];
            ta3 *= mod[choi[9]];  // osc2 second amp mod
            tf2 += param[15] * param[19] * mod[choi[6]];
            tf2 += param[21] * mod[choi[7]];
            // tf/=3.f;
            // ta/=2.f;
            mod[4] = (param[28] + param[28] * (1.f - ta3));  // osc2 fm out

            // then generate the actual phase:
            phase[currentvoice][2] += tabX * tf2;
            if (phase[currentvoice][2] >= tabF) {
                phase[currentvoice][2] -= tabF;
                // if (*phase>=tabF) *phase = 0; //just in case of extreme fm
            }

#ifdef _PREFETCH
            __builtin_prefetch(&param[14], 0, 0);
            __builtin_prefetch(&param[29], 0, 0);
            __builtin_prefetch(&param[38], 0, 0);
            __builtin_prefetch(&param[48], 0, 0);
            __builtin_prefetch(&param[56], 0, 0);
#endif

            if (phase[currentvoice][2] < 0.f) {
                phase[currentvoice][2] += tabF;
                //	if(*phase < 0.f) *phase = tabF-1;
            }
            osc2 = table[choi[5]][iP2];
            // osc2 = Oscillator(tf2,choice[currentvoice][5],&phase[currentvoice][2]);
            mod[4] *= osc2;  // osc2 fm out

            // ------------------------------------- mix the 2 oscillators pre filter
            // temp=(parameter[currentvoice][14]-parameter[currentvoice][14]*ta1);
            temp = (param[14] * (1.f - ta1));
            temp *= osc1;
            temp += osc2 * (param[29] * (1.f - ta2));
            temp *= 0.5f;  // get the volume of the sum into a normal range
            temp += anti_denormal;

            // ------------- calculate the filter settings ------------------------------
            mf = (1.f - (param[38] * mod[choi[10]]));
            mf += (1.f - (param[48] * mod[choi[11]]));
            mo = param[56] * mf;


#ifdef _PREFETCH
            __builtin_prefetch(&param[30], 0, 0);
            __builtin_prefetch(&param[31], 0, 0);
            __builtin_prefetch(&param[32], 0, 0);
            __builtin_prefetch(&param[40], 0, 0);
            __builtin_prefetch(&param[41], 0, 0);
            __builtin_prefetch(&param[42], 0, 0);
            __builtin_prefetch(&param[50], 0, 0);
            __builtin_prefetch(&param[51], 0, 0);
            __builtin_prefetch(&param[52], 0, 0);
#endif

            clib1 = fabs(mo);
            clib2 = fabs(mo - 1.0f);
            mo = clib1 + 1.0f;
            mo -= clib2;
            mo *= 0.5f;
            /*
            if (mo<0.f) mo = 0.f;
            else if (mo>1.f) mo = 1.f;
            */

            morph = (1.0f - mo);
/*
tf= (srate * (parameter[currentvoice][30]*morph+parameter[currentvoice][33]*mo)
); f[currentvoice][0] = 2.f * tf - (tf*tf*tf) * 0.1472725f;// / 6.7901358;

tf= (srate * (parameter[currentvoice][40]*morph+parameter[currentvoice][43]*mo)
); f[currentvoice][1] = 2.f * tf - (tf*tf*tf)* 0.1472725f; // / 6.7901358;;

tf = (srate * (parameter[currentvoice][50]*morph+parameter[currentvoice][53]*mo)
); f[currentvoice][2] = 2.f * tf - (tf*tf*tf) * 0.1472725f;// / 6.7901358;
*/

// parallel calculation:
#ifdef _VECTOR
            union f4vector a __attribute__((aligned(16))),
                b __attribute__((aligned(16))), c __attribute__((aligned(16))),
                d __attribute__((aligned(16))), e __attribute__((aligned(16)));

            b.f[0] = morph;
            b.f[1] = morph;
            b.f[2] = morph;
            b.f[3] = morph;
            a.f[0] = param[30];
            a.f[1] = param[31];
            a.f[2] = param[32];
            a.f[3] = param[40];
            d.f[0] = param[41];
            d.f[1] = param[42];
            d.f[2] = param[50];
            d.f[3] = param[51];
            c.v = a.v * b.v;
            // c.v = __builtin_ia32_mulps (a.v, b.v);
            e.v = d.v * b.v;
            // e.v = __builtin_ia32_mulps (d.v, b.v);

            tf1 = c.f[0];
            q[currentvoice][0] = c.f[1];
            v[currentvoice][0] = c.f[2];
            tf2 = c.f[3];
            q[currentvoice][1] = e.f[0];
            v[currentvoice][1] = e.f[1];
            tf3 = e.f[2];
            q[currentvoice][2] = e.f[3];

#else
            tf1 = param[30];
            q[currentvoice][0] = param[31];
            v[currentvoice][0] = param[32];
            tf2 = param[40];

            q[currentvoice][1] = param[41];
            v[currentvoice][1] = param[42];
            tf3 = param[50];
            q[currentvoice][2] = param[51];
#endif

            v[currentvoice][2] = param[52];

#ifdef _PREFETCH
            __builtin_prefetch(&param[33], 0, 0);
            __builtin_prefetch(&param[34], 0, 0);
            __builtin_prefetch(&param[35], 0, 0);
            __builtin_prefetch(&param[43], 0, 0);
            __builtin_prefetch(&param[44], 0, 0);
            __builtin_prefetch(&param[45], 0, 0);
            __builtin_prefetch(&param[53], 0, 0);
            __builtin_prefetch(&param[54], 0, 0);
            __builtin_prefetch(&param[55], 0, 0);
#endif

#ifndef _VECTOR
            tf1 *= morph;
            tf2 *= morph;
            q[currentvoice][0] *= morph;
            v[currentvoice][0] *= morph;

            tf3 *= morph;
            v[currentvoice][1] *= morph;
            q[currentvoice][1] *= morph;
            q[currentvoice][2] *= morph;
#endif

            v[currentvoice][2] *= morph;

#ifdef _VECTOR
            a.f[0] = param[33];
            a.f[1] = param[34];
            a.f[2] = param[35];
            a.f[3] = param[43];
            d.f[0] = param[44];
            d.f[1] = param[45];
            d.f[2] = param[53];
            d.f[3] = param[54];
            b.f[0] = mo;
            b.f[1] = mo;
            b.f[2] = mo;
            b.f[3] = mo;
            c.v = a.v * b.v;
            // c.v = __builtin_ia32_mulps (a.v, b.v);
            e.v = d.v * b.v;
            // e.v = __builtin_ia32_mulps (d.v, b.v);

            tf1 += c.f[0];
            tf2 += c.f[3];
            tf3 += e.f[2];
            q[currentvoice][0] += c.f[1];  // parameter[currentvoice][34]*mo;
            q[currentvoice][1] += e.f[0];  // parameter[currentvoice][44]*mo;
            q[currentvoice][2] += e.f[3];  // parameter[currentvoice][54]*mo;
            v[currentvoice][0] += c.f[2];  // parameter[currentvoice][35]*mo;
            v[currentvoice][1] += e.f[1];  // parameter[currentvoice][45]*mo;

#else
            tf1 += param[33] * mo;
            tf2 += param[43] * mo;
            tf3 += param[53] * mo;
#endif


#ifndef _VECTOR
            q[currentvoice][0] += param[34] * mo;
            q[currentvoice][1] += param[44] * mo;
            q[currentvoice][2] += param[54] * mo;

            v[currentvoice][0] += param[35] * mo;
            v[currentvoice][1] += param[45] * mo;

            tf1 *= srate;
            tf2 *= srate;
            tf3 *= srate;
#endif

#ifdef _VECTOR
            // prepare next calculations

            a.f[0] = param[55];
            a.f[1] = tf1;
            a.f[2] = tf2;
            a.f[3] = tf3;
            g.f[0] = mo;  // b.f[1] = 2.f; b.f[2] = 2.f; b.f[3] = 2.f;
            j.v = a.v * i.v;  // tf * srate
            c.v = j.v * g.v;  // tf * 2
            // c.v = __builtin_ia32_mulps (a.v, g.v);

            v[currentvoice][2] += c.f[0];  // parameter[currentvoice][55]*mo;

            // f[currentvoice][0] = c.f[1];//2.f * tf1;
            // f[currentvoice][1] = c.f[2];//2.f * tf2;
            // f[currentvoice][2] = c.f[3];//2.f * tf3;
            // pow(c.v,3);
            d.v = c.v - ((j.v * j.v * j.v) * h.v);

            f[currentvoice][0] = d.f[1];  //(tf1*tf1*tf1) * 0.1472725f;// / 6.7901358;

            f[currentvoice][1] = d.f[2];  //(tf2*tf2*tf2)* 0.1472725f; // / 6.7901358;;

            f[currentvoice][2] = d.f[3];  //(tf3*tf3*tf3) * 0.1472725f;// / 6.7901358;
#else
            v[currentvoice][2] += param[55] * mo;

            f[currentvoice][0] = 2.f * tf1;
            f[currentvoice][1] = 2.f * tf2;
            f[currentvoice][2] = 2.f * tf3;

            f[currentvoice][0] -= (tf1 * tf1 * tf1) * 0.1472725f;  // / 6.7901358;

            f[currentvoice][1] -= (tf2 * tf2 * tf2) * 0.1472725f;  // / 6.7901358;;

            f[currentvoice][2] -= (tf3 * tf3 * tf3) * 0.1472725f;  // / 6.7901358;
#endif
            //----------------------- actual filter calculation -------------------------
            // first filter
            float reso = q[currentvoice][0];  // for better scaling the volume with rising q
            low[currentvoice][0] =
                low[currentvoice][0] + f[currentvoice][0] * band[currentvoice][0];
            high[currentvoice][0] = ((reso + ((1.f - reso) * 0.1f)) * temp) -
                                    low[currentvoice][0] -
                                    (reso * band[currentvoice][0]);
            // high[currentvoice][0] = (reso *temp) - low[currentvoice][0] - (q[currentvoice][0]*band[currentvoice][0]);
            band[currentvoice][0] =
                f[currentvoice][0] * high[currentvoice][0] + band[currentvoice][0];

            reso = q[currentvoice][1];
            // second filter
            low[currentvoice][1] =
                low[currentvoice][1] + f[currentvoice][1] * band[currentvoice][1];
            high[currentvoice][1] = ((reso + ((1.f - reso) * 0.1f)) * temp) -
                                    low[currentvoice][1] -
                                    (reso * band[currentvoice][1]);
            band[currentvoice][1] =
                f[currentvoice][1] * high[currentvoice][1] + band[currentvoice][1];
            /*
                low[currentvoice][1] = low[currentvoice][1] + f[currentvoice][1]
            * band[currentvoice][1]; high[currentvoice][1] = (q[currentvoice][1]
            * band[currentvoice][1]) - low[currentvoice][1] -
            (q[currentvoice][1]*band[currentvoice][1]); band[currentvoice][1]=
            f[currentvoice][1] * high[currentvoice][1] + band[currentvoice][1];
            */
            // third filter
            reso = q[currentvoice][2];
            low[currentvoice][2] =
                low[currentvoice][2] + f[currentvoice][2] * band[currentvoice][2];
            high[currentvoice][2] = ((reso + ((1.f - reso) * 0.1f)) * temp) -
                                    low[currentvoice][2] -
                                    (reso * band[currentvoice][2]);
            band[currentvoice][2] =
                f[currentvoice][2] * high[currentvoice][2] + band[currentvoice][2];

            mod[7] = (low[currentvoice][0] * v[currentvoice][0]) +
                     band[currentvoice][1] * v[currentvoice][1] +
                     band[currentvoice][2] * v[currentvoice][2];

            //---------------------------------- amplitude shaping

            result = (1.f - mod[choi[13]] * param[100]);  ///_MULTITEMP;
            result *= mod[7];
            result *= egCalc(currentvoice, 0);  // the final shaping envelope

            // --------------------------------- delay unit
            if (delayI[currentvoice] >= delayBufferSize) {
                delayI[currentvoice] = 0;

                // printf("clear %d : %d : %d\n",currentvoice,delayI[currentvoice],delayJ[currentvoice]);
            }
            delayMod = 1.f - (param[110] * mod[choi[14]]);

            delayJ[currentvoice] =
                delayI[currentvoice] - ((param[111] * maxDelayTime) * delayMod);

            if (delayJ[currentvoice] < 0) {
                delayJ[currentvoice] += delayBufferSize;
            }
            else if (delayJ[currentvoice] > delayBufferSize) {
                delayJ[currentvoice] = 0;
            }

            // if (delayI[currentvoice]>95000)
            // printf("jab\n");

            tdelay = result * param[114] +
                     (delayBuffer[currentvoice][delayJ[currentvoice]] * param[112]);
            tdelay += anti_denormal;
            // tdelay -= anti_denormal;
            delayBuffer[currentvoice][delayI[currentvoice]] = tdelay;
            /*
            if (delayI[currentvoice]>95000)
            {
                printf("lll %d : %d : %d\n",currentvoice,delayI[currentvoice],delayJ[currentvoice]);
                fflush(stdout);
            }
            */
            mod[18] = tdelay;
            result += tdelay * param[113];
            delayI[currentvoice] = delayI[currentvoice] + 1;

            // --------------------------------- output
            float *buffer = bufferVoices[currentvoice];
            buffer[index] = result * param[101];
            bufferAux1[index] += result * param[108];
            bufferAux2[index] += result * param[109];
            result *= param[106];  // mix volume
            bufferMixLeft[index] += result * (1.f - param[107]);
            bufferMixRight[index] += result * param[107];

            // buffer[index] = Oscillator(50.2f,&phase1) * 0.5f;
            // Initialization done here is the oscillator loop
        }
    }
}  // end of process function

void cpuHandleMidi(snd_seq_event_t *ev)
{
    unsigned int c = _MULTITEMP;  // channel of incoming data

    switch (ev->type) {  // first check the controllers
        // they usually come in hordes
    case SND_SEQ_EVENT_CONTROLLER: {
        c = ev->data.control.channel;
#ifdef _DEBUG
        fprintf(stderr, "Control event on Channel %2d: %2d %5d       \r",
                c, ev->data.control.param, ev->data.control.value);
#endif
        if (c < _MULTITEMP) {
            if (ev->data.control.param == 1)
                modulator[c][16] = ev->data.control.value * 0.007874f;  // /127.f;
            else if (ev->data.control.param == 12)
                modulator[c][17] = ev->data.control.value * 0.007874f;  // /127.f;
            else if (ev->data.control.param == 2)
                modulator[c][20] = ev->data.control.value * 0.007874f;  // /127.f;
            else if (ev->data.control.param == 3)
                modulator[c][21] = ev->data.control.value * 0.007874f;  // /127.f;
            else if (ev->data.control.param == 4)
                modulator[c][22] = ev->data.control.value * 0.007874f;  // /127.f;
            else if (ev->data.control.param == 5)
                modulator[c][23] = ev->data.control.value * 0.007874f;  // /127.f;
            else if (ev->data.control.param == 14)
                modulator[c][24] = ev->data.control.value * 0.007874f;  // /127.f;
            else if (ev->data.control.param == 15)
                modulator[c][25] = ev->data.control.value * 0.007874f;  // /127.f;
            else if (ev->data.control.param == 16)
                modulator[c][26] = ev->data.control.value * 0.007874f;  // /127.f;
            else if (ev->data.control.param == 17)
                modulator[c][27] = ev->data.control.value * 0.007874f;  // /127.f;
        }
        break;
    }
    case SND_SEQ_EVENT_PITCHBEND: {
        c = ev->data.control.channel;
#ifdef _DEBUG
        fprintf(stderr, "Pitchbender event on Channel %2d: %5d   \r",
                c, ev->data.control.value);
#endif
        if (c < _MULTITEMP)
            modulator[c][2] = ev->data.control.value * 0.0001221f;  // /8192.f;
        break;
    }
    case SND_SEQ_EVENT_CHANPRESS: {
        c = ev->data.control.channel;
#ifdef _DEBUG
        fprintf(stderr, "touch event on Channel %2d: %5d   \r",
                c, ev->data.control.value);
#endif
        if (c < _MULTITEMP)
            modulator[c][15] = (float)ev->data.control.value * 0.007874f;
        break;
    }

    case SND_SEQ_EVENT_NOTEON: {
        c = ev->data.note.channel;
#ifdef _DEBUG
        fprintf(stderr, "Note On event on Channel %2d: %5d       \r",
                c, ev->data.note.note);
#endif
        if (c < _MULTITEMP) {
            if (ev->data.note.velocity > 0) {
                lastnote[c] = ev->data.note.note;
                midif[c] = midi2freq[ev->data.note.note];  // lookup the frequency
                modulator[c][19] = ev->data.note.note * 0.007874f;  // fill the value in as normalized modulator
                modulator[c][1] =
                    (float)1.f - (ev->data.note.velocity * 0.007874f);  // fill in the velocity as modulator
                egStart(c, 0);  // start the engines!
                if (EGrepeat[c][1] == 0)
                    egStart(c, 1);
                if (EGrepeat[c][2] == 0)
                    egStart(c, 2);
                if (EGrepeat[c][3] == 0)
                    egStart(c, 3);
                if (EGrepeat[c][4] == 0)
                    egStart(c, 4);
                if (EGrepeat[c][5] == 0)
                    egStart(c, 5);
                if (EGrepeat[c][6] == 0)
                    egStart(c, 6);

                break;  // not the best method but it breaks only when a note on is
            }  // if velo == 0 it should be handled as noteoff...
        }
    }
        // ...so its necessary that here follow the noteoff routine
    case SND_SEQ_EVENT_NOTEOFF: {
        c = ev->data.note.channel;
#ifdef _DEBUG
        fprintf(stderr, "Note Off event on Channel %2d: %5d      \r",
                c, ev->data.note.note);
#endif
        if (c < _MULTITEMP)
            if (lastnote[c] == ev->data.note.note) {
                egStop(c, 0);
                if (EGrepeat[c][1] == 0)
                    egStop(c, 1);
                if (EGrepeat[c][2] == 0)
                    egStop(c, 2);
                if (EGrepeat[c][3] == 0)
                    egStop(c, 3);
                if (EGrepeat[c][4] == 0)
                    egStop(c, 4);
                if (EGrepeat[c][5] == 0)
                    egStop(c, 5);
                if (EGrepeat[c][6] == 0)
                    egStop(c, 6);
            }
        break;
    }

#ifdef _DEBUG
    default: {

        if (ev->type != SND_SEQ_EVENT_CLOCK) {
            fprintf(stderr, "unknown event %d on Channel %2d: %5d   \r",
                    ev->type, ev->data.control.channel,
                    ev->data.control.value);
        }
    }
#endif
    }  // end of switch
}

int cpuReceiveChoice(int voice, int i, int value)
{
    if ((voice < _MULTITEMP) && (i < _CHOICEMAX)) {
        choice[voice][i] = value;
        return 0;
    }
    else
        return 1;
}

int cpuReceiveParameter(int voice, int i, float value)
{
    if ((voice < _MULTITEMP) && (i > 0) && (i < _PARACOUNT)) {
        parameter[voice][i] = value;
        // if ((i==2)||(i==17))
        //	parameter[voice][i]=1.f-value;
    }

    // if ((i==10) && (parameter[10]!=0)) parameter[10]=1000.f;
    // printf("%s <- f:%f, i:%d\n\n", path, argv[0]->f, argv[1]->i);
    // fflush(stdout);
    switch (i) {
    // reset the filters
    case 0: {
        low[voice][0] = 0.f;
        high[voice][0] = 0.f;
        band[voice][0] = 0.f;
        low[voice][1] = 0.f;
        high[voice][1] = 0.f;
        band[voice][1] = 0.f;
        low[voice][2] = 0.f;
        high[voice][2] = 0.f;
        band[voice][2] = 0.f;
        phase[voice][1] = 0.f;
        phase[voice][2] = 0.f;
        phase[voice][3] = 0.f;
        memset(delayBuffer[voice], 0, sizeof(delayBuffer[voice]));
        break;
    }

    case 60:
        EG[voice][1][1] = value;
        break;
    case 61:
        EG[voice][1][2] = value;
        break;
    case 62:
        EG[voice][1][3] = value;
        break;
    case 63:
        EG[voice][1][4] = value;
        break;
    case 64: {
        EGrepeat[voice][1] = (value > 0) ? 1 : 0;
        if (EGrepeat[voice][1] > 0)
            egStart(voice, 1);
        break;
    }
    case 65:
        EG[voice][2][1] = value;
        break;
    case 66:
        EG[voice][2][2] = value;
        break;
    case 67:
        EG[voice][2][3] = value;
        break;
    case 68:
        EG[voice][2][4] = value;
        break;
    case 69: {
        EGrepeat[voice][2] = (value > 0) ? 1 : 0;
        if (EGrepeat[voice][2] > 0)
            egStart(voice, 2);
        break;
    }
    case 70:
        EG[voice][3][1] = value;
        break;
    case 71:
        EG[voice][3][2] = value;
        break;
    case 72:
        EG[voice][3][3] = value;
        break;
    case 73:
        EG[voice][3][4] = value;
        break;
    case 74: {
        EGrepeat[voice][3] = (value > 0) ? 1 : 0;
        if (EGrepeat[voice][3] > 0)
            egStart(voice, 3);
        break;
    }
    case 75:
        EG[voice][4][1] = value;
        break;
    case 76:
        EG[voice][4][2] = value;
        break;
    case 77:
        EG[voice][4][3] = value;
        break;
    case 78:
        EG[voice][4][4] = value;
        break;
    case 79: {
        EGrepeat[voice][4] = (value > 0) ? 1 : 0;
        if (EGrepeat[voice][4] > 0)
            egStart(voice, 4);
        break;
    }
    case 80:
        EG[voice][5][1] = value;
        break;
    case 81:
        EG[voice][5][2] = value;
        break;
    case 82:
        EG[voice][5][3] = value;
        break;
    case 83:
        EG[voice][5][4] = value;
        break;
    case 84: {
        EGrepeat[voice][5] = (value > 0) ? 1 : 0;
        if (EGrepeat[voice][5] > 0)
            egStart(voice, 5);
        break;
    }
    case 85:
        EG[voice][6][1] = value;
        break;
    case 86:
        EG[voice][6][2] = value;
        break;
    case 87:
        EG[voice][6][3] = value;
        break;
    case 88:
        EG[voice][6][4] = value;
        break;
    case 89: {
        EGrepeat[voice][6] = (value > 0) ? 1 : 0;
        if (EGrepeat[voice][6] > 0)
            egStart(voice, 6);
        break;
    }
    case 102:
        EG[voice][0][1] = value;
        break;
    case 103:
        EG[voice][0][2] = value;
        break;
    case 104:
        EG[voice][0][3] = value;
        break;
    case 105:
        EG[voice][0][4] = value;
        break;
    }
    // float g=parameter[30]*parameter[56]+parameter[33]*(1.0f-parameter[56]);
#ifdef _DEBUG
    printf("%i %i %f \n", voice, i, value);
#endif
    return 0;
}
