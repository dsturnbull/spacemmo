#ifndef __src_lib_cpu_cpu_h
#define __src_lib_cpu_cpu_h

typedef struct stack_cpu_st stack_cpu_t;
typedef struct input_st input_t;
typedef struct tty_st tty_t;
typedef struct keyboard_st keyboard_t;
typedef struct comm_port_st comm_port_t;

typedef enum cpu_e {
    CPU_STACK_CPU,
} cpu_e;

typedef struct cpu_st {
    stack_cpu_t *cpu;
    input_t *input;
    keyboard_t *kbd;
    tty_t *tty;
    comm_port_t *comm_ports;
} cpu_t;

cpu_t * init_cpu(cpu_e);
void cpu_load(cpu_t *, const char *fn);
void cpu_start(cpu_t *);
void cpu_reset(cpu_t *);
void cpu_stop(cpu_t *);
void cpu_step(cpu_t *);
void cpu_status(cpu_t *);

#endif

