// rakuchord_pencase extend
// fork 2014.11.30
//  pencase: 168 (compatible)
//  nenga: 328 (compatible)

// rakuchord first - 0
// rakuchord rev2

#include <SPI.h>
#include <Wire.h>
#include <tones.h>
#include <MultiTunes.h>
#include <font.h>

#define ADDRESS_OLED 0x3C

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

struct Setting {
  char octave;
  char croctave;
  byte waveType;
  byte aseq[8];
  bool galpe;
  unsigned int rSpeed;
  int arSpeed;
  byte mEnvMode;
  byte cEnvMode;
  byte rseq[8];
  bool grythm;
  bool vf;
};

struct Setting preset[2] ={
  {
   1, // octave
   1, // croctave
   0, // waveType
   {1, 2, 3, 0, 1, 2, 3, 0}, // aseq
   false, // galpe
   1200, // rSpeed
   0, // arSpeed
   0, // mEnvMode
   0, // cEnvMode
   {  // rseq
     0x3f,
     0x3f,
     0x3f | 0x80,
     0,
     0x3f,
     0x3f,
     0x3f | 0x80,
     0
   },
   false, // grythm
   false, // vf
  },
  {
   1, // octave
   0, // croctave
   1, // waveType
   {1, 2, 3, 0, 1, 2, 3, 0}, // aseq
   true, // galpe
   1200, // rSpeed
   0, // arSpeed
   0, // mEnvMode
   0, // cEnvMode
   {  // rseq
     0x3f,
     0x3f,
     0x3f | 0x80,
     0,
     0x3f,
     0x3f,
     0x3f | 0x80,
     0
   },
   true, // grythm
   false, // vf
  },

};

char octave = 1;
char croctave = 1; // 和音相対オクターブ
char shiftTone = 12;
char shiftMagic = 0;
#define M_NONE 0
#define M_MAJOR 1
#define M_MINOR 2
#define M_SUS4 3
byte shiftMode = M_NONE;
byte glideMode = 0;
byte waveType = 0;

byte apos = 0;
byte aseq[]= {1, 2, 3, 0, 1, 2, 3, 0};
boolean galpe = false;

unsigned int rythmCount = 0;
unsigned int alpeCount = 0;

int aV[] = {0,0,0,0};
byte toneCount = 0;

#define DSPPED 1200
unsigned int rSpeed = DSPPED;
int arSpeed = 0; // alpe speed
byte mEnvMode = 0;
byte cEnvMode = 0;

byte rpos = 1;
byte rseq[]= {
  0x3f,
  0x3f,
  0x3f | 0x80,
  0,
  0x3f,
  0x3f,
  0x3f | 0x80,
  0
  };
boolean grythm = false;

void loadSetting(struct Setting *setting){
  octave = setting->octave;
  croctave = setting->croctave;
  waveType = setting->waveType;
  memcpy(aseq, setting->aseq, 8);
  galpe = setting->galpe;
  rSpeed = setting->rSpeed;
  arSpeed = setting->arSpeed;
  mEnvMode = setting->mEnvMode;
  cEnvMode = setting->cEnvMode;
  memcpy(rseq, setting->rseq, 8);
  grythm = setting->grythm;
  vf = setting->vf;

  mkWave(waveType);
}

inline int getTone(byte no){
  return  (tones[shiftMagic + magic[no] + shiftTone]) << (octave);
}

