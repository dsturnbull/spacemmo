#ifndef __src_lib_cpu_h
#define __src_lib_cpu_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/event.h>

#define CPU_MEMORY_SZ       0x1000  // 4KB
#define CPU_STACK           0x0f40  // 64 stack depth
#define CPU_RET_STACK       0x0f60  // 64 stack depth
#define CPU_IDT             0x0f80  // 64 interrupts

#define LOG(...) do {                                                       \
    if (cpu->debug)                                                         \
        fprintf(cpu->log, __VA_ARGS__);                                     \
    } while (0)

typedef struct tty_st tty_t;
typedef struct disk_st disk_t;

typedef enum irq_e {
    IRQ_CLK     =   0x0f80,
    IRQ_TTY     =   0x0f84,
    IRQ_KBD     =   0x0f88,
    IRQ_DISK_SET=   0x0f8c,
    IRQ_DISK_RD =   0x0f90,
    IRQ_DISK_WR =   0x0f94,
} irq_t;

typedef enum op_e {
    NOP     =   0x00,
    HLT     =   0x01,
    LOAD    =   0x02,
    STORE   =   0x03,
    ADD     =   0x04,
    SUB     =   0x05,
    MUL     =   0x06,
    DIV     =   0x07,
    AND     =   0x08,
    OR      =   0x09,
    JMP     =   0x0a,
    JE      =   0x0b,
    JZ      =   0x0c,
    JNZ     =   0x0d,
    CALL    =   0x0e,
    RET     =   0x0f,
    DUP     =   0x10,
    PUSH    =   0x11,
    POP     =   0x12,
    SWAP    =   0x13,
    INT     =   0x14,
} op_t;

typedef struct opcode_st {
    int op:6;   // 0x40 instructions
    int mode:2; // reg, imm8, imm16, imm32
} opcode_t;

typedef struct cpu_st {
    uint8_t mem[CPU_MEMORY_SZ];
    uint8_t *ip, *sp, *bp;

    int kq;
    struct kevent ke;

    FILE *log;
    bool debug;
    long cycles;
    bool halted;

    tty_t *tty;
    disk_t *disk;
} cpu_t;

cpu_t * init_cpu();
void load_cpu(cpu_t *, uint8_t *, size_t);
void run_cpu(cpu_t *);
void step_cpu(cpu_t *);
void reset_cpu(cpu_t *);
void print_region(cpu_t *, uint8_t *, uint8_t *, size_t, char *, int);

void handle_irq(cpu_t *, uint8_t *);
void set_timer_isr(cpu_t *, uint32_t, uint32_t);
void handle_timer(cpu_t *);
void set_kbd_isr(cpu_t *, uint32_t);
void handle_kbd(cpu_t *);

#endif

