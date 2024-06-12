#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo {
    /* Stego file information */
    char *stego_fname;
    FILE *fptr_stego;
    char image_data[MAX_IMAGE_BUF_SIZE];
    int secret_file_size;

    /* Output file information */
    char *output_fname;
    FILE *fptr_output;
} DecodeInfo;

/* Check operation type */
OperationType check_operation_type(char *argv[]);

/* Read and validate Encode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

Status do_decoding(DecodeInfo *decInfo);

Status open_decode_files(DecodeInfo *decInfo);

Status check_magic_string_in_stego(DecodeInfo *decInfo);

Status encode_image_to_data(const char *data, int size, DecodeInfo *decInfo);

Status encode_lsb_bytes(char *image_buffer, int size, const char *magic_string);

Status decode_secret_file_size(DecodeInfo *decInfo);

Status decode_lsb_to_data(char *image_buffer, int size);

Status decode_secret_file_data(DecodeInfo *decInfo);

#endif