void setTone(byte no, byte p){
  vol[0] = 14;
  d[0] = getTone(no) + ((glideMode == 1)?(p-255):0);
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
  static byte preAdd = M_NONE;
  if(pre != no || preAdd != shiftMode){
    pre = no;
    preAdd = shiftMode;
    sprintf(buf, "chord %d", no);
    TIMSK1 = 0;
    lcdDraw(no, shiftMode);
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
    case 3: shiftTone -=1; drawDisplay();break;
    case 4: shiftTone = 12; drawDisplay();break;
    case 5: shiftTone +=1; drawDisplay();break;
    //case 3: shiftTone -=1;break;
    //case 4: shiftTone = 12;break;
    //case 5: glideMode = 1;break;
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
  drawDisplay();
}

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
    case 0: arSpeed = 2; alpeCount = 0;apos = 0; rythmCount = 0;rpos = 1;break;
    case 1: arSpeed = 1; alpeCount = 0;apos = 0; rythmCount = 0;rpos = 1;break;
    case 2: arSpeed = 0; alpeCount = 0;apos = 0; rythmCount = 0;rpos = 1;break;
    case 3: arSpeed = -1; alpeCount = 0;apos = 0; rythmCount = 0;rpos = 1;break;
    case 4: arSpeed = -2;alpeCount = 0;apos = 0; rythmCount = 0;rpos = 1;break;
    case 5: arSpeed = -3; alpeCount = 0;apos = 0; rythmCount = 0;rpos = 1;break;
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

void drawText(char* buf, byte y){
  Wire.beginTransmission(ADDRESS_OLED);
  Wire.write(0b10000000); //control byte, Co bit = 1, D/C# = 0 (command)
  Wire.write(0xB0 + y); //set page start address(B0～B7)
  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
  Wire.write(0x21); //set Column Address
  Wire.write(0); //Column Start Address(0～127)
  Wire.write(127); //Column Stop Address
  Wire.endTransmission();

  while(*buf != 0){
    Wire.beginTransmission(ADDRESS_OLED);
    Wire.write(0b01000000);
    for(int i = 0; i < 4; i ++){
      Wire.write(pgm_read_byte_near(&(font[*buf-' '][i])));
    }
    Wire.write(0x00);
    Wire.endTransmission();
    buf++;
  }
}

char getMEnvModeChar(){
  switch(mEnvMode){
    case 0: return 'N';
    case 3: return 'F';
  }
}
char getCEnvModeChar(){
  switch(cEnvMode){
    case 0: return 'N';
    case 3: return 'F';
  }
}

void drawDisplay(){
  drawText("=RAKU CHORD=",0);
  char buf[13];
  char tmp[13];
 
  // line 2
  switch(waveType){
    case 0:strncpy(tmp, "SAW", 13);break;
    case 1:strncpy(tmp, "SQU", 13);break;
    case 2:strncpy(tmp, "SAW", 13);break;
  }
  sprintf(buf, "~%3s %c s%02do%02d", tmp, vf?'D':'*', shiftTone, croctave);
  drawText(buf, 2);

  // line 3, 4 :rythm
  drawRythm();
  if(grythm){
    strncpy(tmp, "ON", 13);
  }else{
    strncpy(tmp, "OFF", 13);
  }
  sprintf(buf, " %3s %04d", tmp, rSpeed);
  drawText(buf, 4);

  // line 5, 6 :alpe
  drawAlpe();
  if(galpe){
    strncpy(tmp, "ON", 13);
  }else{
    strncpy(tmp, "OFF", 13);
  }
  sprintf(buf, " %3s SP:%02d", tmp, arSpeed);
  drawText(buf, 6);


  sprintf(buf, "(M:%c, C:%c)", getMEnvModeChar(), getCEnvModeChar());
  drawText(buf, 7);
  
  // line7
  switch(gmode){
    case M_PLAY: 
      drawText(">> PLAY      ",1);
      break;
    case M_STEP: 
      drawText(">> STEP      ",1);
      break;
    case M_SYNTH:
      drawText(">> SYNTH     ",1);
      break;
    case M_MENV: 
      drawText(">> MELODY ENV",1);
      break;
    case M_CENV: 
      drawText(">> CHORD ENV ",1);
      break;
    case M_ALPE: 
      drawText(">> ALPE      ",1);
      break;
  }
}

void drawRythm(){
  char buf[13];
  buf[0] = 'R';
  buf[1] = ' ';
  buf[2] = '[';
  for(byte i = 0; i < 8; i ++){
    switch(rseq[i]){
      case 0x00:      buf[i + 3]='_';break;

      case 0x0f:      buf[i + 3]='A';break;
      case 0x1f:      buf[i + 3]='B';break;
      case 0x3f:      buf[i + 3]='C';break;

      case 0x8f|0x80: buf[i + 3]='1';break;
      case 0x9f|0x80: buf[i + 3]='2';break;
      case 0xbf|0x80: buf[i + 3]='3';break;
    }
  }
  buf[11] = ']';
  buf[12] = 0;
  drawText(buf, 3);
}

void drawAlpe(){
  char buf[13];
  buf[0] = 'A';
  buf[1] = ' ';
  buf[2] = '[';
  for(byte i = 0; i < 8; i ++){
    switch(aseq[i]){
      case 0: buf[i + 3]='0';break;
      case 1: buf[i + 3]='1';break;
      case 2: buf[i + 3]='2';break;
      case 3: buf[i + 3]='3';break;
      case 4: buf[i + 3]='_';break;
    }
  }
  buf[11] = ']';
  buf[12] = 0;
  drawText(buf, 5);
}

#define SAMPLE 64

void mkWave(byte type){
  waveType = type;
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
          setTone(n, trigger[n]);
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
          drawDisplay();
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
          drawDisplay();
        }
        if(n >= 7 && n < 14){
          setTone(n, trigger[n]);
        }else if(n >=7 && n < 21){
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
            //case 1: mEnvMode = 1;break;
            //case 2: mEnvMode = 2;break;
            case 1: mEnvMode = 3;break;
          }
          drawDisplay();
        }
        if(n >= 7 && n < 14){
          setTone(n, trigger[n]);
        }else if(14 <= n && n < 21){
          setChord(n - 14);
        }
        break;
      case M_CENV:
        if(trigger[n] == 0){
          switch(n){
            case 0: cEnvMode = 0;break;
            //case 1: cEnvMode = 1;break;
            //case 2: cEnvMode = 2;break;
            case 1: cEnvMode = 3;break;
          }
          drawDisplay();
        }
        if(n >= 7 && n < 14){
          setTone(n, trigger[n]);
        }else if(14 <= n && n < 21){
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
          drawDisplay();
        }
        if(14 <= n && n < 21){
          setChord(n - 14);
        }
        break;
    }
  }else{ // gsetting
    if(trigger[n] == 0){
      switch(n){
        case 0: galpe = false;  break;
        case 1: galpe = true;   break;
        case 2: grythm = false; break;
        case 3: grythm = true;  break;
        case 4: vf = 0;         break;
        case 5: vf = 1;         break;

        case 7: loadSetting(&preset[0]);break;
        case 8: loadSetting(&preset[1]);break;

        case 14:gmode = M_PLAY; break;
        case 15:gmode = M_STEP; break;
        case 16:gmode = M_SYNTH;break;
        case 17:gmode = M_MENV; break;
        case 18:gmode = M_CENV; break;
        case 19:gmode = M_ALPE; break;
      }
      drawDisplay();
    }
  }
  if(trigger[n] < 255){
    trigger[n] ++;
  }
}
void triggerOff(byte n){
  byte count = 0;
  if(trigger[n] != 0){
    trigger[n] = 0;
    if(n >= 21){
      shiftMode = M_NONE;
      glideMode = 0;
    }
    if(n == 27){
      gsetting = false;
    }
  }
}

