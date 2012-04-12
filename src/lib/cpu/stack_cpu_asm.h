#ifndef __src_stack_cpu_asm_h
#define __src_stack_cpu_asm_h

#include <stdint.h>
#include <sys/types.h>

#include "src/lib/cpu/stack_cpu.h"

typedef struct stack_cpu_asm_st {
    size_t labels_size;
    size_t label_count;
    struct label **labels;

    size_t missings_size;
    size_t missing_count;
    struct label **missing;

    size_t prog_len;
    uint8_t *prog;
    uint8_t *ip;

    size_t data_size;
    size_t data_count;
    struct data **data;
} stack_cpu_asm_t;

struct label {
    char *name;
    uint8_t addr;
};

struct data {
    char *name;
    uint8_t addr;
};

stack_cpu_asm_t * init_stack_cpu_asm();
size_t stack_cpu_asm(stack_cpu_asm_t *, const char *);
void parse_file(stack_cpu_asm_t *, const char *);
uint64_t read_value(stack_cpu_asm_t *, char *, uint8_t *);
void define_constant(stack_cpu_asm_t *, char *, uint32_t);
void make_label(stack_cpu_asm_t *, char *);
op_t parse_op(char *);
void normalise_line(char **);
void replace_sentinels(stack_cpu_asm_t *);
void print_prog(stack_cpu_asm_t *);

#endif

