#include <stdio.h>
#include <string.h>
#include "decode.h"

/* Validate decode arguments */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if (argv[2] == NULL || strstr(argv[2], ".bmp") == NULL)  // Check if stego image is provided and is .bmp
    {
        printf("ERROR: Please provide a valid .bmp stego image\n"); // Error message for invalid input
        return e_failure; // Return failure status
    }

    decInfo->stego_image_fname = argv[2]; // Store stego image file name
    decInfo->output_fname = "decoded";    // Set default output file base name

    return e_success; // Return success
}

/* Open required decode files */
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r"); // Open stego image in read binary mode
    if (decInfo->fptr_stego_image == NULL)                                // Check if file open failed
    {
        return e_failure; // Return failure
    }

    return e_success; // Return success
}

//Decode one byte from LSB 
Status decode_byte_from_lsb(char *data, char *image_buffer)
{
    char ch = 0;

    for (int i = 0; i < 8; i++)          // Loop through 8 bits
    {
        char get = image_buffer[i] & 1; // Get LSB from image byte
        ch = ch | (get << (7 - i));          // Place bit at correct position (MSB first)
    }

    *data = ch;                          // Store decoded character
    return e_success;
}


/* Decode data from image */
Status decode_data_from_image(char *data, int size, FILE *fptr_image)
{
    char buffer[8]; // Buffer to read 8 bytes from image

    for (int i = 0; i < size; i++) // Loop for required number of bytes
    {
        fread(buffer, 8, 1, fptr_image);           // Read 8 image bytes
        decode_byte_from_lsb(&data[i], buffer);    // Decode one byte from LSBs
    }
    return e_success; // Return success
}

/* Decode magic string */
Status decode_magic_string(DecodeInfo *decInfo)
{
    char magic[strlen(MAGIC_STRING) + 1]; // Buffer to store decoded magic string
    magic[strlen(MAGIC_STRING)] = '\0';   // Null terminate the string

    decode_data_from_image(magic, strlen(MAGIC_STRING), decInfo->fptr_stego_image); // Decode magic string

    if (strcmp(magic, MAGIC_STRING) != 0) // Compare decoded and original magic string
    {
        printf("ERROR: Magic string mismatch. Not a stego image\n"); // Error if mismatch
        return e_failure; // Return failure
    }

    return e_success; // Return success
}

/* Decode secret file extension */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    decode_data_from_image(decInfo->extn_secret_file, strlen(".txt"), decInfo->fptr_stego_image); // Decode file extension
    decInfo->extn_secret_file[strlen(".txt")] = '\0'; // Null terminate extension string

    return e_success; // Return success
}

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    decode_data_from_image((char *)&decInfo->size_secret_file, sizeof(long), decInfo->fptr_stego_image); // Decode secret file size
    return e_success; // Return success
}

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char ch; // Variable to store decoded byte
    char output_file[50]; // Buffer for output file name

    sprintf(output_file, "%s%s", decInfo->output_fname, decInfo->extn_secret_file); // Create output file name

    decInfo->fptr_output = fopen(output_file, "w"); // Open output file in write binary mode
    if (decInfo->fptr_output == NULL)                 // Check if file open failed
    {
        return e_failure; // Return failure
    }

    for (long i = 0; i < decInfo->size_secret_file; i++) // Loop for secret file size
    {
        decode_data_from_image(&ch, 1, decInfo->fptr_stego_image); // Decode one byte
        fwrite(&ch, 1, 1, decInfo->fptr_output);                   // Write byte to output file
    }

    fclose(decInfo->fptr_output); // Close output file
    return e_success; // Return success
}

/* Main decoding function */
Status do_decoding(DecodeInfo *decInfo)
{
    if (open_decode_files(decInfo) == e_failure) // Open stego image file
        return e_failure; // Return failure if file open fails

    fseek(decInfo->fptr_stego_image, 54, SEEK_SET); // Skip BMP header

    if (decode_magic_string(decInfo) == e_failure) // Decode and verify magic string
        return e_failure; // Return failure if mismatch

    decode_secret_file_extn(decInfo); // Decode secret file extension
    decode_secret_file_size(decInfo); // Decode secret file size
    decode_secret_file_data(decInfo); // Decode secret file data

    return e_success; // Return success
}