void lcdSetup(){
  Wire.begin();
  Wire.setClock(400000L);
  delay(100);
    
  Wire.beginTransmission(ADDRESS_OLED);
  Wire.write(0b10000000); //control byte, Co bit = 1, D/C# = 0 (command)
  Wire.write(0xAE); //display off
  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
  Wire.write(0xA8); //Set Multiplex Ratio  0xA8, 0x3F
  Wire.write(0b00111111); //64MUX
  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
  Wire.write(0xD3); //Set Display Offset 0xD3, 0x00
  Wire.write(0x00);
  Wire.write(0b10000000); //control byte, Co bit = 1, D/C# = 0 (command)
  Wire.write(0x40); //Set Display Start Line 0x40
  Wire.write(0b10000000); //control byte, Co bit = 1, D/C# = 0 (command)
  Wire.write(0xA1); //Set Segment re-map 0xA0/0xA1
  Wire.write(0b10000000); //control byte, Co bit = 1, D/C# = 0 (command)
  Wire.write(0xC8); //Set COM Output Scan Direction 0xC0,/0xC8
  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
  Wire.write(0xDA); //Set COM Pins hardware configuration 0xDA, 0x02
  Wire.write(0b00010010);
  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
  Wire.write(0x81); //Set Contrast Control 0x81, default=0x7F
  Wire.write(255); //0-255
  Wire.write(0b10000000); //control byte, Co bit = 1, D/C# = 0 (command)
  Wire.write(0xA4); //Disable Entire Display On
  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
  Wire.write(0xA6); //Set Normal Display 0xA6, Inverse display 0xA7
  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
  Wire.write(0xD5); //Set Display Clock Divide Ratio/Oscillator Frequency 0xD5, 0x80
  Wire.write(0b10000000);
  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
  Wire.write(0x20); //Set Memory Addressing Mode
  Wire.write(0x10); //Page addressing mode
  Wire.endTransmission();
  Wire.beginTransmission(ADDRESS_OLED);
  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
  Wire.write(0x22); //Set Page Address
  Wire.write(0); //Start page set
  Wire.write(7); //End page set
  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
  Wire.write(0x21); //set Column Address
  Wire.write(0); //Column Start Address
  Wire.write(127); //Column Stop Address
  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
  Wire.write(0x8D); //Set Enable charge pump regulator 0x8D, 0x14
  Wire.write(0x14);
  Wire.write(0b10000000); //control byte, Co bit = 1, D/C# = 0 (command)
  Wire.write(0xAF); //Display On 0xAF
  Wire.endTransmission();

  delay(10);
}
uint8_t display[64]={
  0b00000000, 0b00000000,0b00000000, 0b00000000,
  0b00000000, 0b00000000,0b00000000, 0b00000000,
  0b00000000, 0b00000000,0b00000000, 0b00000000,
  0b00000000, 0b00000000,0b00000000, 0b00000000,
  0b00000000, 0b00000000,0b00000000, 0b00000000,
  0b00000000, 0b00000000,0b00000000, 0b00000000,
  0b00000000, 0b00000000,0b00000000, 0b00000000,
  0b00000000, 0b00000000,0b00000000, 0b00000000,
  0b00000000, 0b00000000,0b00000000, 0b00000000,
  0b00000000, 0b00000000,0b00000000, 0b00000000,
  0b00000000, 0b00000000,0b00000000, 0b00000000,
  0b00000000, 0b00000000,0b00000000, 0b00000000,
  0b00000000, 0b00000000,0b00000000, 0b00000000,
  0b00000000, 0b00000000,0b00000000, 0b00000000,
  0b00000000, 0b00000000,0b00000000, 0b00000000,
  0b00000000, 0b00000000,0b00000000, 0b00000000,

/*
  0b00000111, 0b10000000,0b00000000, 0b00000000,
  0b00001111, 0b11000000,0b00000000, 0b00000000,
  0b00011111, 0b11100000,0b00000000, 0b00000000,
  0b00111111, 0b11110000,0b00000000, 0b00000000,
  0b01111100, 0b11111000,0b00000000, 0b00000000,
  0b11111000, 0b01111100,0b00000000, 0b00000000,
  0b11110000, 0b00111100,0b00000000, 0b00000000,
  0b11110000, 0b00111100,0b00000000, 0b00000000,
  0b11111111, 0b11111100,0b00000000, 0b00000000,
  0b11111111, 0b11111100,0b11100000, 0b11100000,
  0b11111111, 0b11111100,0b11100000, 0b11101010,
  0b11111111, 0b11111100,0b10000000, 0b10001010,
  0b11110000, 0b00111100,0b11101010, 0b11101010,
  0b11110000, 0b00111100,0b00101010, 0b00101111,
  0b11110000, 0b00111100,0b11101010, 0b11100010,
  0b11110000, 0b00111100,0b11101110, 0b11100010,
  */
};
uint8_t dataA[8]={
  0b11111100,
  0b11111110,
  0b00110011,
  0b00110011,
  0b00110011,
  0b11111110,
  0b11111100,
  0b00000000,
};
uint8_t dataB[8]={
  0b11111111,
  0b11111111,
  0b11011011,
  0b11011011,
  0b11011011,
  0b11111111,
  0b01101110,
  0b00000000,
};
uint8_t dataC[8]={
  0b00111100,
  0b01111110,
  0b11100111,
  0b11000011,
  0b11000011,
  0b11100111,
  0b01100110,
  0b00000000
};
uint8_t dataD[8] = {
  0b11111111,
  0b11111111,
  0b11000011,
  0b11000011,
  0b11000011,
  0b01111110,
  0b00111100,
  0b00000000
};
uint8_t dataE[8]={
  0b11111111,
  0b11111111,
  0b11011011,
  0b11011011,
  0b11011011,
  0b11011011,
  0b00000000,
  0b00000000,
};
uint8_t dataF[8]={
  0b11111111,
  0b11111111,
  0b00011011,
  0b00011011,
  0b00011011,
  0b00011011,
  0b00000000,
  0b00000000,
};
uint8_t dataG[8]={
  0b00111100,
  0b01111110,
  0b11100111,
  0b11000011,
  0b11010011,
  0b11110111,
  0b11110110,
  0b00000000
};

