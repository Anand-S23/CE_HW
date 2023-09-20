#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define arr_size(arr) (sizeof((arr)) / sizeof(((arr)[0])))

typedef struct binary_file_t
{
    size_t size;
    u8 *data;
} binary_file_t;

#define REG_ENCODING_MAX 17
#define WIDE_START 8

const char *reg_encoding_values[REG_ENCODING_MAX] = {
    "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh", 
    "ax", "cx", "dx", "bx", "sp", "bp", "si", "di"
};

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

void print_bits(u8 n) 
{
    for (int i = 7; i >= 0; i--) putchar('0' + ((n >> i) & 1));
}

void decode_immediate_to_reg(u8 *data)
{
    // int opcode = data[0] >> 4;
    int w = (data[0] >> 3) & 1;
    int reg = data[0] & 0b111;

    assert(reg < REG_ENCODING_MAX);

    int mov_val = w ? ((u16)data[0] << 8) | data[1] : data[0];
    const char *dst_reg = reg_encoding_values[reg + WIDE_START * w];

    printf("mov %s, %d\n", dst_reg, mov_val);
}

void decode_rm_to_reg(u8 *data)
{
    // int opcode = data[0] >> 2;
    int dw = data[0] & 0b11;
    int d = dw >> 1;
    int w = dw & 1;

    // int mod = data[1] >> 6;
    int reg = (data[1] >> 3) & 0b111;
    int rm = data[1] & 0b111;

    assert(reg < REG_ENCODING_MAX && rm < REG_ENCODING_MAX);

    int wide_mod = (w == 1) ? WIDE_START : 0;
    const char *dst_reg, *src_reg;
    if (d) 
    {
        dst_reg = reg_encoding_values[reg + wide_mod];
        src_reg = reg_encoding_values[rm + wide_mod];
    }
    else
    {
        dst_reg = reg_encoding_values[rm + wide_mod];
        src_reg = reg_encoding_values[reg + wide_mod];
    }

    printf("mov %s, %s\n", dst_reg, src_reg);
}

void decode_instructions(binary_file_t *bf)
{
    printf("bits 16\n\n");

    for (int i = 0; i < bf->size; i += 2)
    {
        int opcode = bf->data[i] >> 4;
        if (opcode == 0b1011)
        {
            decode_immediate_to_reg(bf->data + i);
            continue;
        }

        opcode = bf->data[i] >> 2;
        if (opcode == 0b100010)
        {
            int w = (bf->data[i] >> 3) & 1;
            decode_rm_to_reg(bf->data + i);
            if (w) i += 1;
            continue;
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Useage: %s <input_binary_instruction>\n", argv[0]);
        exit(1);
    }

    binary_file_t bf = read_binary_file(argv[1]);
    decode_instructions(&bf);
    free(bf.data);

    return 0;
}

