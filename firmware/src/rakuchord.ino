// rakuchord_pencase extend
// fork 2014.11.30
//  pencase: 168 (compatible)
//  nenga: 328 (compatible)

// rakuchord first - 0
// rakuchord rev2

#include <SPI.h>
#include <Wire.h>
#include <U8x8lib.h>
#include <tones.h>
#include <MultiTunes.h>

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);

byte trigger[] = {
  0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0
};

// =========================================-
#define M_PLAY 0
#define M_STEP 1
#define M_SYNTH 2
#define M_MENV 3
#define M_CENV 4
#define M_ALPE 5
byte gmode = M_PLAY;
boolean gsetting = false;

char octave = 1;
char croctave = 1; // 和音相対オクターブ
char shiftTone = 12;
char shiftMagic = 0;
#define M_NONE 0
#define M_MAJOR 1
#define M_MINOR 2
#define M_SUS4 3
byte shiftMode = M_NONE;

byte apos = 0;
byte aseq[]= {1, 2, 3, 0, 1, 2, 3, 0};
boolean galpe = false;

unsigned int rythmCount = 0;
unsigned int alpeCount = 0;

int aV[] = {0,0,0,0};

inline int getTone(byte no){
  return  (tones[shiftMagic + magic[no] + shiftTone]) << (octave);
}

void setTone(byte no){
  vol[0] = 14;
  d[0] = getTone(no);
  if(vf){ // detune
    vol[4] = 8;
    d[4] = getTone(no) + (getTone(no) >> 6);
  }
}
#define LOW_VOL 6
#define MID_VOL 12
#define ALPE_VOL 14

int getMiddleMajorTone(byte no){
  switch(shiftMode){
    case M_NONE: return getTone(no + 2);
    case M_MAJOR:return getTone(no + 2);
    case M_MINOR:return (tones[shiftMagic + magic[no+2] - 1 + shiftTone]) << octave;
    case M_SUS4: return (tones[shiftMagic + magic[no+2] + 1 + shiftTone]) << octave;
  }
  return 0; //error
}

int getMiddleMinorTone(byte no){
  switch(shiftMode){
    case M_NONE: return getTone(no + 2);
    case M_MINOR:return getTone(no + 2);
    case M_MAJOR:return (tones[shiftMagic + magic[no+2] + 1 + shiftTone]) << octave;
    case M_SUS4: return (tones[shiftMagic + magic[no+2] + 2 + shiftTone]) << octave;
  }
  return 0; //error
}

int shift(int n, int s){
  if(s > 0)return n >> s;
  return n << (-s);
}

void setChord(byte no){
  char buf[32];
  static byte pre = 0;
  if(pre != no){
    pre = no;
    sprintf(buf, "chord %d", no);
    TIMSK1 = 0;
    lcdDraw(buf);
    TIMSK1 = 1<<TOIE1;
  }

  if(galpe){
    byte tmp = magic[no + 2] - magic[no];
    unsigned int middle;
    switch(tmp){
        case 4: middle = getMiddleMajorTone(no);break;
        case 3: middle = getMiddleMinorTone(no);break;
    }
    unsigned int alpeTones[] = {getTone(no) >> 1, getTone(no), middle, getTone(no + 4), 0};

    d[1] = shift(alpeTones[aseq[apos]],croctave);
    vol[1] = ALPE_VOL;

  }else{
    // not alpe mode
    vol[1] = LOW_VOL;
    d[1] = shift(getTone(no),croctave);

    vol[2] = MID_VOL;
    byte tmp = magic[no + 2] - magic[no];
    switch(tmp){
      case 4: d[2] = shift(getMiddleMajorTone(no),croctave); break;
      case 3: d[2] = shift(getMiddleMinorTone(no),croctave); break;
    }

    vol[3] = LOW_VOL;
    d[3] = shift(getTone(no + 4),croctave);
  }
}
void setShift(byte no){
  switch(no){
    case 6:
      break;
    case 3: shiftTone -=1;break;
    case 4: shiftTone = 12;break;
    case 5: shiftTone +=1;break;
    case 0: shiftMode = M_SUS4;break;
    case 1: shiftMode = M_MAJOR;break;
    case 2: shiftMode = M_MINOR;break;
  }
}
void setSynthShift(byte no){
  switch(no){
    case 6:
      break;
    case 0: croctave = -3;break;
    case 1: croctave = -2;break;
    case 2: croctave = -1;break;
    case 3: croctave = 0;break;
    case 4: croctave = 1;break;
    case 5: croctave = 2;break;
  }
}


#define DSPPED 1200
unsigned int rSpeed = DSPPED;
int arSpeed = 0; // alpe speed
byte mEnvMode = 0;
byte cEnvMode = 0;

