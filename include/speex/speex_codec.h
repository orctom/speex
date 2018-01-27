#include "speex.h"

#ifndef SPEEX_SPEEX_CODEC_H
#define SPEEX_SPEEX_CODEC_H

#define DEBUG

#define max(a,b) ( ((a)>(b)) ? (a):(b) )
#define min(a,b) ( ((a)>(b)) ? (b):(a) )


typedef struct {
    SpeexBits bits;
    void *state;
    int frame_size;
} SpeexState;


SpeexState *create_encoder(int mode, int quality, int sampling_rate);
SpeexState *create_decoder(int mode);

int encode(SpeexState *state, short *in, int size, char *out);
int decode(SpeexState *state, char *in, int size, short *out);

void destroy(SpeexState *state);

#endif
