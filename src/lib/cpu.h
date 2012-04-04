#ifndef __src_lib_cpu_h
#define __src_lib_cpu_h

#define CPU_MEMORY_SZ   0x010000
#define CPU_IO_PORT_A   0x00FF00

#define HLT 0xf0
#define LDX 0xf1
#define LDI 0xf2
#define LDP 0xf3
#define STX 0xf4
#define OUT 0xf5
#define JMP 0xf6
#define JZ  0xf7
#define JNZ 0xf8
#define SUB 0xf9
#define ADD 0xfa

#include <stdint.h>
#include <sys/types.h>

#include "src/lib/spacemmo.h"

struct cpu_st {
    uint32_t ax;
    uint32_t *ip;
    uint32_t mem[CPU_MEMORY_SZ];
};

struct label {
    char *name;
    uint32_t addr;
};

struct data {
    char *name;
    uint32_t addr;
};

struct {
    size_t labels_size;
    size_t label_count;
    struct label **labels;

    size_t missings_size;
    size_t missing_count;
    struct label **missing;

    size_t prog_len;
    uint32_t *prog;

    size_t data_size;
    size_t data_count;
    struct data **data;
} asmr;

cpu_t * init_cpu();
size_t cpu_asm(const char *);
uint32_t read_value(char *, uint32_t *);
void push(uint32_t **, char **, uint32_t, size_t);
void load_prog(cpu_t *, uint32_t *, size_t);
void run_prog(cpu_t *);

#endif

