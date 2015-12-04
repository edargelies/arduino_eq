#include <LedControl.h>

int DIN = 12;
int CS =  11;
int CLK = 10;

int maxdB = 12;
int mindB = -4;
int rows = 8;
int columns = 8;
int range, stepSize;
int dBBin[] = {2, 8, 10, 11, 4, 2, 4, 0};
int ledArr[8];
int rowArr[8];

LedControl lc=LedControl(DIN,CLK,CS,0);

void setup(){
 lc.shutdown(0,true);       //The MAX72XX is in power-saving mode on startup
 lc.setIntensity(0,15);      // Set the brightness to maximum value
 lc.clearDisplay(0);         // and clear the display

 Serial.begin(9600);
 range = maxdB - mindB;
 stepSize = range / 8; 
}

void loop() {
  memset(rowArr,0,sizeof(rowArr));
 for (int i = 0; i < 8; i++){
  ledArr[i]=calcColumnVal(random(-4,12));
  for (int j = 0; j < 8; j++){
    rowArr[j] = rowArr[j] + 0.5 + pow(bitRead(ledArr[i],j)*2,j);
  }
 }
 for (int i = 0; i < 8; i++){
  lc.setColumn(0,i,ledArr[i]);
 }
}

int calcColumnVal(int dB){
  int val = ceil((dB-mindB)/stepSize);
  // pow deals with floating point numbers, correct with -.5
  //To-do write my own pow for ints
  val = -.5 + pow(2, val);
  return val;
}

int power(int base, int power){
    if(power == 0) return 1;
    return base * pow(base, --power);
}


