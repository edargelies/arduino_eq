//Simple Audio In w filter and output to 8 bit DAC
//by Eric Dargelies
//Credit: Amanda Ghassaei for Audio in and out to 8 bit DAC
//November 2015

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
*/
#include <Filters.h>
int audioIn;

void setup(){
  for (byte i=0;i<8;i++){
    pinMode(i,OUTPUT);//set digital pins 0-7 as outputs (DAC)
  }
}

void loop(){
  float frequency = analogRead(A1) * 8000/1023;
  FilterOnePole lowpassFilter( LOWPASS, frequency );
  while( true ) {
    lowpassFilter.input( A0 );
    audioIn = analogRead(A0);//read voltage at A0
    audioIn = (audioIn+1)/4 - 1;//scale from 10 bit (0-1023) to 8 bit (0-255)
    if (audioIn<0){ //deal with negative numbers
      audioIn = 0;
    }
    PORTD = audioIn;
    frequency = analogRead(A1) * 8000/1023;
    lowpassFilter( LOWPASS, frequency );
  }
}

