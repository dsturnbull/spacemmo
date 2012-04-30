#ifndef __src_lib_cpu_cpu_h
#define __src_lib_cpu_cpu_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/event.h>

#define CPU_MEMORY_SZ       0x1000000   // 16MB
#define CPU_STACK           0xffa000    // 8KB stack
#define CPU_RET_STACK       0xffc000    // 1024 ret stack depth
#define CPU_IDT             0xffe000    // 1024 interrupts

#define LOG(...) do {                                                       \
    if (cpu->debug)                                                         \
        fprintf(cpu->log, __VA_ARGS__);                                     \
    } while (0)

typedef struct cpu_st cpu_t;
typedef struct instruction_st instruction_t;
typedef struct tty_st tty_t;
typedef struct disk_st disk_t;
typedef struct port_st port_t;

typedef struct cpu_st {
    uint8_t mem[CPU_MEMORY_SZ];
    uint8_t *ip, *sp, *rp;

    int kq;
    struct kevent ke;

    FILE *log;
    bool debug;
    long cycles;
    bool halted;
    char **src;

    void (**opmap)(cpu_t *, instruction_t *);

    tty_t *tty;
    port_t *port0, *port1, *port2, *port3;
} cpu_t;

typedef enum irq_e {
    IRQ_CLK     =   0xffe000,
    IRQ_TTY     =   0xffe008,
    IRQ_KBD     =   0xffe010,

    IRQ_P0      =   0xffe020,
    IRQ_P1      =   0xffe028,
    IRQ_P2      =   0xffe030,
    IRQ_P3      =   0xffe038,

    IRQ_P0_BUF  =   0xffe100,
    IRQ_P1_BUF  =   0xffe200,
    IRQ_P2_BUF  =   0xffe300,
    IRQ_P3_BUF  =   0xffe400,
} irq_t;

typedef struct opcode_st {
    unsigned int op:5;
    unsigned int flags:3;
} opcode_t;

typedef struct instruction_st {
    unsigned int op:5;
    unsigned int flags:3;
    size_t len;
} instruction_t;

typedef enum op_e {
    // 0-operands
    NOP     =   0x00,
    HLT     =   0x01,
    CALL    =   0x02,
    RET     =   0x03,

    // b/w/d/q operand
    LOAD    =   0x04,
    STORE   =   0x05,
    ADD     =   0x06,
    SUB     =   0x07,
    MUL     =   0x08,
    DIV     =   0x09,
    AND     =   0x0a,
    OR      =   0x0b,
    JMP     =   0x0c,
    JE      =   0x0d,
    JNE     =   0x0e,
    JZ      =   0x0f,
    JNZ     =   0x10,
    DUP     =   0x11,
    PUSH    =   0x12,
    POP     =   0x13,
    SWAP    =   0x14,
    INT     =   0x15,
} op_t;

cpu_t * init_cpu();
void free_cpu(cpu_t *);
void load_cpu(cpu_t *, char *);
void run_cpu(cpu_t *);
void step_cpu(cpu_t *);
void reset_cpu(cpu_t *);
void print_region(cpu_t *, uint8_t *, uint8_t *, size_t, int);

void handle_op      (cpu_t *, opcode_t *);
void handle_nop     (cpu_t *, instruction_t *);
void handle_hlt     (cpu_t *, instruction_t *);
void handle_call    (cpu_t *, instruction_t *);
void handle_ret     (cpu_t *, instruction_t *);
void handle_load    (cpu_t *, instruction_t *);
void handle_store   (cpu_t *, instruction_t *);
void handle_add     (cpu_t *, instruction_t *);
void handle_sub     (cpu_t *, instruction_t *);
void handle_mul     (cpu_t *, instruction_t *);
void handle_div     (cpu_t *, instruction_t *);
void handle_and     (cpu_t *, instruction_t *);
void handle_or      (cpu_t *, instruction_t *);
void handle_jmp     (cpu_t *, instruction_t *);
void handle_je      (cpu_t *, instruction_t *);
void handle_jne     (cpu_t *, instruction_t *);
void handle_jz      (cpu_t *, instruction_t *);
void handle_jnz     (cpu_t *, instruction_t *);
void handle_dup     (cpu_t *, instruction_t *);
void handle_push    (cpu_t *, instruction_t *);
void handle_pop     (cpu_t *, instruction_t *);
void handle_swap    (cpu_t *, instruction_t *);
void handle_int     (cpu_t *, instruction_t *);

void handle_irq(cpu_t *, uint8_t *);
void set_timer_isr(cpu_t *, uint64_t, uint64_t);
void handle_timer(cpu_t *);
void set_kbd_isr(cpu_t *, uint64_t);
void handle_kbd(cpu_t *);
port_t * find_port(cpu_t *, irq_t);
void set_port_isr(cpu_t *, port_t *, uint64_t);
void handle_port_read(cpu_t *, port_t *);

#endif

