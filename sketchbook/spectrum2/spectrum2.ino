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

LedControl lc = LedControl(DIN, CLK, CS, 0);

void setup() {
  lc.shutdown(0, true);      //The MAX72XX is in power-saving mode on startup
  lc.setIntensity(0, 15);     // Set the brightness to maximum value
  lc.clearDisplay(0);         // and clear the display

  Serial.begin(9600);
  range = maxdB - mindB;
  stepSize = range / 8;
  //  populateLedMatrix();
  //  drawLed();

}

void loop() {
  populateLedMatrix();
  drawLed();

}

void populateLedMatrix() {
  memset(rowArr, 0, sizeof(rowArr));
  int colIndex = columns - 1;
  for (int i = 0; i < columns; i++) {
    ledArr[i] = calcColumnVal(random(-4, 12));
    //ledArr[i] = calcColumnVal(dBBin[colIndex]);
    for (int j = 0; j < rows; j++) {
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
  int rowIndex = rows - 1;
  for (int i = 0; i < rows; i++) {
    lc.setRow(0, rowIndex, rowArr[i]);
    rowIndex--;
  }
}

int calcColumnVal(int dB) {
  int val = ceil((dB - mindB) / stepSize);
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


