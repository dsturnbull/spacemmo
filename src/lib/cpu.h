#ifndef __src_lib_cpu_h
#define __src_lib_cpu_h

#include <stdint.h>
#include <sys/types.h>
#include <sys/time.h>

#include "src/lib/spacemmo.h"

#define CPU_MEMORY_SZ   0x100000
#define CPU_IO_PORT_A   0x0FFFC // 4 bytes
#define CPU_CLOCK       0x0FFF0 // 12 bytes

#define R0              0x020000
#define R1              0x040000
#define R2              0x080000
#define R3              0x100000
#define R4              0x120000
#define R5              0x140000
#define R6              0x180000
#define R7              0x200000

#define get_define(a) a

#define CHECK_OP(c) {                                                       \
    if (strcasecmp(op, #c) == 0) {                                          \
        return c;                                                           \
    }                                                                       \
} while (0)

struct cpu_st {
    uint32_t r0;
    uint32_t r1;
    uint32_t *ip;
    uint32_t mem[CPU_MEMORY_SZ];
    struct timeval time;
};

typedef enum op_e {
    NOP,    // 0x0000
    HLT,    // 0x0001
    LD0,    // 0x0002
    LDI,    // 0x0003
    LDP,    // 0x0004
    ST0,    // 0x0005
    OUT,    // 0x0006
    JMP,    // 0x0007
    JZ ,    // 0x0008
    JNZ,    // 0x0009
    SUB,    // 0x000a
    ADD,    // 0x000b
    DIV,    // 0x000c
    MUL,    // 0x000d

    // meta
    INC,
    DEC,
    DATA,
} op_t;

struct label {
    char *name;
    uint32_t addr;
};

struct data {
    char *name;
    uint32_t addr;
};

struct asmr_st {
    size_t labels_size;
    size_t label_count;
    struct label **labels;

    size_t missings_size;
    size_t missing_count;
    struct label **missing;

    size_t prog_len;
    uint32_t *prog;
    uint32_t *ip;

    size_t data_size;
    size_t data_count;
    struct data **data;
};

cpu_t * init_cpu();
asmr_t * init_asmr();
size_t cpu_asm(asmr_t *, const char *);
void make_label(asmr_t *, char *);
op_t parse_op(char *);
void normalise_line(char **);
void op_hlt(asmr_t *);
uint32_t read_value(asmr_t *, char *, uint32_t *);
void push(asmr_t *, char **, uint32_t, size_t);
void load_prog(cpu_t *, uint32_t *, size_t);
void run_prog(cpu_t *);

#endif

