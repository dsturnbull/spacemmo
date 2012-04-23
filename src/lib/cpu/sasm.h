#ifndef __src_lib_cpu_sasm_h
#define __src_lib_cpu_sasm_h

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#include "src/lib/cpu/cpu.h"

#define MAX_LABELS      1024
#define MAX_MISSINGS    1024

struct label {
    char *name;
    uint32_t addr;
    uint32_t *data;
    size_t data_len;
};

typedef struct sasm_st {
    struct label labels[MAX_LABELS];
    struct label missing[MAX_MISSINGS];

    size_t prog_sz;
    size_t prog_len;
    uint8_t *prog;
    uint8_t *ip;
    uint8_t *data;

    size_t lineno;
} sasm_t;

sasm_t * init_sasm();
size_t assemble(sasm_t *, const char *);
void parse_file(sasm_t *, const char *);
uint32_t read_value(sasm_t *, char *, uint8_t *);
void push(sasm_t *, char **, uint8_t, size_t);
void define_constant(sasm_t *, char *, uint32_t);
void define_data(sasm_t *, char *, char *);
void define_variable(sasm_t *, char *, size_t);
void make_label(sasm_t *, char *);
op_t parse_op(char *);
void normalise_line(char **);
void write_data_labels(sasm_t *);
void replace_sentinels(sasm_t *);
void print_prog(sasm_t *);

#endif