uint8_t dataSus4[16]={
  0b10111011, 0b10101110,
  0b10101010, 0b00101010,
  0b11101011, 0b10111010,
  0b00000000, 0b00000000,
  0b00111000, 0b00000000,
  0b00100000, 0b00000000,
  0b11111000, 0b00000000,
  0b00000000, 0b00000000,

/*
  0b11100000, 0b11100000,
  0b11100000, 0b11101010,
  0b10000000, 0b10001010,
  0b11101010, 0b11101010,
  0b00101010, 0b00101111,
  0b11101010, 0b11100010,
  0b11101110, 0b11100010,
  */
};
uint8_t dataMinor[16]={
  0b11100000, 0b00000000,
  0b11110000, 0b00000000,
  0b00010000, 0b00000000,
  0b11110000, 0b00000000,
  0b11110000, 0b00000000,
  0b00010000, 0b00000000,
  0b11110000, 0b00000000,
  0b11100000, 0b00000000,

/*
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b01111110, 0b00000000,
  0b11011011, 0b00000000,
  0b11011011, 0b00000000,
  0b11011011, 0b00000000,
  */
};
uint8_t dataMajor[16]={
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
};

uint8_t logo[64]={
/*
  0b00000111, 0b10000000,0b00000000, 0b00000000,
  0b00001111, 0b11000000,0b00000000, 0b00000000,
  0b00011111, 0b11100000,0b00000000, 0b00000000,
  0b00111111, 0b11110000,0b00000000, 0b00000000,
  0b01111100, 0b11111000,0b00000000, 0b00000000,
  0b11111000, 0b01111100,0b00000000, 0b00000000,
  0b11110000, 0b00111100,0b00000000, 0b00000000,
  0b11110000, 0b00111100,0b00000000, 0b00000000,

  0b11111111, 0b11111100,0b00000000, 0b00000000,
  0b11111111, 0b11111100,0b11100000, 0b11100000,
  0b11111111, 0b11111100,0b11100000, 0b11101010,
  0b11111111, 0b11111100,0b10000000, 0b10001010,
  0b11110000, 0b00111100,0b11101010, 0b11101010,
  0b11110000, 0b00111100,0b00101010, 0b00101111,
  0b11110000, 0b00111100,0b11101010, 0b11100010,
  0b11110000, 0b00111100,0b11101110, 0b11100010,
*/

0b00000000,0b00000000,0b00000000,0b00000000,
0b00000000,0b00000000,0b00000000,0b00000000,
0b00000000,0b00000000,0b00000000,0b00110000,
0b00001110,0b01001010,0b10100000,0b00101000,
0b00001010,0b10101010,0b10100000,0b00101000,
0b00001100,0b11101100,0b10100000,0b01100000,
0b00001010,0b10101010,0b11100000,0b01100000,
0b00000000,0b00000000,0b00000000,0b00000000,

0b00000000,0b11101001,0b01110111,0b01100000,
0b00000000,0b10001001,0b01010101,0b01010000,
0b00000000,0b10001111,0b01010110,0b01010000,
0b00000000,0b11101001,0b01110101,0b01100000,
0b00000000,0b00000000,0b00000000,0b00000000,
0b00000000,0b00000000,0b00000000,0b00000000,
0b00000000,0b00000000,0b00000000,0b00000000,
0b00000000,0b00000000,0b00000000,0b00000000,

/*
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b01111000,
  0b00000000, 0b00101000,
  0b00000000, 0b01011000,
  0b00000000, 0b00000000,

  0b00001111, 0b01110000,
  0b00001001, 0b00101000,
  0b00001001, 0b01110000,
  0b00000000, 0b00000000,
  0b00001111, 0b01111000,
  0b00000100, 0b00100000,
  0b00000100, 0b01011000,
  0b00001111, 0b00000000,

  0b00000000, 0b01111000,
  0b00001111, 0b01000000,
  0b00001001, 0b01111000,
  0b00001111, 0b00000000,
  0b00000000, 0b00000000,
  0b00001111, 0b00000000,
  0b00000101, 0b00000000,
  0b00001011, 0b00000000,

  0b00000000, 0b00000000,
  0b00001111, 0b01100000,
  0b00001001, 0b01111100,
  0b00000110, 0b00000100,
  0b00000000, 0b00011000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  */

};


