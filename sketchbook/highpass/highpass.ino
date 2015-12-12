/*
 * Highpass Audio Filter
 * Eric Dargelies
 * 
 * This is just an idea of how to create a highpass filter
 * In my experiments the Arduino doesn't seem to be fast 
 * enough to complete the floating point operations required
 * for audio filter and output.
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
  FilterOnePole highpassFilter(HIGHPASS, frequency);
  while( true ) {
    incomingAudio = analogRead(A0)/4 - 127;
    highpassFilter.input(incomingAudio);//filter to include the negative component of audio signal
    outgoingAudio = highpassFilter.output() + 127;//final output is a lowpassed, add back 127 for output to DAC
    if (incomingAudio<0){ //deal with negative numbers
      incomingAudio = 0;
    }
    PORTD = outgoingAudio;//Output to DAC
    frequency = analogRead(A1) * 4800.0 / 1023.0;
    FilterOnePole highpassFilter(HIGHPASS, frequency);
  }
}
