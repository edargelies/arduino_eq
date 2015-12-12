/* Arduino Audio Reverb

   Modifed 2015 Eric Dargelies
   Removed LED writes, updated code to be reflective of hardware setup.
   Added additional audio effects from other labs.
   on http://interface.khm.de/index.php/lab/interfaces-advanced/arduino-realtime-audio-processing/

   Arduino Realtime Audio Processing
   2 ADC 8-Bit Mode
   analog input 0 is used to sample the audio signal
   analog input 1 is used to control an audio effect
   PWM DAC with Timer2 as analog output



   KHM 2008 / Lab3/  Martin Nawrath nawrath@khm.de
   Kunsthochschule fuer Medien Koeln
   Academy of Media Arts Cologne

*/


#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

/*variables for LED control*/
#define LOG_OUT 1 // Turns on log function resources
#define OCTAVE 1 // Turns on octave function resources
#define FHT_N 128 // bins = [0, 1, 2:4, 5:8, 9:16, 17:32, 3:64]

#include <LedControl.h>
#include <FHT.h>

#define DIN 13 // The arduino Digital pin that provides the data for the LED matrix
#define CS 12 // The digital pin that provides the chip select input for the LED matrix
#define CLK 10 // The digital pin that provides the clock input for the LED matrix
#define MAX_DB 12
#define MIN_DB -4
#define ROWS 8 // Number of rows of the LED matrix, left as a parameter in case more matrices are added
#define COLUMNS 8 // Number of columns of the LED matrix, left as a parameter in case more matrices are added
int range, stepSize;
int ledArr[8];
int rowArr[8];
String sampleSet;
/****************************/

boolean div32;
boolean div16;
// interrupt variables accessed globally
volatile boolean f_sample;
volatile byte byteADC0;
volatile byte byteADC1;
volatile byte byteADC2;
//volatile byte noop;

int audioIn; // The reading from analog pin 0
int effectIn; // The reading from analog pin 1
LedControl lc = LedControl(DIN, CLK, CS, 0); // Initialize the LED control

void setup()
{
  lc.shutdown(0, false);      //The MAX72XX is in power-saving mode on startup
  lc.setIntensity(0, 15);     // Set the brightness to maximum value
  lc.clearDisplay(0);         // and clear the display
  range = MAX_DB - MIN_DB;
  stepSize = range / ROWS;

  Serial.begin(57600);        // connect to the serial port
  Serial.print("Arduino Audio Spectrum");

  // set adc prescaler  to 32 for 38kHz sampling frequency
  sbi(ADCSRA, ADPS0);
  cbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS2);

  sbi(ADMUX, ADLAR); // 8-Bit ADC in ADCH Register
  sbi(ADMUX, REFS0); // VCC Reference
  cbi(ADMUX, REFS1);
  cbi(ADMUX, MUX0);  // Set Input Multiplexer to Channel 0
  cbi(ADMUX, MUX1);
  cbi(ADMUX, MUX2);
  cbi(ADMUX, MUX3);

  // Timer2 PWM Mode set to fast PWM
  cbi (TCCR2A, COM2A0);
  sbi (TCCR2A, COM2A1);
  sbi (TCCR2A, WGM20);
  sbi (TCCR2A, WGM21);
  cbi (TCCR2B, WGM22);

  // Timer2 Clock Prescaler to : 1
  sbi (TCCR2B, CS20);
  cbi (TCCR2B, CS21);
  cbi (TCCR2B, CS22);

  // Timer2 PWM Port Enable
  sbi(DDRB, 3);                   // set digital pin 11 to output

  cbi (TIMSK0, TOIE0);             // disable Timer0 !!! delay is off now
  sbi (TIMSK2, TOIE2);             // enable Timer2 Interrupt
}



void loop()
{
  sampleInput();
  populateLedMatrix();
  drawLed();
}
void sampleInput() {
  memset(fht_input, 0, sizeof(fht_input));
  for (int i = 0; i < FHT_N; i++) { //may be able to optimize here 0 at odd indices
    fht_input[i] = byteADC0; // put real data into bins, these are samples between 0-255
    Serial.print("Sample in value ");Serial.print(i);Serial.print(" : ");Serial.println(fht_input[i]);
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
    Serial.print("FHT octave: ");Serial.print(fht_oct_out[colIndex]);Serial.print(" for column ");Serial.println(colIndex);
    ledArr[i] = calcColumnVal(fht_oct_out[colIndex]/10);
    Serial.print("Led DB: ");Serial.print(ledArr[i]);Serial.print(" for led array ");Serial.println(i);
    for (int j = 0; j < ROWS; j++) {
      if (i == 0) {
        rowArr[j] = bitRead(ledArr[i], j);
      }
      else {
        rowArr[j] = rowArr[j] + pwer(bitRead(ledArr[i], j) * 2, i);
      }
      Serial.print("Row array value: ");Serial.print(rowArr[j]);Serial.print(" for row ");Serial.println(j);
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
//******************************************************************
// Timer2 Interrupt Service at 62.5 KHz
// here the audio and pot signal is sampled in a rate of:  16Mhz / 256 / 2 / 2 = 15625 Hz
// runtime : xxxx microseconds
ISR(TIMER2_OVF_vect) {

  PORTB = PORTB  | 1 ;

  div32 = !div32;                          // divide timer2 frequency / 2 to 31.25kHz
  if (div32) {
    div16 = !div16; //
    if (div16) {                       // sample channel 0 and 1 alternately so each channel is sampled with 15.6kHz
      byteADC0 = ADCH;                  // get ADC channel 0
      OCR2A = byteADC0;
      sbi(ADMUX, MUX0);              // set multiplexer to channel 1
      f_sample = true;
    }
    else
    {
      byteADC1 = ADCH;               // get ADC channel 1
      cbi(ADMUX, MUX0);              // set multiplexer to channel 0
    }
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t"); //No operations to create a delay
    sbi(ADCSRA, ADSC);             // start next conversion
  }
}
