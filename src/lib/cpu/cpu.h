#ifndef __src_lib_cpu_cpu_h
#define __src_lib_cpu_cpu_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/event.h>
#include <sys/stat.h>

#define MEMORY_SZ   0x1000000   // 16MB

#define IRQ_P0_BUF  0x00fc000
#define IRQ_P1_BUF  0x00fc100
#define IRQ_P2_BUF  0x00fc200
#define IRQ_P3_BUF  0x00fc300

#define STACK       0x00fd000    // 4KB stack
#define RET_STACK   0x00fe000    // 512 ret stack depth
#define IDT         0x00ff000    // 512 interrupts

#define LOG(...) do {                                                       \
    if (cpu->debug)                                                         \
        fprintf(cpu->log, __VA_ARGS__);                                     \
    } while (0)

typedef struct cpu_st cpu_t;
typedef struct instruction_st instruction_t;
typedef struct tty_st tty_t;
typedef struct mmu_st mmu_t;
typedef struct port_st port_t;

typedef struct cpu_st {
    uint8_t mem[MEMORY_SZ];
    uint8_t *ip, *sp, *rp;

    int kq;
    struct kevent ke;

    FILE *log;
    bool debug;
    long cycles;
    struct timespec *ts;
    bool idling;
    char **src;

    void (**opmap0)(cpu_t *, instruction_t *);
    void (**opmap1)(cpu_t *, instruction_t *, uint64_t);
    void (**opmap2)(cpu_t *, instruction_t *, uint64_t, uint64_t);

    tty_t *tty;
    mmu_t *mmu;
    port_t *port0, *port1, *port2, *port3;
} cpu_t;

typedef enum irq_e {
    IRQ_CLK     =   IDT + 0x000,
    IRQ_TTY     =   IDT + 0x008,
    IRQ_KBD     =   IDT + 0x010,
    IRQ_MMU     =   IDT + 0x018,

    IRQ_P0      =   IDT + 0x020,
    IRQ_P1      =   IDT + 0x028,
    IRQ_P2      =   IDT + 0x030,
    IRQ_P3      =   IDT + 0x038,
} irq_t;

typedef struct opcode_st {
    unsigned int flags:3;
    unsigned int op:5;
} opcode_t;

typedef struct instruction_st {
    unsigned int flags:3;
    unsigned int op:5;
    size_t len;
} instruction_t;

typedef enum op_e {
    NOP     =   0x00,
    HLT     =   0x01,
    CALL    =   0x02,
    RET     =   0x03,
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
void handle_io(cpu_t *);
void idle_cpu(cpu_t *);

void handle_op      (cpu_t *, opcode_t *);
void handle_nop     (cpu_t *, instruction_t *);
void handle_hlt     (cpu_t *, instruction_t *);
void handle_call    (cpu_t *, instruction_t *, uint64_t);
void handle_ret     (cpu_t *, instruction_t *);
void handle_load    (cpu_t *, instruction_t *);
void handle_store   (cpu_t *, instruction_t *, uint64_t);
void handle_add     (cpu_t *, instruction_t *, uint64_t, uint64_t);
void handle_sub     (cpu_t *, instruction_t *, uint64_t, uint64_t);
void handle_mul     (cpu_t *, instruction_t *, uint64_t, uint64_t);
void handle_div     (cpu_t *, instruction_t *, uint64_t, uint64_t);
void handle_and     (cpu_t *, instruction_t *, uint64_t, uint64_t);
void handle_or      (cpu_t *, instruction_t *, uint64_t, uint64_t);
void handle_jmp     (cpu_t *, instruction_t *, uint64_t);
void handle_je      (cpu_t *, instruction_t *, uint64_t);
void handle_jne     (cpu_t *, instruction_t *, uint64_t);
void handle_jz      (cpu_t *, instruction_t *, uint64_t);
void handle_jnz     (cpu_t *, instruction_t *, uint64_t);
void handle_dup     (cpu_t *, instruction_t *, uint64_t);
void handle_push    (cpu_t *, instruction_t *);
void handle_pop     (cpu_t *, instruction_t *, uint64_t);
void handle_swap    (cpu_t *, instruction_t *);
void handle_int     (cpu_t *, instruction_t *);

void handle_irq(cpu_t *, uint8_t *);
void set_timer_isr(cpu_t *, uint64_t, uint64_t);
void handle_timer(cpu_t *);
void set_kbd_isr(cpu_t *, uint64_t);
void handle_kbd(cpu_t *);
port_t * find_port(cpu_t *, irq_t);
void handle_port_read(cpu_t *, port_t *);

#endif

