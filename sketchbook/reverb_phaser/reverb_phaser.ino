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
  soundEffect = reverb;  // This is how you select an effect from the above enum
  Serial.begin(57600);        // connect to the serial port
  Serial.print("Arduino Audio ");Serial.println(soundEffect);

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
  if ((soundEffect ==  reverb) || (soundEffect == stutter)) { //Reverb or delay/stutter
    while (!f_sample) {     // wait for Sample Value from ADC
    }                       // Cycle 15625 KHz = 64uSec

    f_sample = false;

    buffVal = delayBuff[buffIndex] ;              // read the delay buffer
    effectIn = 127 - buffVal ;              // substract offset
    effectIn = effectIn * byteADC1 / 255;     // scale delayed sample with potentiometer

    originalAudio = byteADC0;
    audioIn = 127 - originalAudio;          // substract offset from new sample
    audioIn = audioIn + effectIn;                 // add delayed sample and new sample
    if (audioIn < -127) audioIn = -127; // Audio limiter
    if (audioIn > 127) audioIn = 127;   // Audio limiter

    buffVal = 127 + audioIn;                // add offset
    if (soundEffect == reverb) { // reverb
      delayBuff[buffIndex] = originalAudio;                // store sample in audio buffer      
    }
    else {  // delay/stutter
      delayBuff[buffIndex] = buffVal;
    }

    buffIndex++;
    buffIndex = buffIndex & 511;         // limit bufferindex 0..511

    OCR2A = buffVal;            // Sample Value to PWM Output
  }
  else { //Phaser
    while (!f_sample) {     // wait for Sample Value from ADC
    }                       // Cycle 15625 KHz = 64uSec
    f_sample = false;

    buffVal = byteADC0;
    delayBuff[buffIndex] = buffVal;        // write to buffer
    audioIn = delayBuff[buffIndex2];            // read the delay buffer

    byteADC1 = byteADC1 / 20;        // limit poti value to 512
    buffIndex++;

    buffIndex2 = buffIndex - byteADC1;


    buffIndex2 = buffIndex2 & 511;         // limit index 0..511
    buffIndex = buffIndex & 511;         // limit index 0..511

    audioPhased = audioIn + buffVal;
    audioPhased = audioPhased / 2;

    buffVal = audioPhased;

    OCR2A = buffVal;          // Sample Value to PWM Output

  }
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
    // noop++;
    // noop--;
    // noop++;
    // noop--;    // short delay before start conversion
    sbi(ADCSRA, ADSC);             // start next conversion
  }
}
