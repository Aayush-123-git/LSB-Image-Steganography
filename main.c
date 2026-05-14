#include <stdio.h>      // Standard input-output functions
#include <string.h>     // For strcmp()
#include "encode.h"     // Encode-related declarations
#include "decode.h"     // Decode-related declarations
#include "types.h"      // User-defined types

OperationType check_operation_type(char *argv[])  // Check encode or decode option
{
    if (argv[1] == NULL) return e_unsupported;     // No operation provided

    if (strcmp(argv[1], "-e") == 0) return e_encode;   // Encode option
    else if (strcmp(argv[1], "-d") == 0) return e_decode; // Decode option
    else return e_unsupported;                      // Invalid option
}

int main(int argc, char *argv[])
{
    OperationType op_type;                          // Stores operation type
    op_type = check_operation_type(argv);           // Get operation type

    if (op_type == e_encode)                        // Encoding block
    {
        EncodeInfo encInfo;                         // Encode structure

        if (argc < 4)                               // Check argument count
        {
            printf("Usage:\n./a.out -e <src.bmp> <secret.txt> [stego.bmp]\n");
            return 1;
        }

        if (read_and_validate_encode_args(argv, &encInfo) == e_failure) // Validate args
        {
            printf("ERROR: Invalid encode arguments\n");
            return 1;
        }

        if (do_encoding(&encInfo) == e_failure)     // Perform encoding
        {
            printf("ERROR: Encoding failed\n");
            return 1;
        }

        printf("Encoding completed successfully\n"); // Success message
    }
    else if (op_type == e_decode)                   // Decoding block
    {
        DecodeInfo decInfo;                         // Decode structure

        if (argc < 3)                               // Check argument count
        {
            printf("Usage:\n./a.out -d <stego.bmp>\n");
            return 1;
        }

        if (read_and_validate_decode_args(argv, &decInfo) == e_failure) // Validate args
        {
            printf("ERROR: Invalid decode arguments\n");
            return 1;
        }

        if (do_decoding(&decInfo) == e_failure)     // Perform decoding
        {
            printf("ERROR: Decoding failed\n");
            return 1;
        }

        printf("Decoding completed successfully\n"); // Success message
    }
    else                                            // Unsupported operation
    {
        printf("ERROR: Unsupported operation\n");   // Error message
        printf("Use -e for encode or -d for decode\n");
        return 1;
    }

    return 0;                                       // Normal termination
}