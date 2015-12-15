/*
   Spectrum Analyzer
   Eric Dargelies

   This is an audio spectrum analyer.  The code expects an audio signal
   that has been gained and DC offset to oscillate around 2.5V on
   analog pin 0.  The LED matrix will dynamically size the data
   based on the magnitude of the FHT output for the correct LED matrix
   size.
*/

#define LOG_OUT 1 // Turns on log function resources
#define OCTAVE 1 // Turns on octave function resources
#define FHT_N 256 // bins = [0, 1, 2:4, 5:8, 9:16, 17:32, 33:64, 65:128]

#include <LedControl.h>
#include <FHT.h>

#define DIN 12 // The arduino Digital pin that provides the data for the LED matrix
#define CS 11 // The digital pin that provides the chip select input for the LED matrix
#define CLK 10 // The digital pin that provides the clock input for the LED matrix
#define ROWS 8 // Number of rows of the LED matrix, left as a parameter in case more matrices are added
#define COLUMNS 8 // Number of columns of the LED matrix, left as a parameter in case more matrices are added
int maxMag;
int ledArr[8];
int rowArr[8];



LedControl lc = LedControl(DIN, CLK, CS, 0); // Initialize the LED control

void setup() {
  maxMag = -1023;
  lc.shutdown(0, false);      // The MAX72XX is in power-saving mode on startup
  lc.setIntensity(0, 15);     // Set the brightness to maximum value
  lc.clearDisplay(0);         // and clear the display

  //  Serial.begin(57600);
}

void loop() {
  sampleInput();
  populateLedMatrix();
  drawLed();
}

void sampleInput() {
  memset(fht_input, 0, sizeof(fht_input));
  for (int i = 0; i < FHT_N; i++) { 
    fht_input[i] = analogRead( A0 ); // put real data into bins
    //    Serial.print("Sample in value "); Serial.print(i); Serial.print(" : "); Serial.println(fht_input[i]);
  }
  fht_window(); // window the data for better frequency response
  fht_reorder(); // reorder the data before doing the fht
  fht_run(); // process the data in the fht
  fht_mag_octave(); // take the output of the fht
}

void populateLedMatrix() {
  memset(rowArr, 0, sizeof(rowArr));  // Zero-out the row array
  maxMag--; //This is here to continue updating the maximum magnitude
  int colIndex = COLUMNS - 1;
  for (int i = 0; i < COLUMNS; i++) {
    Serial.print("FHT octave: "); Serial.print(fht_oct_out[colIndex]); Serial.print(" for column "); Serial.println(colIndex);
    ledArr[i] = calcColumnVal(fht_oct_out[colIndex]);
    Serial.print("Led DB: "); Serial.print(ledArr[i]); Serial.print(" for led array "); Serial.println(i);
    for (int j = 0; j < ROWS; j++) {
      if (i == 0) {
        rowArr[j] = bitRead(ledArr[i], j);
      }
      else {
        rowArr[j] = rowArr[j] + pwer(bitRead(ledArr[i], j) * 2, i);
      }
      //      Serial.print("Row array value: "); Serial.print(rowArr[j]); Serial.print(" for row "); Serial.println(j);
    }
    colIndex--; // This ordering is a correction for the orientation of the matrix
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
  if (dB > maxMag) {
    maxMag = dB; //dynamically size the array for the input
  }
  int val = ceil(dB * (ROWS - 1) / maxMag);
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


