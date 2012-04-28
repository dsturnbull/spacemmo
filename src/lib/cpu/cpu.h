#ifndef __src_lib_cpu_cpu_h
#define __src_lib_cpu_cpu_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/event.h>

#define CPU_MEMORY_SZ       0x1000000   // 16MB
#define CPU_STACK           0x0ffa000   // 8KB stack
#define CPU_RET_STACK       0x0ffc000   // 1024 ret stack depth
#define CPU_IDT             0x0ffe000   // 1024 interrupts

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
    disk_t *disk;
    port_t *port0, *port1, *port2, *port3;
} cpu_t;

typedef enum irq_e {
    IRQ_DBG     =   0x0ffe000,

    IRQ_CLK     =   0x0ffe010,
    IRQ_TTY     =   0x0ffe014,
    IRQ_KBD     =   0x0ffe018,
    IRQ_DISK_SET=   0x0ffe01c,

    IRQ_DISK_RD =   0x0ffe020,
    IRQ_DISK_WR =   0x0ffe024,
    IRQ_P0_IN   =   0x0ffe028,
    IRQ_P0_OUT  =   0x0ffe02c,

    IRQ_P1_IN   =   0x0ffe030,
    IRQ_P1_OUT  =   0x0ffe034,
    IRQ_P2_IN   =   0x0ffe038,
    IRQ_P2_OUT  =   0x0ffe03c,

    IRQ_P3_IN   =   0x0ffe040,
    IRQ_P3_OUT  =   0x0ffe044,
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
    RET     =   0x02,
    CALL    =   0x03,

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
    JZ      =   0x0e,
    JNZ     =   0x0f,
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

void handle_op      (cpu_t *, opcode_t *);
void handle_nop     (cpu_t *, instruction_t *);
void handle_hlt     (cpu_t *, instruction_t *);
void handle_ret     (cpu_t *, instruction_t *);
void handle_call    (cpu_t *, instruction_t *);
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
void handle_jz      (cpu_t *, instruction_t *);
void handle_jnz     (cpu_t *, instruction_t *);
void handle_dup     (cpu_t *, instruction_t *);
void handle_push    (cpu_t *, instruction_t *);
void handle_pop     (cpu_t *, instruction_t *);
void handle_swap    (cpu_t *, instruction_t *);
void handle_int     (cpu_t *, instruction_t *);

void handle_irq(cpu_t *, uint8_t *);
void set_timer_isr(cpu_t *, uint32_t, uint32_t);
void handle_timer(cpu_t *);
void set_kbd_isr(cpu_t *, uint32_t);
void handle_kbd(cpu_t *);
void set_port_isr(cpu_t *, int, uint32_t);
void handle_port_read(cpu_t *, port_t *);

#endif

