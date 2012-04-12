#ifndef __src_stack_cpu_h
#define __src_stack_cpu_h

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

#define MAX_COMM_PORTS 8

typedef struct tty_st tty_t;
typedef struct keyboard_st keyboard_t;
typedef struct comm_port_st comm_port_t;

typedef struct stack_cpu_st {
    uint8_t mem[CPU_MEMORY_SZ];
    uint8_t *code, *data,  *stack, *frames;
    uint8_t *ip,           *sp,    *rp;

    size_t cycles;
    bool debug;
    bool halted;

    keyboard_t *kbd;
    tty_t *tty;
    comm_port_t *comm_ports;
} stack_cpu_t;

typedef struct ins_st {
    uint8_t op:6;
    uint8_t opt:2;
} ins_t;

typedef enum op_e {
    // 0 args
    HLT,    // 0x0000
    LOAD,   // 0x0001
    STORE,  // 0x0002
    ADD,    // 0x0003
    SUB,    // 0x0004
    MUL,    // 0x0005
    DIV,    // 0x0006
    JMP,    // 0x0007
    JZ ,    // 0x0008
    JNZ,    // 0x0009
    CALL,   // 0x000a
    RET,    // 0x000b
    DUP,    // 0x000c
    POP,    // 0x000d
    SWAP,   // 0x000e
    INT,    // 0x000f
    DEBUG,  // 0x0010

    // 1 arg
    PUSH,   // 0x0011
} op_t;

stack_cpu_t * init_stack_cpu();

void load_prog(stack_cpu_t *, uint8_t *, size_t);
void run_prog(stack_cpu_t *);
void step(stack_cpu_t *);
void handle_interrupt(stack_cpu_t *);
void print_state(stack_cpu_t *);
void reset(stack_cpu_t *);

#endif

