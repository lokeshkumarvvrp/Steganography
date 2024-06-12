#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

Status open_decode_files(DecodeInfo *decInfo)
{
    // output file
    decInfo->fptr_output = fopen(decInfo->output_fname, "w");
    // Do Error handling
    if (decInfo->fptr_output == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->output_fname);

    	return d_failure;
    }

    // Stego Image file
    decInfo->fptr_stego = fopen(decInfo->stego_fname, "r");
    // Do Error handling
    if (decInfo->fptr_stego == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_fname);

    	return d_failure;
    }

    // No failure return e_success
    return d_success;
}

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    //validate .bmp file -- ./a.out -d stego.bmp output.txt
    if(argv[2] != NULL && strcmp(strstr(argv[2],"."), ".bmp") == 0)
    {
        decInfo -> stego_fname = argv[2];
    }
    else
    {
        return d_failure;
    }
    if(argv[3] != NULL)
    {
        decInfo -> output_fname = argv[3];
    }
    else
    {
        decInfo -> output_fname = "output.txt";
    }
    return d_success;
}

Status encode_lsb_bytes(char *image_buffer,int size,const char *magic_string)
{
    for (int i = 0; i < size; i++)
    {
        // Extract the last bit from each byte
        unsigned char lsb = (image_buffer[i] >> 7) & 0x01;

        // Check if the LSB matches the corresponding bit in the magic string
        if (lsb != ((magic_string[i / 8] >> 7) & 0x01))
        {
            // LSB does not match, return failure
            return d_failure;
        }
    }
    // All LSBs matched, return success
    return d_success;
}

Status encode_image_to_data(const char *data, int size, DecodeInfo *decInfo)
{
    // Read 2 bytes directly since the magic string is only 2 characters long
    fread(decInfo->image_data, sizeof(char),2, decInfo->fptr_stego);

    // Call encode_lsb_bytes function
    return encode_lsb_bytes(decInfo->image_data, 16, MAGIC_STRING);
}



Status check_magic_string_in_stego(DecodeInfo *decInfo)
{
    fseek(decInfo->fptr_stego, 54, SEEK_SET);
    return encode_image_to_data(MAGIC_STRING, 2, decInfo);
}

Status decode_lsb_to_data(char *image_buffer, int size)
{
    char character = 0;
    for(int i = 0; i < size; i++)
    {
        character = (character << 1) | (image_buffer[i] & 0x01);
    }
    return d_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    fseek(decInfo->fptr_stego, 54 + 16 + 32 + 32, SEEK_SET); 
    char secret[32]; 
    fread(secret, sizeof(char), 32, decInfo->fptr_stego); 
    int length = 0;
    for (int i = 0; i < 32; i++) {
        length = (length << 1) | (secret[i] & 0x01);
    }
    decInfo -> secret_file_size = length;
    return d_success;
}


Status decode_secret_file_data(DecodeInfo *decInfo)
{
    int length = 54 + 16 + 32 + 32 + 32;
    char secret_data[8];
    fseek(decInfo->fptr_stego, length, SEEK_SET);
    
    // String to store the decoded data
    char decoded_data[MAX_SECRET_BUF_SIZE * decInfo->secret_file_size + 1]; // +1 for null terminator
    int decoded_index = 0;

    // Loop through each byte of secret data
    for(int i = 0; i < decInfo->secret_file_size; i++)
    {
        // Read a block of LSBs from the stego image
        fread(secret_data, sizeof(char), 8, decInfo->fptr_stego);
        
        // Reconstruct character from the LSBs
        char character = 0;
        for (int j = 0; j < 8; j++) {
            character |= (secret_data[j] & 0x01) << (7 - j); // Reconstruct character from LSBs
        }

        // Append the character to the decoded_data string
        decoded_data[decoded_index++] = character;
    }

    // Add null terminator to the decoded_data string
    decoded_data[decoded_index] = '\0';

    // Append the decoded_data string to the output file
    fprintf(decInfo->fptr_output, "%s", decoded_data);

    return d_success;
}




Status do_decoding(DecodeInfo *decInfo)
{
    if(open_decode_files(decInfo) == d_success)
    {
         printf("Successfully opened all the required files\n");  
         if(check_magic_string_in_stego(decInfo))
         {
             printf("Magic string encoded and checked successfully\n");
             if(decode_secret_file_size(decInfo) == d_success)
             {
                printf("Secret file size decoded successfully\n");
                if(decode_secret_file_data(decInfo) == d_success)
                {
                    printf("Secret file data decoded successfully\n");
                }
                else
                {
                    printf("Error : secret file data decode failed");
                    return d_failure;
                }
             }
             else
             {
                printf("Error : Secret file size decode failed");
                return d_failure;
             }

         }
         else
         {
               printf("The magic string is not available in this .bmp file\n");
               return d_failure;
         }
    }
    else
    {
        printf("Failed to open the required files\n");
        return d_failure;
    }
    return d_success;
}
