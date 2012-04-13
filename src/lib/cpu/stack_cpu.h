#ifndef __src_stack_cpu_h
#define __src_stack_cpu_h

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#define CPU_MEMORY_SZ 0x10000
#define CPU_CODE      0x00000
#define CPU_DATA      0x04000
#define CPU_STACK     0x06000
#define CPU_FRAMES    0x08000
#define CPU_IO        0x09000

typedef enum irq_e {
    IRQ_KBD,
    IRQ_TTY,
} irq_t;

#define GET_CONST(c) {                                                      \
    if (strcasecmp(op, #c) == 0) {                                          \
        return c;                                                           \
    }                                                                       \
} while (0)

#define MEM_FMT "%08x"
#define MAX_COMM_PORTS 8

typedef struct tty_st tty_t;
typedef struct keyboard_st keyboard_t;
typedef struct comm_port_st comm_port_t;

typedef struct stack_cpu_st {
    uint32_t mem[CPU_MEMORY_SZ];
    uint32_t *code, *data,  *stack, *frames;
    uint32_t *ip,           *sp,    *rp;

    size_t cycles;
    bool debug;
    bool halted;
    FILE *log;

    keyboard_t *kbd;
    tty_t *tty;
    comm_port_t *comm_ports;
} stack_cpu_t;

typedef enum op_e {
    // 0 args
    HLT,    // 0x0000
    LOAD,   // 0x0001
    STORE,  // 0x0002
    ADD,    // 0x0003
    SUB,    // 0x0004
    MUL,    // 0x0005
    DIV,    // 0x0006
    AND,    // 0x0007
    JMP,    // 0x0008
    JZ ,    // 0x0009
    JNZ,    // 0x000a
    CALL,   // 0x000b
    RET,    // 0x000c
    DUP,    // 0x000d
    POP,    // 0x000e
    SWAP,   // 0x000f
    INT,    // 0x0010
    DEBUG,  // 0x0011

    // 1 arg
    PUSH,   // 0x0012
} op_t;

stack_cpu_t * init_stack_cpu();

void load_prog(stack_cpu_t *, uint32_t *, size_t);
void run_prog(stack_cpu_t *);
void step(stack_cpu_t *);
void handle_interrupt(stack_cpu_t *);
void print_state(stack_cpu_t *);
void reset(stack_cpu_t *);

#endif

