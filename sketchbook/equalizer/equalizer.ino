#define LOG_OUT 1
#define OCTAVE 1
#define FHT_N 128

#include <LedControl.h>
#include <FHT.h>
#include <Filters.h>
#include <helper.ino>

#define DIN 12
#define CS 11
#define CLK 10
#define MAX_DB 12
#define MIN_DB -4
#define ROWS 8
#define COLUMNS 8
#define DSP_SZ 8
int range, stepSize;
int dBBin[] = {2, 8, 10, 11, 4, 2, 4, 0};
int ledArr[8];
int rowArr[8];
String sampleSet;
int audioIn;



LedControl lc = LedControl(DIN, CLK, CS, 0);
FilterOnePole lowpassFilter( LOWPASS, 8000 ); //initialize the lowpass filter

void setup() {
  lc.shutdown(0, false);      //The MAX72XX is in power-saving mode on startup
  lc.setIntensity(0, 15);     // Set the brightness to maximum value
  lc.clearDisplay(0);         // and clear the display

  Serial.begin(115200);
  range = MAX_DB - MIN_DB;
  stepSize = range / 8;

  for (byte i=0;i<8;i++){
    pinMode(i,OUTPUT);//set digital pins 0-7 as outputs (DAC)
  }
  //  populateLedMatrix();
  //  drawLed();
}

void loop() {
  filterInput();
  sampleInput();
  populateLedMatrix();
  drawLed();
//  sampleFix();

}

void filterInput(){
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


void sampleInput() {
  for (int i=0; i<FHT_N; i++) {
    int sample = analogRead(A0);
    fht_input[i] = sample; // put real data into bins
//    Serial.println(sample);
  }
  fht_window(); // window the data for better frequency response
  fht_reorder(); // reorder the data before doing the fht
  fht_run(); // process the data in the fht
  fht_mag_octave(); // take the output of the fht
}

void populateLedMatrix() {
  memset(rowArr, 0, sizeof(rowArr));
  int colIndex = COLUMNS - 1;
  for (int i = 0; i < COLUMNS; i++) {
    Serial.print("FHT octave");
    Serial.println(fht_oct_out[i]);
    ledArr[i] = calcColumnVal(fht_oct_out[colIndex]/10);
    Serial.print("Led db");
    Serial.println(ledArr[i]);

//    ledArr[i] = calcColumnVal(random(-4,12));

    //ledArr[i] = calcColumnVal(dBBin[colIndex]);
    for (int j = 0; j < ROWS; j++) {
      if (i == 0) {
        rowArr[j] = bitRead(ledArr[i], j);
      }
      else {
        rowArr[j] = rowArr[j] + pwer(bitRead(ledArr[i], j) * 2, i);
      }
    }
    colIndex--;
  }
}

void drawLed() {
  int rowIndex = ROWS - 1;
  for (int i = 0; i < ROWS; i++) {
    lc.setRow(0, rowIndex, rowArr[i]);
    rowIndex--;
  }
}

int calcColumnVal(int dB) {
  int val = ceil((dB - MIN_DB) / stepSize);
  val = pwer(2, val) - 1;
  return val;
}

int pwer(int base, int power) {
  int val = 1;
  if (power == 0) {
    return val;
  }
  while (power > 0) {
    val = val * base;
    power--;
  }
  return val;
}


