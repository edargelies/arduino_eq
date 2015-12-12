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

#include <Filters.h>
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

boolean div32;
boolean div16;
// interrupt variables accessed globally
volatile boolean f_sample;
volatile byte byteADC0;
volatile byte byteADC1;
volatile byte byteADC2;
//volatile byte noop;

int buffIndex;
int buffIndex2;
float frequency;

int audioIn; // The reading from analog pin 0
int audioPhased;
int originalAudio;

int effectIn; // The reading from analog pin 1
byte buffVal;

byte delayBuff[512];  // Audio Memory Array 8-Bit

enum effect {
  reverb,
  stutter, // delay, but delay is a keyword in Arduino
  phaser
};
effect soundEffect;

void setup()
{
  Serial.begin(57600);        // connect to the serial port
  Serial.print("Arduino Audio Filter");

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

void filter() {
  frequency = byteADC1;
  FilterOnePole filterOneLowpass( LOWPASS, 2000 );
  while (true) {
  //  filterOneLowpass.input( byteADC0 );
    //    OCR2A = filterOneLowpass.output();          // Sample Value to PWM Output
    OCR2A = byteADC0;
  }
}
void loop()
{
  filter();
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

String effectToString(effect type) {
  if (type == reverb) {
    return "Reverb";
  }
  else if (type == stutter) {
    return "Delay";
  }
  else if (type == phaser) {
    return "Phaser";
  }
  else {
    return "Undefined";
  }
}

