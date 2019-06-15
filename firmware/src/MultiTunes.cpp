#include <Arduino.h>
#include "MultiTunes.h"

volatile unsigned int d[5] = { 0 }; // default value( for debug)
static unsigned int dn[5] = { 0 };// work variable
volatile unsigned char vol[5] = { 0 }; // volume par channel
volatile byte nf;
volatile byte nf2;
int noise;
int noise2;

volatile byte vf;

byte wave[8][64];


const unsigned int timerLoadValue = 220;
//volatile unsigned int lfo = 0;

volatile byte nv = 5;

extern unsigned int rythmCount;
extern unsigned int alpeCount;

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
}

// sample 16 -> 12
// sample 32 -> 11
// sample 64 -> 10
ISR(TIMER1_OVF_vect) {    // Timer/Counter1 Overflow
  static byte realcount = 0;
  static byte timeCount = 0;
  constexpr byte interruptsPerTick = 8;

  TCNT2 = timerLoadValue; // Reset the timer
  dn[0] = dn[0] + d[0];/* + vf * ((*(wave[15] + (lfo >> 10))) << 2);*/
  dn[1] = dn[1] + d[1];
  dn[2] = dn[2] + d[2];
  dn[3] = dn[3] + d[3];
  dn[4] = dn[4] + d[4];

  if((realcount & 0x3f) == 0x3f)noise = (noise>>1) + ((bitRead(noise, 13) xor bitRead(noise, 3) xor 1) << 15);
  noise2 = 2100005341  * noise2 + 1651869;
  realcount ++;
  // wave table (slow)
  //lfo+=lfop;

  unsigned char s[5];
  unsigned char v, sample;
#define GET_WAVE_SAMPLE(i)                           \
    (v = vol[i],                                     \
     (v != 0) ? \
       sample = wave[v / 2][dn[i] >> 10],              \
       ((v & 1) ? sample >> 4 : sample & 0xf)    \
     : 0)

  s[0] = GET_WAVE_SAMPLE(0);
  s[1] = GET_WAVE_SAMPLE(1);
  s[2] = GET_WAVE_SAMPLE(2);
  s[3] = GET_WAVE_SAMPLE(3);
  s[4] = GET_WAVE_SAMPLE(4);

  const unsigned int level =
          s[0] + s[1] + s[2] + s[3] + s[4] +
          ((noise) & nf) + ((noise2>>8) &nf2);
  OCR1A = level;

  // Update time counters
  if (++timeCount == interruptsPerTick) {
    rythmCount++;
    alpeCount++;
    timeCount = 0;
  }
}

