/*
 * Bandpass Audio Filter
 * Eric Dargelies
 * 
 * This is just an idea of how to create a bandpass filter
 * In my experiments the Arduino doesn't seem to be fast 
 * enough to complete the floating point operations required
 * for audio filter and output.
 * 
 * Credit to Amanda Ghassaei for the DAC code
*/
#include <Filters.h>
int incomingAudio;
int outgoingAudio;

void setup(){
  for (byte i=0;i<8;i++){
    pinMode(i,OUTPUT);//set digital pins 0-7 as outputs (DAC)
  }
}

void loop(){
  float frequency = analogRead(A1) * 4800.0 / 1023.0;
  float bandwidth = analogRead(A2) * 4800.0 / 1023.0;
  FilterOnePole lowpassFilter(LOWPASS, (frequency + bandwidth/2));
  FilterOnePole highpassFilter(HIGHPASS, (frequency - bandwidth/2));
  while( true ) {
    incomingAudio = analogRead(A0)/4 - 127;//filter to include the negative component of audio signal
    lowpassFilter.input(incomingAudio);//lowpass
    highpassFilter.input(lowpassFilter.output()); //filter the lowpassed signal
    outgoingAudio = highpassFilter.output() + 127;//final output is a bandpass, add back 127 for output to DAC
    if (outgoingAudio<0){ //deal with negative numbers
      outgoingAudio = 0;
    }
    PORTD = outgoingAudio;//Output to DAC
    frequency = analogRead(A1) * 4800.0 / 1023.0;
    bandwidth = analogRead(A2) * 4800.0 / 1023.0;
    FilterOnePole lowpassFilter(LOWPASS, (frequency + bandwidth/2));
    FilterOnePole highpassFilter(HIGHPASS, (frequency - bandwidth/2));
  }
}
