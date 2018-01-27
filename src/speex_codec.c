#include "speex_codec.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
        memcpy(buffer, in, sizeof(short) * len);
#ifdef DEBUG
        printf("%d i   frame size: %d, size: %d, n frame: %d, start: %d, end: %d, len: %d\n", i, frame_size, size, n_frame, start, end, len);
#endif
        speex_encode_int(state->state, buffer, &state->bits);
        int n_bytes = speex_bits_nbytes(&state->bits);
        total_bytes += n_bytes;
#ifdef DEBUG
        printf("%d ii  frame size: %d, size: %d, n frame: %d, n_bytes: %d\n", i, frame_size, size, n_frame, n_bytes);
#endif
        char output_buffer[n_bytes];
        speex_bits_write(&state->bits, output_buffer, n_bytes);

        for (int j = 0; j < n_bytes; j++)
        {
            out[i * n_bytes + j] = output_buffer[j];
        }
#ifdef DEBUG
        printf("%d iii frame size: %d, size: %d, n frame: %d, n_byte: %d\n", i, frame_size, size, n_frame, n_bytes);
#endif
    }
    return total_bytes;
}

int decode(SpeexState *state, char *in, int size, short *out)
{
    fflush(stdout);
    int frame_size = state->frame_size;
    //int n_frame = ((size - 1) / frame_size) + 1;
    short decoded[320 * 20];
    int total_bytes = 0;
    char buffer[3200];
    /*
    for (int i = 0; i < n_frame; i++)
    {
        speex_bits_reset(&state->bits);
#ifdef DEBUG
        printf("%d     frame size: %d, size: %d, n frame: %d\n", i, frame_size, size, n_frame);
#endif
        int start = i * frame_size;
        int end = min(start + frame_size, size);
        int len = end - start;
        memcpy(buffer, in, sizeof(char) * len);
#ifdef DEBUG
        printf("%d i   frame size: %d, size: %d, n frame: %d, start: %d, end: %d, len: %d\n", i, frame_size, size, n_frame, start, end, len);
#endif
        speex_bits_read_from(state->state, buffer, len);
        int n_bytes = speex_decode_int(state, &state->bits, decoded);
        total_bytes += n_bytes;
#ifdef DEBUG
        printf("%d ii  frame size: %d, size: %d, n frame: %d, n_bytes: %d\n", i, frame_size, size, n_frame, n_bytes);
#endif

        memcpy(out + sizeof(short) * len * i, decoded, sizeof(short) * len);
#ifdef DEBUG
        printf("%d iii frame size: %d, size: %d, n frame: %d, n_byte: %d\n", i, frame_size, size, n_frame, n_bytes);
#endif
    }
    */
    speex_bits_reset(&state->bits);
    for (int i = 0; i < size; i += frame_size) {
        int start_pos = i;
        int end_pos = i + frame_size;
        if (end_pos > size) {
            break;
        }
        printf("frame_size: %d, start: %d, end: %d\n", frame_size, start_pos, end_pos);
        fflush(stdout);
        memcpy(buffer, in + start_pos, frame_size);
        fflush(stdout);
        speex_bits_read_from(&state->bits, buffer, frame_size);
        int n_bytes = speex_decode_int(state->state, &state->bits, decoded);
        printf("frame_size: %d, start: %d, end: %d, n_bytes: %d\n", frame_size, start_pos, end_pos, n_bytes);
        if (n_bytes <= 0) {
            printf("speex decode error: %d\n", n_bytes);
            return 0;
        }
        memcpy(out + total_bytes, decoded, n_bytes);
        total_bytes += n_bytes;
        printf("n_bytes: %d, total_bytes: %d\n", n_bytes, total_bytes);
        fflush(stdout);
    }
    return total_bytes;
}

void destroy(SpeexState *state)
{
    speex_bits_destroy(&state->bits);
    speex_decoder_destroy(state->state);
    free(state);
}
