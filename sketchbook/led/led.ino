#include <LedControl.h>

int DIN = 12;
int CS =  11;
int CLK = 10;

int maxdB = 12;
int mindB = -4;
int range;
int stepSize;

LedControl lc=LedControl(DIN,CLK,CS,0);

void setup(){
 lc.shutdown(0,true);       //The MAX72XX is in power-saving mode on startup
 lc.setIntensity(0,15);      // Set the brightness to maximum value
 lc.clearDisplay(0);         // and clear the display

 range = maxdB - mindB;
 stepSize = range / 2;
}

void loop() {
  for (int i = 0; i <= 7; i++){
    for (int j = 0; j <=7; j++){
      lc.setLed(0, i, j, true); 
      delay(1);          
    }
  }
  for (int i = 7; i >= 0; i--){
    for (int j = 7; j >= 0; j--){
      lc.setLed(0, i, j, false);
      delay(1);     
    }
  }

}

int calcColumnVal(int dB){
  
}