byte defaultAdd[] = {
 M_MAJOR, // C
 M_MINOR, // Dm
 M_MINOR, // Em
 M_MAJOR, // F
 M_MAJOR, // G
 M_MINOR, // Am
 M_MINOR, // Bm
};

static unsigned char lookup[16] = {
0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, };

uint8_t reverse(uint8_t n) {
 return (lookup[n&0b1111] << 4) | lookup[n>>4];
}

void lcdClear(){
  Wire.beginTransmission(ADDRESS_OLED);
  Wire.write(0b10000000); //control byte, Co bit = 1, D/C# = 0 (command)
  Wire.write(0xB0); //set page start address
  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
  Wire.write(0x21); //set Column Address
  Wire.write(0); //Column Start Address
  Wire.write(127); //Column Stop Address
  Wire.endTransmission();

  for(int k = 0; k < 8; k ++){
    Wire.beginTransmission(ADDRESS_OLED);
    Wire.write(0b10000000); //control byte, Co bit = 1, D/C# = 0 (command)
    Wire.write(0xB0 + k); //set page start address(B0～B7)
    Wire.endTransmission();
    for(int j = 0; j < 8*2; j ++){
      Wire.beginTransmission(ADDRESS_OLED);
      Wire.write(0b01000000);
      for(int i=0; i<8; i++){
        Wire.write(0x00);
      }
      Wire.endTransmission();
    }
  }
}

