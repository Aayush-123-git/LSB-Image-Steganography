#include <stdio.h>              // Standard I/O functions
#include <string.h>             // String handling functions
#include "encode.h"             // Encode-related declarations

// Validate encode arguments
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if (strstr(argv[2], ".bmp") == NULL)              // Check source image extension
    {
        printf("ERROR: Source image must be .bmp\n"); // Error if not bmp
        return e_failure;
    }

    encInfo->src_image_fname = argv[2];               // Store source image filename  //src_image_fname is the charcter pointer
    encInfo->secret_fname = argv[3];                  // Store secret filename

    char *extn = strstr(argv[3], ".txt");                // Find secret file extension
    if (extn == NULL)                                 // If extension not found
    {
        printf("ERROR: Secret file must have extension\n");
        return e_failure;
    }

    strcpy(encInfo->extn_secret_file, extn);           // Copy extension to structure

    if (argv[4] == NULL)                              // If output file not provided
        encInfo->stego_image_fname = "stego.bmp";     // Use default stego filename
    else
        encInfo->stego_image_fname = argv[4];         // Use user-provided stego file

    return e_success;                                 // Arguments validated successfully
}

// Open required files
Status open_files(EncodeInfo *encInfo)
{
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");   // Open source image--beautiful.bmp
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");         // Open secret file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w"); // Open output image--stego.bmp

    if (encInfo->fptr_src_image == NULL || encInfo->fptr_secret == NULL || encInfo->fptr_stego_image == NULL) // File open check
    {
        return e_failure;
    }
    return e_success;                             // Files opened successfully
}

// Get BMP image capacity
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;                           // Variables to store dimensions
    fseek(fptr_image, 18, SEEK_SET);              // Seek to width location that comes after 18 bytes in BMP header
    fread(&width, sizeof(int), 1, fptr_image);    // Read width //4 bytes
    fread(&height, sizeof(int), 1, fptr_image);   // Read height //4 bytes
    return width * height * 3;                    // Return image capacity (RGB)
}//One Pixel = 3 bytes (1-RED 1-GREEN 1-BLUE)

// Get Secret file size
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);                     // Move file pointer to end
    return ftell(fptr);                           // Return file size
}

// Check image capacity
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image); // Get image capacity
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);           // Get secret file size

    long required = (strlen(MAGIC_STRING) +        // Calculate required bits
                    strlen(encInfo->extn_secret_file) +
                    sizeof(int) +
                    encInfo->size_secret_file) * 8;

    if ((encInfo->image_capacity - 54) >= required) // Check if image can hold secret data
        return e_success;
    else
        return e_failure;
}

// Copy BMP header
Status copy_bmp_header(FILE *src, FILE *dest)//src and dest are file pointers to beautiful.bmp and stego.bmp
{
    char header[54];                              // BMP header size
    fseek(src, 0, SEEK_SET);                      // Move to beginning of source image
    fread(header, 54, 1, src);                    // Read header
    fwrite(header, 54, 1, dest);                  // Write header to stego image
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)           // Loop through 8 bits
    {
        image_buffer[i] = image_buffer[i] & (~1);            // Clear LSB
        char get = (data >> (7 - i)) & 1;                    // Extract MSB first
        image_buffer[i] =  image_buffer[i] | get;            // Set LSB with data bit
    }
    return e_success;
} 

// Encode data to image
Status encode_data_to_image(   char* data              , int size        , FILE *src               , FILE *dest)
{
    char buffer[8];                               // Buffer to hold 8 image bytes

    for (int i = 0; i < size; i++)                // Loop through each data byte
    {
        fread(buffer, 8, 1, src);                 // Read 8 bytes from image beautiful.bmp
        encode_byte_to_lsb(data[i], buffer);      // Encode one byte
        fwrite(buffer, 8, 1, dest);               // Write modified bytes to stego.bmp
    }
    return e_success;
}

// Encode magic string
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    return encode_data_to_image((char *)magic_string, strlen(magic_string),encInfo->fptr_src_image,encInfo->fptr_stego_image);
}

// Encode secret file extension
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    return encode_data_to_image(  (char *)file_extn    , strlen(file_extn)   ,encInfo->fptr_src_image,encInfo->fptr_stego_image);
}

// Encode secret file size
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    return encode_data_to_image((char *)&file_size     , sizeof(file_size)    ,encInfo->fptr_src_image,encInfo->fptr_stego_image);
}

// Encode secret file data
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;                                      // Variable to hold secret data
    while (fread(&ch, 1, 1, encInfo->fptr_secret)) // Read secret file byte by byte
    {
        encode_data_to_image(         &ch              , 1                    ,encInfo->fptr_src_image,encInfo->fptr_stego_image);
    }
    return e_success;
}

// Copy remaining image data
Status copy_remaining_img_data(FILE *src, FILE *dest)
{
    char ch;                                      // Variable to copy remaining bytes
    while (fread(&ch, 1, 1, src))                 // Read remaining image data
        fwrite(&ch, 1, 1, dest);                  // Write to stego image
    return e_success;
}

// Main encoding function
/*WE HAVE CALLED ALL THE  FUNCTIONS WITHIN THE do_encoding function*/
Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) == e_failure)         // Open files
        return e_failure;

    if (check_capacity(encInfo) == e_failure)     // Check image capacity
        return e_failure;

    copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image); // Copy BMP header
    encode_magic_string(MAGIC_STRING, encInfo);   // Encode magic string
    encode_secret_file_extn(encInfo->extn_secret_file, encInfo); // Encode extension
    encode_secret_file_size(encInfo->size_secret_file, encInfo); // Encode file size
    encode_secret_file_data(encInfo);              // Encode secret data
    copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image); // Copy remaining data

    return e_success;                              // Encoding completed successfully
}