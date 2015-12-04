#define LOG_OUT 1 // Turns on log function resources
#define OCTAVE 1 // Turns on octave function resources
#define FHT_N 128 // bins = [0, 1, 2:4, 5:8, 9:16, 17:32, 3:64]

#include <LedControl.h>
#include <FHT.h>

#define DIN 13 // The arduino Digital pin that provides the data for the LED matrix
#define CS 12 // The digital pin that provides the chip select input for the LED matrix
#define CLK 11 // The digital pin that provides the clock input for the LED matrix
#define MAX_DB 12
#define MIN_DB -4
#define ROWS 8 // Number of rows of the LED matrix, left as a parameter in case more matrices are added
#define COLUMNS 8 // Number of columns of the LED matrix, left as a parameter in case more matrices are added
int range, stepSize;
int ledArr[8];
int rowArr[8];
String sampleSet;



LedControl lc = LedControl(DIN, CLK, CS, 0); // Initialize the LED control

void setup() {
  lc.shutdown(0, false);      //The MAX72XX is in power-saving mode on startup
  lc.setIntensity(0, 15);     // Set the brightness to maximum value
  lc.clearDisplay(0);         // and clear the display

  Serial.begin(115200);
  range = MAX_DB - MIN_DB;
  stepSize = range / 8;
  for (int i=0;i<8;i++){ // Prepare the digital out pins for audio output
    pinMode(i,OUTPUT);
  }
}

void loop() {
  sampleInput();
  populateLedMatrix();
  drawLed();
}

void sampleInput() {
  memset(fht_input, 0, sizeof(fht_input));
  for (int i=0; i<FHT_N; i++) { //may be able to optimize here 0 at odd indices
    int sample = analogRead( A0 );
//    PORTD = sample;
    fht_input[i] = sample; // put real data into bins
  }
  fht_window(); // window the data for better frequency response
  fht_reorder(); // reorder the data before doing the fht
  fht_run(); // process the data in the fht
  fht_mag_octave(); // take the output of the fht
}

void populateLedMatrix() {
  memset(rowArr, 0, sizeof(rowArr));  // Zero-out the row array
  int colIndex = COLUMNS - 1;
  for (int i = 0; i < COLUMNS; i++) {
    Serial.print("FHT octave");
    Serial.println(fht_oct_out[i]);
    ledArr[i] = calcColumnVal(fht_oct_out[colIndex]/10);
    Serial.print("Led db");
    Serial.println(ledArr[i]);
    for (int j = 0; j < ROWS; j++) {
      if (i == 0) {
        rowArr[j] = bitRead(ledArr[i], j);
      }
      else {
        rowArr[j] = rowArr[j] + pwer(bitRead(ledArr[i], j) * 2, i);
      }
    }
    colIndex--; // This ordering is purely a correction for the orientation of the matrix
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