void lcdDraw(byte scale,byte add){
  uint8_t* scaleBmp;
  uint8_t* addBmp;
  switch(scale){
    case 0:  scaleBmp = dataC;break;
    case 1:  scaleBmp = dataD;break;
    case 2:  scaleBmp = dataE;break;
    case 3:  scaleBmp = dataF;break;
    case 4:  scaleBmp = dataG;break;
    case 5:  scaleBmp = dataA;break;
    case 6:  scaleBmp = dataB;break;
  }
  if(add == M_NONE){
    add = defaultAdd[scale];
  }
  switch(add){
    case M_MINOR: addBmp = dataMinor;break;
    case M_MAJOR: addBmp = dataMajor;break;
    case M_SUS4:  addBmp = dataSus4;break;
  }

  for(int i = 0; i < 8; i ++){
    display[32 +1+ i * 4] = reverse(scaleBmp[7-i]);
  }
  for(int i = 0; i < 16; i ++){
    display[i/2*4 + i%2] = reverse(addBmp[15-i]);
  }

  Wire.beginTransmission(ADDRESS_OLED);
  Wire.write(0b10000000); //control byte, Co bit = 1, D/C# = 0 (command)
  Wire.write(0xB0); //set page start address(B0～B7)
  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
  Wire.write(0x21); //set Column Address
  Wire.write(64); //Column Start Address(0～127)
        Wire.write(127); //Column Stop Address
  Wire.endTransmission();

  for(int j = 0; j < 32; j ++){
     if(j%4 == 0){
       Wire.beginTransmission(ADDRESS_OLED);
       Wire.write(0b10000000); //control byte, Co bit = 1, D/C# = 0 (command)
       Wire.write(0xB0 + j/4); //set page start address(B0～B7)
       Wire.endTransmission();
     }
     if(j%4 == 2 || j%4 ==3)continue;
      for(int i=0; i<8; i++){
        if(i%2 == 0){
          Wire.beginTransmission(ADDRESS_OLED);
          Wire.write(0b01000000);
        }
        for(int k=0; k<4; k++){
          Wire.write(((display[j/4*8 + j%4] & (1<<(7-i%8)))?0x0f:0x00) | ((display[j/4*8 + j%4 + 4] & (1<<(7-i%8)))?0xf0:0x00));
        }
        if(i%2 == 1){
          Wire.endTransmission();
        }
      }
  }
}

