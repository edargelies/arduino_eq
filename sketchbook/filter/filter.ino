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
int audioOut;

void setup(){
  Serial.begin(57600);
  for (byte i=0;i<8;i++){
    pinMode(i,OUTPUT);//set digital pins 0-7 as outputs (DAC)
  }
}

void loop(){
  float frequency = analogRead(A1) * 8000.0 / 1023.0;
//  FilterOnePole filterOneLowpass( LOWPASS, 2000.0 );
  FilterTwoPole filterTwoLowpass; // create a two pole Lowpass filter
  filterTwoLowpass.setAsFilter( LOWPASS_BUTTERWORTH, 1000.0 );
  while( true ) {
//    Serial.println(analogRead(A1) * 8000.0 / 1023.0);
//    lowpassFilter.input( A0 );
    audioIn = analogRead( A0 )/4 - 128;
    filterTwoLowpass.input( audioIn );
    audioOut = filterTwoLowpass.output() + 127;//read voltage at A0
    //Serial.print("audioIn: "); Serial.print(audioIn); Serial.print("   audioOut: "); Serial.println(audioOut);
    //audioOut = (audioOut+1)/4 - 1;//scale from 10 bit (0-1023) to 8 bit (0-255)
    if (audioOut<0){ //deal with negative numbers
      audioOut = 0;
    }
    PORTD = audioOut;
//    frequency = analogRead(A1) * 8000.0 / 1023.0;
//    FilterOnePole lowpassFilter( LOWPASS, frequency );
  }
}

