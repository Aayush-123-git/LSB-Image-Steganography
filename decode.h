#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"
#include "common.h"

/* Structure to store decoding information */
typedef struct _DecodeInfo
{
    /* Stego image info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Output secret file info */
    char *output_fname;
    FILE *fptr_output;

    char extn_secret_file[4];
    long size_secret_file;

} DecodeInfo;

/* Function prototypes */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);
Status do_decoding(DecodeInfo *decInfo);
Status open_decode_files(DecodeInfo *decInfo);
Status decode_magic_string(DecodeInfo *decInfo);
Status decode_secret_file_extn(DecodeInfo *decInfo);
Status decode_secret_file_size(DecodeInfo *decInfo);
Status decode_secret_file_data(DecodeInfo *decInfo);
Status decode_data_from_image(char *data, int size, FILE *fptr_image);
Status decode_byte_from_lsb(char *data, char *image_buffer);

#endif