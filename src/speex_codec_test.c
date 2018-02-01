#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "speex/speex_codec.h"

union SwapUnit {
    char char_data[2];
    short short_data;
};

int read_pcm(char* file_name, short** data, int* data_len) {
    FILE *file_fd = fopen(file_name, "r");
    if (file_fd == NULL) {
        printf("open file: %s error.\n", file_name);
        return -1;
    }
    fseek(file_fd, 0, SEEK_END);
    int flen = ftell(file_fd);
    *data_len = flen / 2;
    *data = (short *) malloc(sizeof(short) * *data_len);
    rewind(file_fd);
    union SwapUnit swap_unit;
    for (int i = 0; i < *data_len; ++i) {
        fread(&swap_unit, sizeof(char), 2, file_fd);
        (*data)[i] = swap_unit.short_data;
    }
    fclose(file_fd);
    printf("read file %s success, file size: %d, data size: %d.\n",
           file_name, flen, *data_len);
    return 0;
}

int write_pcm(char *file_name, short *data, int data_len) {
    FILE *file_fd = fopen(file_name, "w");
    if (file_fd == NULL) {
        printf("open file: %s error.", file_name);
        return -1;
    }
    union SwapUnit swap_unit;
    //fwrite(data, sizeof(short), data_len, file_fd);
    for (int i = 0; i < data_len; ++i) {
        swap_unit.short_data = data[i];
        fwrite(&swap_unit, sizeof(char), 2, file_fd);
    }
    fclose(file_fd);
    printf("write file %s success, data size: %d\n",
            file_name, data_len);
    return 0;
}

int write_speex(char *file_name, char *data, int data_len) {
    FILE *file_fd = fopen(file_name, "w");
    if (file_fd == NULL) {
        printf("open file: %s error.", file_name);
        return -1;
    }
    fwrite(data, sizeof(char), data_len, file_fd);
    fclose(file_fd);
    printf("write file %s success, data size: %d\n",
           file_name, data_len);
    return 0;
}

int read_speex(char *file_name, char **data, int *data_len) {
    FILE *file_fd = fopen(file_name, "r");
    if (file_fd == NULL) {
        printf("open file: %s error.\n", file_name);
        return -1;
    }
    fseek(file_fd, 0, SEEK_END);
    int flen = ftell(file_fd);
//    buffer = new char[len + 1];
    *data_len = flen;
    *data = (char *) malloc(sizeof(char) * *data_len);
    rewind(file_fd);
    fread(*data, sizeof(char), *data_len, file_fd);
    fclose(file_fd);
    printf("read file %s success, file size: %d, data size: %d.\n",
           file_name, flen, *data_len);
    return 0;
}

int check_args(int argc, char **argv, int* action_code, char** infile_name, char** outfile_name) {
    if (argc != 4) {
        printf("Usage: <enc/dec> <in_file> <out_file>\n");
        return -1;
    }
    if (strcmp(argv[1], "enc") == 0) {
        *action_code = 0;  // encode
    } else if (strcmp(argv[1], "dec") == 0) {
        *action_code = 1;
    } else {
        return -1;
    }
    *infile_name = argv[2];
    *outfile_name = argv[3];
    return 0;
}

int do_encode(char* infile_name, char* outfile_name) {
    short* data = NULL;
    int data_len;
    int ret = read_pcm(infile_name, &data, &data_len);
    if (ret != 0) {
        return -1;
    }
    SpeexState* speex_state = create_encoder(1, 8, 16000);
    if (speex_state == NULL) {
        printf("create speex state error.\n");
        free(data);
        return -1;
    }
    char* encoded_data = (char*)malloc(sizeof(char) * data_len);
    int encoded_data_len = 0;
    encoded_data_len = encode(speex_state, data, data_len, encoded_data);

    printf("encoded, size: %d\n", encoded_data_len);
    ret = write_speex(outfile_name, encoded_data, encoded_data_len);
    if (ret != 0) {
        free(encoded_data);
        free(data);
        return -1;
    }
    free(encoded_data);
    free(data);
    printf("encode wav file: %s to speex file: %s success.\n", infile_name, outfile_name);
    return 0;
}

int do_decode(char* infile_name, char* outfile_name) {
    char* data = NULL;
    int data_len;
    int ret = read_speex(infile_name, &data, &data_len);
    if (ret != 0) {
        return -1;
    }
    SpeexState* speex_state = create_decoder(1);
    if (speex_state == NULL) {
        printf("create speex state error.\n");
        free(data);
        return -1;
    }
    short* decoded_data = (short*)malloc(sizeof(short) * data_len * 20);
    int decoded_data_len = 0;
    decoded_data_len = decode(speex_state, data, data_len, decoded_data);

    printf("decoded, size: %d\n", decoded_data_len);
    ret = write_pcm(outfile_name, decoded_data, decoded_data_len);
    if (ret != 0) {
        free(decoded_data);
        free(data);
        return -1;
    }
    free(decoded_data);
    free(data);
    return 0;
}

int main(int argc, char **argv)
{
    int action_code = -1;
    char* infile_name = NULL;
    char* outfile_name = NULL;
    if (check_args(argc, argv, &action_code, &infile_name, &outfile_name) != 0) {
        return -1;
    }
    switch (action_code) {
        case 0:
            return do_encode(infile_name, outfile_name);
        case 1:
            return do_decode(infile_name, outfile_name);
        default:
            printf("only supports 'enc' or 'dec'\n");
            return -1;
    }
}
