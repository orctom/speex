#include "speex.h"

#ifndef SPEEX_SPEEX_CODEC_H
#define SPEEX_SPEEX_CODEC_H

#ifdef __cplusplus
extern "C" {
#endif


struct SpeexStateType;
typedef struct SpeexStateType SpeexState;


SpeexState *create_encoder(int mode, int quality, int sampling_rate);
SpeexState *create_decoder(int mode);

int encode(SpeexState *state, short *in, int size, char *out);
int decode(SpeexState *state, char *in, int size, short *out);

void destroy_encoder(SpeexState *state);
void destroy_decoder(SpeexState *state);

#ifdef __cplusplus
}
#endif

#endif
