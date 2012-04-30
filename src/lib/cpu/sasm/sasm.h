#ifndef __src_lib_cpu_sasm_h
#define __src_lib_cpu_sasm_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/stat.h>
#include <stdint.h>
#include <sys/types.h>

#include "src/lib/cpu/cpu.h"

typedef struct opcode_st opcode_t;

typedef struct variable_st {
    char *name;
    uint64_t value;
    uint64_t addr;
    size_t refs_sz;
    size_t refs_len;
    uint64_t *refs;
    uint8_t *data;
    size_t len;
} variable_t;

typedef struct sasm_st {
    int prog_sz;
    int prog_len;
    uint8_t *prog;
    uint8_t *ip;

    variable_t *variables;
    size_t variables_len;
    size_t variables_sz;

    int lineno;
    FILE *src_fp, *sys_fp;
} sasm_t;

sasm_t * init_sasm();
void free_sasm(sasm_t *);
void assemble(sasm_t *, char *);
void push0(sasm_t *, op_t, size_t);
void push1(sasm_t *, op_t, void *, size_t);
variable_t * define_variable(sasm_t *, char *, uint64_t, size_t);
variable_t * new_variable(sasm_t *);
variable_t * find_variable(sasm_t *, char *);
variable_t * find_or_create_variable(sasm_t *, char *);
void add_variable_ref(sasm_t *, variable_t *, uint64_t);
variable_t * define_constant(sasm_t *, char *, uint64_t);
variable_t * define_data(sasm_t *, char *, char *);
variable_t * define_label(sasm_t *, char *);
void write_prologue(sasm_t *);
void write_data(sasm_t *);
uint8_t opflags(size_t);

void print_prog(sasm_t *);

int yyparse();
int yylex();

#endif

