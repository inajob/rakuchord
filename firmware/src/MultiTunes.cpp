#include <Arduino.h>
#include <MultiTunes.h>

volatile unsigned int d[5]; // default value( for debug)
volatile unsigned int dn[5];// work variable
volatile unsigned char vol[5]; // volume par channel
volatile byte nf;
volatile byte nf2;
int noise;
int noise2;

volatile byte vf;

byte wave[16][64];


const unsigned int timerLoadValue = 220;
volatile unsigned int level = 0;
//volatile unsigned int lfo = 0;

volatile byte nv = 5;

void soundSetup(){
  //mkWave(0);

  pinMode(9,OUTPUT);
  // 3-pwm ,pwm
  TCCR1A = _BV(COM1A1) | _BV(WGM10);
  // clk/1 prescale
  TCCR1B = /*_BV(WGM12) |*/ _BV(CS10); //1  		31250
  //TCCR2B = _BV(CS21); //8 	 	3906.25
  //1<<CS22 | 0<<CS21 | 1<<CS20; // clk/128 prescale
  //TCCR2B = TCCR2B & 0b11100000 | 0b00001; // pwm: 31250Hz
  TIMSK1 = 1<<TOIE1; // Timer/Counter2 Overflow Interrupt Enable
  OCR1A = 80; // for debug
  TCNT1 = timerLoadValue;

  vol[0] = vol[1] = vol[2] = vol[3] = vol[4] = 0;
  d[0] = d[1] = d[2] = d[3] = d[4] = 0;
  dn[0] = dn[1] = dn[2] = dn[3] = dn[4] = 0;
}

// sample 16 -> 12
// sample 32 -> 11
// sample 64 -> 10
volatile byte realcount = 0;
ISR(TIMER1_OVF_vect) {    // Timer/Counter1 Overflow
  TCNT2 = timerLoadValue; // Reset the timer
  dn[0] = dn[0] + d[0];/* + vf * ((*(wave[15] + (lfo >> 10))) << 2);*/
  dn[1] = dn[1] + d[1];
  dn[2] = dn[2] + d[2];
  dn[3] = dn[3] + d[3];
  dn[4] = dn[4] + d[4];
  /*
  level = ((dn[0]&(1<<14))?vol[0]:0) +
          ((dn[1]&(1<<14))?vol[1]:0) +
          ((dn[2]&(1<<14))?vol[2]:0) +
          ((dn[3]&(1<<14))?vol[3]:0) +
          ((dn[4]&(1<<14))?vol[4]:0) + (noise & nf);
  */

  if((realcount & 0x3f) == 0x3f)noise = (noise>>1) + ((bitRead(noise, 13) xor bitRead(noise, 3) xor 1) << 15);
  noise2 = 2100005341  * noise2 + 1651869;
 realcount ++;
  // wave table (slow)
  //lfo+=lfop;
  level = wave[vol[0]][dn[0]>>10] +
          wave[vol[1]][dn[1]>>10] +
          wave[vol[2]][dn[2]>>10] +
          wave[vol[3]][dn[3]>>10] +
          wave[vol[4]][dn[4]>>10] + ((noise) & nf) + ((noise2>>8) &nf2);
          ;
  OCR1A = level;
}

