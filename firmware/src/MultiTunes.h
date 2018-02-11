#ifndef MultiTunes_h
#define MultiTunes_h

extern volatile unsigned int d[5]; // default value( for debug)
extern volatile unsigned int dn[5];// work variable
extern volatile unsigned char vol[5]; // volume par channel
extern volatile byte nf;
extern volatile byte nf2;
extern int noise;
extern int noise2;

extern volatile byte vf;

extern byte wave[16][64];


void soundSetup();

#endif
