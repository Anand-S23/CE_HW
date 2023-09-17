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

void decode_instructions(binary_file_t *bf)
{
    printf("bits 16\n\n");

    for (int i = 0; i < bf->size; i += 2)
    {
        int opcode = bf->data[i] >> 2; // ABCDEFGH >> 2 = 00ABCDEF
        int dw = bf->data[i] & 0b00000011;
        int d = dw >> 1;
        int w = dw & 1;
        assert(opcode == 0b100010 && "Somehow not a mov instruction");

        int mod = bf->data[i + 1] >> 6;
        int reg = (bf->data[i + 1] >> 3) & 0b00000111;
        int rm = bf->data[i + 1] & 0b00000111;
        assert(mod == 0b11 && "Somehow not a register to register mov instruction");

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

#if DEBUG
        printf("Opcode: %d, DW: %d\n", opcode, dw);
        print_bits(bf->data[i + 1]);
        printf(" -> mod: %d, reg: %d, r/m: %d\n", mod, reg, rm);
#endif
        printf("mov %s, %s\n", dst_reg, src_reg);
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

