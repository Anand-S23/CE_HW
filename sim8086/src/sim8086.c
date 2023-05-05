#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define arr_size(arr) (sizeof((arr)) / sizeof(((arr)[0])))

typedef struct binary_file_t
{
    size_t size;
    u8 *data;
} binary_file_t;

binary_file_t read_binary_file(const char *filename)
{
    binary_file_t file = {0};

    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Invalid input file\n");
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    file.size = ftell(fp);
    rewind(fp);

    file.data = (u8 *)malloc(file.size);
    fread(file.data, 1, file.size, fp);

    fclose(fp);
    return file;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Useage: %s <input_binary_instruction>\n", argv[0]);
        exit(1);
    }

    binary_file_t bf = read_binary_file(argv[1]);

    for (int i = 0; i < bf.size; ++i)
    {
        int opcode = bf.data[i] >> 2;
        int dw = bf.data[i] & 3;
        printf("%d\n", bf.data[i] & 3);
    }

    free(bf.data);
    return 0;
}

