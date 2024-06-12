#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    //validate .bmp file -- ./a.out -e beatiful.bmp secret.txt
    if(argv[2] != NULL && strcmp(strstr(argv[2],"."), ".bmp") == 0)
    {
        encInfo -> src_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }
    if(argv[3] != NULL && strcmp(strstr(argv[3],"."), ".txt") == 0)
    {
        encInfo -> secret_fname = argv[3];
    }
    else
    {
        return e_failure;
    }
    if(argv[4] != NULL)
    {
        encInfo -> stego_image_fname = argv[4];
    }
    else
    {
        encInfo -> stego_image_fname = "stego.bmp";
    }
    return e_success;

}

uint get_file_size(FILE *fptr_secret)
{
    fseek(fptr_secret,0,SEEK_END);
    return ftell(fptr_secret);
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo -> image_capacity = get_image_size_for_bmp(encInfo -> fptr_src_image);
    encInfo -> size_secret_file = get_file_size(encInfo -> fptr_secret);
    if(encInfo -> image_capacity > (54 + 16 + 32 + 32 + 32 + (encInfo -> size_secret_file * 8)))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

Status copy_bmp_header(FILE *fptr_src_image,FILE *fptr_stego_image)
{
    char header[54];
    fseek(fptr_src_image, 0, SEEK_SET);
    fread(header, sizeof(char), 54, fptr_src_image);
    fwrite(header, sizeof(char), 54, fptr_stego_image);
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    //mask to fetch the bit from data
    unsigned char mask = 1 << 7;
    //loop until the no of bits of the data
    for(int i = 0; i < 8; i++)
    {
        //fetch a bit from the msb of the data or it with the image_buffer
        image_buffer[i] = ((image_buffer[i] & 0xFE) | (data & mask) >> (7 - i));
        mask = mask >> 1; 
    }
    return e_success;
}

Status encode_data_to_image(const char *data,int size,EncodeInfo *encInfo)
{
    for(int i = 0; i < size; i++)
    {
        //read 8 bytes of rgb
        fread(encInfo->image_data, 8, sizeof(char), encInfo->fptr_src_image);
        //call encode_byte_to_lsb function
        encode_byte_to_lsb(data[i], encInfo->image_data);
        fwrite(encInfo->image_data, 8, sizeof(char), encInfo->fptr_stego_image);
    }
    return e_success;
}

Status(encode_magic_string(const char *magic_string,EncodeInfo *encInfo))
{
    //every encoding data should call encode_data_to_image function
    encode_data_to_image(magic_string,strlen(magic_string),encInfo);
    return e_success;
    
}

Status encode_size_to_lsb(char *image_buffer, int size)
{
    unsigned int mask = 1 << 31;
    for(int i = 0; i < 32; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((size & mask) >> (31 - i));
        mask =  mask >> 1;
    }
    return e_success;
}

Status encode_size(int size, FILE *fptr_src, FILE *fptr_stego)
{
    char str[32];
    fread(str, 32, sizeof(char), fptr_src);
    encode_size_to_lsb(str, size);
    fwrite(str, 32, sizeof(char), fptr_stego);
    return e_success;
}

Status encode_secret_file_extn(const char *extn, EncodeInfo *encInfo)
{
    extn = ".txt";
    encode_data_to_image(extn, strlen(extn), encInfo);
    return e_success;
}

Status encode_secret_file_size(long int file_size, EncodeInfo *encInfo)
{
     char str[32];
    fread(str, 32, sizeof(char), encInfo->fptr_src_image);
    encode_size_to_lsb(str, file_size);
    fwrite(str, 32, sizeof(char), encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;
    fseek(encInfo->fptr_secret, 0 ,SEEK_SET);
    for(int i = 0; i < encInfo->size_secret_file; i++)
    {
        fread(&ch, sizeof(char), 1, encInfo->fptr_secret);
        fread(encInfo->image_data, 8, sizeof(char), encInfo->fptr_src_image);
        encode_byte_to_lsb(ch, encInfo->image_data);
        fwrite(encInfo->image_data, 8, sizeof(char), encInfo->fptr_stego_image);
    }
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_stego)
{
    char ch;
    while(fread(&ch, 1, sizeof(char), fptr_src) > 0)
    {
        fwrite(&ch, 1, sizeof(char), fptr_stego);
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    //call the rest of the function
    if(open_files(encInfo) == e_success)
    {
        printf("Successfully opened all the required files\n");
        if(check_capacity(encInfo) == e_success)
        {
            printf("Check capacity:  Possible to encode the secret data\n");
            if(copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image) == e_success)
            {
                printf("Copied header successfully\n");
                if(encode_magic_string(MAGIC_STRING,encInfo) == e_success)
                {
                    printf("Magic string encoded successfully\n");
                    if(encode_size(strlen(".txt"), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                    {
                        printf("Encoded secret file extension size successfully\n");
                        if(encode_secret_file_extn(encInfo->extn_secret_file ,encInfo) == e_success)
                        {
                            printf("Encoded secret file extension successfully\n");
                            if(encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
                            {
                                printf("Successfully encoded secret file size %ld\n",encInfo->size_secret_file);
                                if(encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf("Successfully encoded the secret data\n");
                                    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
                                    {
                                        printf("Copied remaining bytes successfully\n");
                                    }
                                    else
                                    {
                                        printf("Failed to copy the remaining bytes\n");
                                        return e_failure;
                                    }
                                }
                                else
                                {
                                    printf("Failed to encode the secret data\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("Failed to encode the secret file size\n");
                                return e_failure;
                            }
                        }
                        else
                        {
                           printf("Error : Encoded secret file extension\n"); 
                           return e_failure;
                        }
   
                    }
                    else
                    {
                        printf("Error : Encoded secret file extension size\n");  
                        return e_failure;
                    }
                }
                else
                {
                    printf("Failed to encode the magic string\n");
                    return e_failure;
                }
            }
            else
            {
                printf("Failed to copy the header\n");
                return e_failure;
            }
        }
        else
        {
            printf("Check capacity:  Not Possible to encode the secret data\n"); 
            return e_failure; 
        }
    }
    else
    {
        printf("Failed to open the required files\n");
        return e_failure;
    }
    return e_success;
}
