/* main.c - chromatic guitar tuner
 *
 * Copyright (C) 2012 by Bjorn Roche
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  This software is provided "as is" without express or
 * implied warranty.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <portaudio.h>

#define bool int
#define TRUE (1)
#define FALSE (0)

/* -- some basic parameters -- */
#define SAMPLE_RATE (8000)
#define LOOP_SIZE (160)
#define ONLY_FLOATS (TRUE)

//a must be of length 2, and b must be of length 3
void computeSecondOrderLowPassParameters( float srate, float f, float *a, float *b );
//mem must be of length 4.
float processSecondOrderFilter( float x, float *mem, float *a, float *b );
void signalHandler( int signum ) ;

static bool running = TRUE;

/* -- main function -- */
int main( int argc, char **argv ) {
   PaStreamParameters inputParameters;
   float a[2], b[3], mem1[4], mem2[4];
   float data[LOOP_SIZE];
   PaStream *stream = NULL;
   PaError err = 0;
   struct sigaction action;

   // add signal listen so we know when to exit:
   action.sa_handler = signalHandler;
   sigemptyset (&action.sa_mask);
   action.sa_flags = 0;

   sigaction (SIGINT, &action, NULL);
   sigaction (SIGHUP, &action, NULL);
   sigaction (SIGTERM, &action, NULL);

   computeSecondOrderLowPassParameters( SAMPLE_RATE, 330, a, b );
   mem1[0] = 0; mem1[1] = 0; mem1[2] = 0; mem1[3] = 0;
   mem2[0] = 0; mem2[1] = 0; mem2[2] = 0; mem2[3] = 0;

   // initialize portaudio
   err = Pa_Initialize();
   if( err != paNoError ) goto error;

   inputParameters.device = Pa_GetDefaultInputDevice();
   inputParameters.channelCount = 1;
   inputParameters.sampleFormat = paFloat32;
   inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultHighInputLatency ;
   inputParameters.hostApiSpecificStreamInfo = NULL;

   //printf( "Opening %s\n",
   //        Pa_GetDeviceInfo( inputParameters.device )->name );

   err = Pa_OpenStream( &stream,
                        &inputParameters,
                        NULL, //no output
                        SAMPLE_RATE,
                        LOOP_SIZE,
                        paClipOff,
                        NULL,
                        NULL );
   if( err != paNoError ) goto error;

   err = Pa_StartStream( stream );
   if( err != paNoError ) goto error;

   // this is the main loop where we listen to and
   // process audio.
   while( running )
   {
      // read some data
      err = Pa_ReadStream( stream, data, LOOP_SIZE );
      //if( err ) goto error; //FIXME: we don't want to err on xrun

      // low-pass
      for( int j=0; j<LOOP_SIZE; ++j ) {
         data[j] = processSecondOrderFilter( data[j], mem1, a, b );
         //data[j] = processSecondOrderFilter( data[j], mem2, a, b );
      }
      // count zero crossings and measure amplitude:
      static int sign = 1;
      static float amplitude = 0;
      int zeroCrossing = 0;
      for( int j=0; j<LOOP_SIZE; ++j ) {
         float a = data[j]*data[j];
         amplitude = a *.01 + amplitude * .99;
         if( sign == 1 ) {
            if( data[j] < 0 ) {
              sign = -1;
              ++zeroCrossing;
            }
         } else {
            if( data[j] > 0 ) {
              sign = 1;
              ++zeroCrossing;
            }
         }
      }
      static float zeroAve = 0;
      zeroAve = zeroCrossing * .1 + zeroAve * .9 ;
#if ONLY_FLOATS
      printf( "%g\t%g\n", zeroAve, sqrt(amplitude) );
      fflush(stdout);
#else
      printf( "Zero Crossings: %d\t%d\t", zeroCrossing, (int)(zeroAve*100) );
      for( int i=0; i<zeroAve*5; ++i )
         printf( "#" );
      printf( "\n" );
#endif

   }
   err = Pa_StopStream( stream );
   if( err != paNoError ) goto error;

   Pa_Terminate();

   return 0;
 error:
   if( stream ) {
      Pa_AbortStream( stream );
      Pa_CloseStream( stream );
   }
   Pa_Terminate();
   fprintf( stderr, "An error occured while using the portaudio stream\n" );
   fprintf( stderr, "Error number: %d\n", err );
   fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
   return 1;
}

void computeSecondOrderLowPassParameters( float srate, float f, float *a, float *b )
{
   float a0;
   float w0 = 2 * M_PI * f/srate;
   float cosw0 = cos(w0);
   float sinw0 = sin(w0);
   //float alpha = sinw0/2;
   float alpha = sinw0/2 * sqrt(2);

   a0   = 1 + alpha;
   a[0] = (-2*cosw0) / a0;
   a[1] = (1 - alpha) / a0;
   b[0] = ((1-cosw0)/2) / a0;
   b[1] = ( 1-cosw0) / a0;
   b[2] = b[0];
}
float processSecondOrderFilter( float x, float *mem, float *a, float *b )
{
    float ret = b[0] * x + b[1] * mem[0] + b[2] * mem[1]
                         - a[0] * mem[2] - a[1] * mem[3] ;

		mem[1] = mem[0];
		mem[0] = x;
		mem[3] = mem[2];
		mem[2] = ret;

		return ret;
}
void signalHandler( int signum ) { running = FALSE; }
