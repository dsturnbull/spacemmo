#ifndef __src_lib_cpu_cpu_h
#define __src_lib_cpu_cpu_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/event.h>

#define CPU_MEMORY_SZ       0x10000 // 64KB
#define CPU_STACK           0xfe80  // 32 stack depth
#define CPU_RET_STACK       0xff00  // 32 stack depth
#define CPU_IDT             0xff80  // 32 interrupts

#define LOG(...) do {                                                       \
    if (cpu->debug)                                                         \
        fprintf(cpu->log, __VA_ARGS__);                                     \
    } while (0)

typedef struct tty_st tty_t;
typedef struct disk_st disk_t;
typedef struct port_st port_t;

typedef struct cpu_st {
    uint8_t mem[CPU_MEMORY_SZ];
    uint8_t *ip, *sp, *bp;

    int kq;
    struct kevent ke;

    FILE *log;
    bool debug;
    long cycles;
    bool halted;
    char **src;

    tty_t *tty;
    disk_t *disk;
    port_t *port0, *port1, *port2, *port3;
} cpu_t;

typedef enum irq_e {
    IRQ_DBG     =   0xff80,
    IRQ_CLK     =   0xff90,
    IRQ_TTY     =   0xff94,
    IRQ_KBD     =   0xff98,
    IRQ_DISK_SET=   0xff9c,
    IRQ_DISK_RD =   0xffa0,
    IRQ_DISK_WR =   0xffa4,
    IRQ_P0_IN   =   0xffa8,
    IRQ_P0_OUT  =   0xffac,
    IRQ_P1_IN   =   0xffb0,
    IRQ_P1_OUT  =   0xffb4,
    IRQ_P2_IN   =   0xffb8,
    IRQ_P2_OUT  =   0xffbc,
    IRQ_P3_IN   =   0xffc8,
    IRQ_P3_OUT  =   0xffc4,
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

cpu_t * init_cpu();
void free_cpu(cpu_t *);
void load_cpu(cpu_t *, char *);
void run_cpu(cpu_t *);
void step_cpu(cpu_t *);
void reset_cpu(cpu_t *);
void print_region(cpu_t *, uint8_t *, uint8_t *, size_t, int);

void handle_irq(cpu_t *, uint8_t *);
void set_timer_isr(cpu_t *, uint32_t, uint32_t);
void handle_timer(cpu_t *);
void set_kbd_isr(cpu_t *, uint32_t);
void handle_kbd(cpu_t *);
void set_port_isr(cpu_t *, int, uint32_t);
void handle_port_read(cpu_t *, port_t *);

#endif

