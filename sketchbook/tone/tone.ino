#include <LedControl.h>
int DIN = 12;
int CS =  11;
int CLK = 10;

int speakerPin = 3;
int numTones = 11;
int tones[] = {261, 277, 294, 311, 330, 349, 370, 392, 415, 440, 700};

LedControl lc = LedControl(DIN, CLK, CS, 0);

void setup() {
  lc.shutdown(0, true);      //The MAX72XX is in power-saving mode on startup
  lc.setIntensity(0, 15);     // Set the brightness to maximum value
  lc.clearDisplay(0); 
  for (int i = 0; i < numTones; i++)
  {
    tone(speakerPin, tones[i]);
    delay(500);
  }
  noTone(speakerPin);
}

void loop() {
  // put your main code here, to run repeatedly:

}
