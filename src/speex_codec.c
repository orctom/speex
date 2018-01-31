#include "speex_codec.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//#define DEBUG

const SpeexMode *getSpeexMode(int mode)
{
    switch (mode)
    {
        case 0:
            return &speex_nb_mode;
        case 1:
            return &speex_wb_mode;
        case 2:
            return &speex_uwb_mode;
        default:
            return &speex_wb_mode;
    }
}

SpeexState *create_encoder(int mode, int quality, int sampling_rate)
{
    const SpeexMode *speexMode = getSpeexMode(mode);

    SpeexState *state = malloc(sizeof(SpeexState));
    speex_bits_init(&state->bits);
    state->state = speex_encoder_init(speexMode);

    speex_encoder_ctl(state->state, SPEEX_SET_QUALITY, &quality);
    speex_encoder_ctl(state->state, SPEEX_SET_SAMPLING_RATE, &sampling_rate);
    speex_encoder_ctl(state->state, SPEEX_GET_FRAME_SIZE, &state->frame_size);

    return state;
}

SpeexState *create_decoder(int mode)
{
    const SpeexMode *speexMode = getSpeexMode(mode);

    SpeexState *state = malloc(sizeof(SpeexState));
    speex_bits_init(&state->bits);
    state->state = speex_decoder_init(speexMode);

    int enhance = 1;
    speex_decoder_ctl(state->state, SPEEX_SET_ENH, &enhance);

    speex_decoder_ctl(state->state, SPEEX_GET_FRAME_SIZE, &state->frame_size);

    return state;
}

int encode(SpeexState *state, short *in, int size, char *out)
{
    int frame_size = state->frame_size;
    int n_frame = ((size - 1) / frame_size) + 1;
    int total_bytes = 0;
    for (int i = 0; i < n_frame; i++)
    {
        speex_bits_reset(&state->bits);
#ifdef DEBUG
        printf("%d     frame size: %d, size: %d, n frame: %d\n", i, frame_size, size, n_frame);
#endif
        int start = i * frame_size;
        int end = min(start + frame_size, size);
        int len = end - start;
        short buffer[len];

        memcpy(buffer, in + len * i, sizeof(short) * len);
#ifdef DEBUG
        printf("%d i   frame size: %d, size: %d, n frame: %d, start: %d, end: %d, len: %d\n", i, frame_size, size, n_frame, start, end, len);
#endif
        speex_encode_int(state->state, buffer, &state->bits);
        int n_bytes = speex_bits_nbytes(&state->bits);
        total_bytes += n_bytes + 1;
        char output_buffer[n_bytes];
        speex_bits_write(&state->bits, output_buffer, frame_size);
#ifdef DEBUG
        printf("%d ii  frame size: %d, size: %d, n frame: %d, n_bytes: %d vs %d\n", i, frame_size, size, n_frame, n_bytes);
#endif

        int bytesIndex = i * n_bytes + i;
        out[bytesIndex] = n_bytes;

        int offset = bytesIndex + 1;
        memcpy(out + offset, output_buffer, n_bytes);
#ifdef DEBUG
        printf("%d iii frame size: %d, size: %d, n frame: %d, bytesIndex: %d, offset: %d\n", i, frame_size, size, n_frame, bytesIndex, offset);
#endif
    }
#ifdef DEBUG
    printf("total_bytes: %d\n", total_bytes);
#endif
    return total_bytes;
}

int decode(SpeexState *state, char *in, int size, short *out)
{
    fflush(stdout);
    int frame_size = state->frame_size;
    short decoded[frame_size];
    int total_bytes = 0;

    int offset = 0;
    int n_frame = size / 71;
    for (int i = 0; i < n_frame; i++)
    {
#ifdef DEBUG
        printf("%d     frame size: %d, size: %d, n frame: %d\n", i, frame_size, size, n_frame);
#endif
        int len = in[offset++];
        speex_bits_read_from(&state->bits, in + offset, len);
        offset += len;
#ifdef DEBUG
        printf("%d i   frame size: %d, size: %d, n frame: %d, len: %d\n", i, frame_size, size, n_frame, len);
#endif
        speex_decode_int(state->state, &state->bits, decoded);
        memcpy(out + frame_size * i, decoded, sizeof(short) * frame_size);
        total_bytes += frame_size;
    }
#ifdef DEBUG
    printf("total_bytes: %d\n", total_bytes);
#endif
    return total_bytes;
}

void destroy(SpeexState *state)
{
    speex_bits_destroy(&state->bits);
    speex_decoder_destroy(state->state);
    free(state);
}