byte rpos = 0;
byte rseq[]= {
  0x3f,
  0xf3 | 0x80,
  0,
  0x3f,
  0x3f,
  0xf3 | 0x80,
  0,
  0x3f
  };
boolean grythm = false;

void setStepShift(byte no){
  switch(no){
    case 6:
      break;
    case 0: rSpeed -=64;break;
    case 1: rSpeed = DSPPED;break;
    case 2: rSpeed +=64;break;
  }
}
void setAlpeShift(byte no){
  switch(no){
    case 6:
      break;
    case 0: arSpeed = 2; alpeCount = 0;apos = 0; rythmCount = 0;rpos = 0;break;
    case 1: arSpeed = 1; alpeCount = 0;apos = 0; rythmCount = 0;rpos = 0;break;
    case 2: arSpeed = 0; alpeCount = 0;apos = 0; rythmCount = 0;rpos = 0;break;
    case 3: arSpeed = -1; alpeCount = 0;apos = 0; rythmCount = 0;rpos = 0;break;
    case 4: arSpeed = -2;alpeCount = 0;apos = 0; rythmCount = 0;rpos = 0;break;
    case 5: arSpeed = -3; alpeCount = 0;apos = 0; rythmCount = 0;rpos = 0;break;
  }
}

void setStep(byte n){
  switch(rseq[n]){
    case 0:  rseq[n] = 0xf; break;

    case 0xf:  rseq[n] = 0x1f; break;
    case 0x1f:  rseq[n] = 0x3f; break;
    case 0x3f:  rseq[n] = 0xf | 0x80; break;

    case 0xf | 0x80:  rseq[n] = 0x1f | 0x80; break;
    case 0x1f | 0x80:  rseq[n] = 0x3f | 0x80; break;
    case 0x3f | 0x80:  rseq[n] = 0x00; break;
  }
}
void clearStep(){
  for(byte i = 0; i < 8; i ++){
    rseq[i] = 0;
  }
}

void setAStep(byte n){
  aseq[n]++;
  if(aseq[n] > 4){
    aseq[n] = 0;
  }
}
void clearAStep(){
  for(byte i = 0; i < 8; i ++){
    aseq[i] = 4;
  }
}

#define SAMPLE 64

void mkWave(byte type){
  for(byte i=0;i < 16; i++){
    for(byte j=0;j < SAMPLE; j++){
      switch(type){
        case 0:
          wave[i][j] = i*j/(SAMPLE); // i : vol, j : t
          break;
        case 1:
          wave[i][j] = 2 * i * ((j & (SAMPLE>>1))?8:0)/(16+8);  // 8 : 0B100
          break;
        case 2:
          wave[i][j] = i*((sin(j*2*3.14/SAMPLE)+1)/2 * 8)/16;
          break;
      }
    }
  }
}

void triggerOn(byte n){
  if(trigger[n] == 0){
    // event on
    if(n == 27){
      gsetting = true;
    }
  }
  if(!gsetting){
    switch(gmode){
      case M_PLAY:
        if(trigger[n] == 0){
          // event on
        }
        if(n < 14){
          setTone(n);
        }else if(n < 21){
          setChord(n - 14);
        }else{
          if(n < 28){
            if(trigger[n] == 0){
              setShift(n - 21);
            }
          }
        }
        break;
      case M_STEP:
        if(trigger[n] == 0){
          // event on
          if(n == 0){
            clearStep();
          }
          if(n < 14){
            if(n >= 6){
              setStep(n - 6);
            }
          }
          if(n < 28){
            setStepShift(n - 21);
          }
        }
        if(14 <= n && n < 21){
          setChord(n - 14);
        }
        break;
      case M_SYNTH:
        if(trigger[n] == 0){
          switch(n){
            case 0: mkWave(0);break;
            case 1: mkWave(1);break;
            case 2: mkWave(2);break;
          }
        }
        if(n >= 7 && n < 14){
          setTone(n);
        }else if(n < 21){
          setChord(n - 14);
        }else{
          if(n < 28){
            if(trigger[n] == 0){
              setSynthShift(n - 21);
            }
          }
        }
        break;
      case M_MENV:
        if(trigger[n] == 0){
          switch(n){
            case 0: mEnvMode = 0;break;
            case 1: mEnvMode = 1;break;
            case 2: mEnvMode = 2;break;
            case 3: mEnvMode = 3;break;
          }
        }
        if(n >= 7 && n < 14){
          setTone(n);
        }else if(14 <= n && n < 21){
          setChord(n - 14);
        }
        break;
      case M_CENV:
        if(trigger[n] == 0){
          switch(n){
            case 0: cEnvMode = 0;break;
            case 1: cEnvMode = 1;break;
            case 2: cEnvMode = 2;break;
            case 3: cEnvMode = 3;break;
          }
        }
        if(n >= 7 && n < 14){
          setTone(n);
        }else if(n < 21){
          setChord(n - 14);
        }
        break;
      case M_ALPE:
        if(trigger[n] == 0){
          // event on
          if(n == 0){
            clearAStep();
          }
          if(n < 14){
            if(n >= 6){
              setAStep(n - 6);
            }
          }
          if(n < 28){
            setAlpeShift(n - 21);
          }
        }
        if(14 <= n && n < 21){
          setChord(n - 14);
        }
        break;
    }
  }else{ // gsetting
    if(trigger[n] == 0){
      switch(n){
        case 0: galpe = false;break;
        case 1: galpe = true;break;
        case 2: grythm = false;break;
        case 3: grythm = true;break;
        case 4: vf = 0;break;
        case 5: vf = 1;break;
        case 14:gmode = M_PLAY;break;
        case 15:gmode = M_STEP;break;
        case 16:gmode = M_SYNTH;break;
        case 17:gmode = M_MENV;break;
        case 18:gmode = M_CENV;break;
        case 19:gmode = M_ALPE;break;
      }
    }
  }
  if(trigger[n] < 256){
    trigger[n] ++;
  }
}
void triggerOff(byte n){
  byte count = 0;
  if(trigger[n] != 0){
    trigger[n] = 0;
    if(n >= 21){
      shiftMode = M_NONE;
    }
    if(n == 27){
      gsetting = false;
    }
  }
}

