#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int argc, char *argv[]) {
    if (check_operation_type(argv) == e_encode) {
        EncodeInfo encInfo;
        printf("Selected Encoding\n");
        if (read_and_validate_encode_args(argv, &encInfo) == e_success) {
            printf("Read and validate argument is successful\n");
            if (do_encoding(&encInfo) == e_success) {
                printf("Completed Encoding\n");
            } else {
                printf("Failed to encode the message\n");
            }
        } else {
            printf("Failed: Read and validate argument\n");
        }
    }

    if (check_operation_type(argv) == e_decode) {
        DecodeInfo decInfo;
        printf("Selected Decoding\n");
        if (read_and_validate_decode_args(argv, &decInfo) == d_success) {
            printf("Read and validate argument is successful\n");
            if (do_decoding(&decInfo) == d_success) {
                printf("Completed Decoding\n");
            } else {
                printf("Failed to decode the message\n");
            }
        } else {
            printf("Failed: Read and validate argument\n");
        }
    } else {
        printf("Invalid Option\n");
        printf("--------------------------Usage----------------------------\n");
        printf("Encoding: ./a.out -e beautiful.bmp secret.txt stego.bmp\n");
        printf("Decoding: ./a.out -d stego.bmp output.txt\n");
    }
    return 0;
}




OperationType check_operation_type(char *argv[])
{
    if(strcmp(argv[1], "-e") == 0)
    {
        return e_encode;
    }
    else if(strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}