inline uint8_t doubleHead(uint8_t x){
  return
    ((0b00010000 & x)>>4) |
    ((0b00010000 & x)>>3) |
    ((0b00100000 & x)>>3) |
    ((0b00100000 & x)>>2) |
    ((0b01000000 & x)>>2) |
    ((0b01000000 & x)>>1) |
    ((0b10000000 & x)>>1) |
    ((0b10000000 & x));
}
inline uint8_t doubleTail(uint8_t x){
  return
    ((0b00001000 & x)<<4) |
    ((0b00001000 & x)<<3) |
    ((0b00000100 & x)<<3) |
    ((0b00000100 & x)<<2) |
    ((0b00000010 & x)<<2) |
    ((0b00000010 & x)<<1) |
    ((0b00000001 & x)<<1) |
    ((0b00000001 & x));
}

void logoDraw(){
  Wire.beginTransmission(ADDRESS_OLED);
  Wire.write(0b10000000); //control byte, Co bit = 1, D/C# = 0 (command)
  Wire.write(0xB0); //set page start address(B0～B7)
  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
  Wire.write(0x21); //set Column Address
  Wire.write(0); //Column Start Address(0～127)
  Wire.write(63); //Column Stop Address
  Wire.endTransmission();

  for(int j = 0; j < 16; j ++){
    Wire.beginTransmission(ADDRESS_OLED);
    Wire.write(0b01000000);
    Wire.write(doubleTail(logo[j*4 + 3]));
    Wire.write(doubleTail(logo[j*4 + 3]));
    Wire.write(doubleTail(logo[j*4 + 3]));
    Wire.write(doubleTail(logo[j*4 + 3]));
    Wire.endTransmission();
  }
  for(int j = 0; j < 16; j ++){
    Wire.beginTransmission(ADDRESS_OLED);
    Wire.write(0b01000000);
    Wire.write(doubleHead(logo[j*4 + 3]));
    Wire.write(doubleHead(logo[j*4 + 3]));
    Wire.write(doubleHead(logo[j*4 + 3]));
    Wire.write(doubleHead(logo[j*4 + 3]));
    Wire.endTransmission();
  }

  for(int j = 0; j < 16; j ++){
    Wire.beginTransmission(ADDRESS_OLED);
    Wire.write(0b01000000);
    Wire.write(doubleTail(logo[j*4 + 2]));
    Wire.write(doubleTail(logo[j*4 + 2]));
    Wire.write(doubleTail(logo[j*4 + 2]));
    Wire.write(doubleTail(logo[j*4 + 2]));
    Wire.endTransmission();
  }
  for(int j = 0; j < 16; j ++){
    Wire.beginTransmission(ADDRESS_OLED);
    Wire.write(0b01000000);
    Wire.write(doubleHead(logo[j*4 + 2]));
    Wire.write(doubleHead(logo[j*4 + 2]));
    Wire.write(doubleHead(logo[j*4 + 2]));
    Wire.write(doubleHead(logo[j*4 + 2]));
    Wire.endTransmission();
  }

  for(int j = 0; j < 16; j ++){
    Wire.beginTransmission(ADDRESS_OLED);
    Wire.write(0b01000000);
    Wire.write(doubleTail(logo[j*4 + 1]));
    Wire.write(doubleTail(logo[j*4 + 1]));
    Wire.write(doubleTail(logo[j*4 + 1]));
    Wire.write(doubleTail(logo[j*4 + 1]));
    Wire.endTransmission();
  }
  for(int j = 0; j < 16; j ++){
    Wire.beginTransmission(ADDRESS_OLED);
    Wire.write(0b01000000);
    Wire.write(doubleHead(logo[j*4 + 1]));
    Wire.write(doubleHead(logo[j*4 + 1]));
    Wire.write(doubleHead(logo[j*4 + 1]));
    Wire.write(doubleHead(logo[j*4 + 1]));
    Wire.endTransmission();
  }
  for(int j = 0; j < 16; j ++){
    Wire.beginTransmission(ADDRESS_OLED);
    Wire.write(0b01000000);
    Wire.write(doubleTail(logo[j*4 + 0]));
    Wire.write(doubleTail(logo[j*4 + 0]));
    Wire.write(doubleTail(logo[j*4 + 0]));
    Wire.write(doubleTail(logo[j*4 + 0]));
    Wire.endTransmission();
  }

  for(int j = 0; j < 16; j ++){
    Wire.beginTransmission(ADDRESS_OLED);
    Wire.write(0b01000000);
    Wire.write(doubleHead(logo[j*4 + 0]));
    Wire.write(doubleHead(logo[j*4 + 0]));
    Wire.write(doubleHead(logo[j*4 + 0]));
    Wire.write(doubleHead(logo[j*4 + 0]));
    Wire.endTransmission();
  }

}

void setup(){
  soundSetup();
  lcdSetup();
  lcdClear();
  //logoDraw();
  drawDisplay();
  
  mkWave(0);
  
  pinMode(2,OUTPUT); // sync pulse
  digitalWrite(2, LOW);

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
    digitalWrite(2, HIGH); // sync pulse
  }
  if(rythmCount == 100){
    digitalWrite(2, LOW); // sync pulse
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

  boolean pushed = false;

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
      if(!b1){triggerOn(14);pushed = true;}else{triggerOff(14);}
      if(!b2 && pushed == false){triggerOn(15);pushed = true;}else{triggerOff(15);}
      if(!b3 && pushed == false){triggerOn(16);pushed = true;}else{triggerOff(16);}
      if(!b4 && pushed == false){triggerOn(17);pushed = true;}else{triggerOff(17);}
      if(!b5 && pushed == false){triggerOn(18);pushed = true;}else{triggerOff(18);}
      if(!b6 && pushed == false){triggerOn(19);pushed = true;}else{triggerOff(19);}
      if(!b7 && pushed == false){triggerOn(20);pushed = true;}else{triggerOff(20);}
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