void lcdSetup(){
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0,1,"Hello");
  u8x8.drawString(0,2,"RakuChord");
}

void lcdDraw(char* s){
  u8x8.drawString(0,4, s);
}


void setup(){
  soundSetup();
  lcdSetup();
  
  mkWave(0);
  
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(7, HIGH);
  
  pinMode(8, INPUT);
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  pinMode(12, INPUT);
  pinMode(13, INPUT);
  pinMode(14, INPUT);
  pinMode(15, INPUT);
  
  // pull up
  digitalWrite(8, HIGH);
  digitalWrite(10, HIGH);
  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);
  digitalWrite(14, HIGH);
  digitalWrite(15, HIGH);

  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  
  // boot sound
  d[1] = getTone(0);
  vol[1] = 12;
  d[2] = getTone(2);
  vol[2] = 12;
  d[3] = getTone(4);
  vol[3] = 12;
  d[4] = getTone(0) >> 1;
  vol[4] = 12;
}

int count = 0;
byte bcount = 0;
byte rcount = 0;
byte bscan = 0;

void loop(){
  if(rythmCount == 0){
    // rythm loop
    if(grythm){
      if(rseq[rpos]){
        if((rseq[rpos] & 0x80) == 0x80){
          nf2 = rseq[rpos] & 0x7f;
        }else{
          nf = rseq[rpos];
        }
      }
    }
    rpos = (rpos + 1) & B0111;
  }
  if(alpeCount == 0){
    apos = (apos + 1) & B0111;
  }

  if(rcount == 0){
    // rethm decay
    if(nf != 0){
      nf = (nf >> 1);
    }
    if(nf2 != 0){
      nf2 = (nf2 >> 1);
    }
  }

  if(count == 0){
    //melody decay
    switch(mEnvMode){
      case 0:  // fast - normal
        if(vol[0] > 4){vol[0] -= 2;}else if(vol[0] > 0)vol[0]--;
      break;
      case 1: // normal
        if(vol[0] > 0)vol[0]--;
      break;
      case 2: // fast
        if(vol[0] > 2)vol[0]-=2;
        if(vol[0] > 0) vol[0] --;
      break;
      case 3: // f-fast
        if(vol[0] > 4)vol[0] -=4;
        if(vol[0] > 0) vol[0] --;
      break;
    }
    switch(cEnvMode){
      // chord decay
      case 0:  // fast - normal
        if(vol[1] > 4){vol[1] -= 2;}else if(vol[1] > 0)vol[1]--;
        if(vol[2] > 4){vol[2] -= 2;}else if(vol[2] > 0)vol[2]--;
        if(vol[3] > 4){vol[3] -= 2;}else if(vol[3] > 0)vol[3]--;
        if(vol[4] > 4){vol[4] -= 2;}else if(vol[4] > 0)vol[4]--;
        break;
      case 1:  // normal
        if(vol[1] > 0)vol[1]--;
        if(vol[2] > 0)vol[2]--;
        if(vol[3] > 0)vol[3]--;
        if(vol[4] > 0)vol[4]--;
        break;
      case 2:  // fast
        if(vol[1] > 2)vol[1]-=2;
        if(vol[1] > 0)vol[1] --;
        if(vol[2] > 2)vol[2]-=2;
        if(vol[2] > 0)vol[2] --;
        if(vol[3] > 2)vol[3]-=2;
        if(vol[3] > 0)vol[3] --;
        if(vol[4] > 2)vol[4]-=2;
        if(vol[4] > 0)vol[4] --;
        break;
      case 3:  // f-fast
        if(vol[1] > 4)vol[1] -=4;
        if(vol[1] > 0)vol[1] --;
        if(vol[2] > 4)vol[2] -=4;
        if(vol[2] > 0)vol[2] --;
        if(vol[3] > 4)vol[3] -=4;
        if(vol[3] > 0)vol[3] --;
        if(vol[4] > 4)vol[4] -=4;
        if(vol[4] > 0)vol[4] --;
        break;
    }
  }
  if(bcount == 0){
    // button scan loop
    bscan++;
    if(bscan == 8){
      bscan = 0;
    }
    switch(bscan){
      case 0:
        pinMode(4, INPUT);
        digitalWrite(4, HIGH);
        pinMode(5, INPUT);
        digitalWrite(5, HIGH);
        pinMode(6, INPUT);
        digitalWrite(6, HIGH);
        pinMode(7, OUTPUT);
        digitalWrite(7, LOW);
        break;
      case 1:
        pinMode(4, INPUT);
        digitalWrite(4, HIGH);
        pinMode(5, OUTPUT);
        digitalWrite(5, LOW);
        pinMode(6, INPUT);
        digitalWrite(6, HIGH);
        pinMode(7, INPUT);
        digitalWrite(7, HIGH);
      break;
      case 2:
        pinMode(4, INPUT);
        digitalWrite(4, HIGH);
        pinMode(5, INPUT);
        digitalWrite(5, HIGH);
        pinMode(6, OUTPUT);
        digitalWrite(6, LOW);
        pinMode(7, INPUT);
        digitalWrite(7, HIGH);
      break;
      case 3:
        pinMode(4, OUTPUT);
        digitalWrite(4, LOW);
        pinMode(5, INPUT);
        digitalWrite(5, HIGH);
        pinMode(6, INPUT);
        digitalWrite(6, HIGH);
        pinMode(7, INPUT);
        digitalWrite(7, HIGH);
      break;

    }
  }

  // button read
  boolean b1 = digitalRead(15);
  boolean b2 = digitalRead(14);
  boolean b3 = digitalRead(13);
  boolean b4 = digitalRead(12);
  boolean b5 = digitalRead(11);
  boolean b6 = digitalRead(10);
  boolean b7 = digitalRead(8);
  switch(bscan){
    case 0:
      if(!b1){triggerOn(21);}else{triggerOff(21);}
      if(!b2){triggerOn(22);}else{triggerOff(22);}
      if(!b3){triggerOn(23);}else{triggerOff(23);}
      if(!b4){triggerOn(24);}else{triggerOff(24);}
      if(!b5){triggerOn(25);}else{triggerOff(25);}
      if(!b6){triggerOn(26);}else{triggerOff(26);}
      if(!b7){triggerOn(27);}else{triggerOff(27);}
    break;
    case 3:
      if(!b1){triggerOn(0);}else{triggerOff(0);}
      if(!b2){triggerOn(1);}else{triggerOff(1);}
      if(!b3){triggerOn(2);}else{triggerOff(2);}
      if(!b4){triggerOn(3);}else{triggerOff(3);}
      if(!b5){triggerOn(4);}else{triggerOff(4);}
      if(!b6){triggerOn(5);}else{triggerOff(5);}
      if(!b7){triggerOn(6);}else{triggerOff(6);}
    break;
    case 1:
      if(!b1){triggerOn(7);}else{triggerOff(7);}
      if(!b2){triggerOn(8);}else{triggerOff(8);}
      if(!b3){triggerOn(9);}else{triggerOff(9);}
      if(!b4){triggerOn(10);}else{triggerOff(10);}
      if(!b5){triggerOn(11);}else{triggerOff(11);}
      if(!b6){triggerOn(12);}else{triggerOff(12);}
      if(!b7){triggerOn(13);}else{triggerOff(13);}
    break;
    case 2:
      if(!b1){triggerOn(14);}else{triggerOff(14);}
      if(!b2){triggerOn(15);}else{triggerOff(15);}
      if(!b3){triggerOn(16);}else{triggerOff(16);}
      if(!b4){triggerOn(17);}else{triggerOff(17);}
      if(!b5){triggerOn(18);}else{triggerOff(18);}
      if(!b6){triggerOn(19);}else{triggerOff(19);}
      if(!b7){triggerOn(20);}else{triggerOff(20);}
    break;
  }

  // counter management
  rcount = (rcount + 1) & 0x7f;
  bcount = (bcount + 1) & 0xf;
  count = (count + 1) & 0x1ff;
  rythmCount = (rythmCount + 1);
  if(rythmCount > rSpeed){
    rythmCount = 0;
  }
  alpeCount = (alpeCount + 1);
  if(alpeCount > (shift(rSpeed, arSpeed))){
    alpeCount = 0;
  }
}
