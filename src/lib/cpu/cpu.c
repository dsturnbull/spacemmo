#include <stdio.h>
#include <stdlib.h>

#include "src/lib/cpu/cpu.h"
#include "src/lib/ui/input.h"
#include "src/lib/cpu/stack_cpu.h"
#include "src/lib/cpu/stack_cpu_asm.h"

cpu_t *
init_cpu(cpu_e type)
{
    cpu_t *this = malloc(sizeof(cpu_t));

    this->input = init_input(NULL);

    switch (type) {
        case CPU_STACK_CPU:
            this->cpu = init_stack_cpu();
            this->kbd = this->cpu->kbd;
            this->tty = this->cpu->tty;
            this->comm_ports = this->cpu->comm_ports;
            this->halted = &this->cpu->halted;
            break;
    }

    /* this->cpu->debug = true; */

    return this;
}

// TODO genericise

void
cpu_load(cpu_t *cpu, const char *fn)
{
    stack_cpu_asm_t *as = init_stack_cpu_asm();
    stack_cpu_asm(as, fn);
    printf("%lu bytes loaded\n", as->prog_len);
    load_prog(cpu->cpu, as->prog, as->prog_len);
    print_prog(as);
}

void
cpu_start(cpu_t *cpu)
{
    cpu->cpu->halted = false;
}

void
cpu_reset(cpu_t *cpu)
{
}

void
cpu_stop(cpu_t *cpu)
{
}

void
cpu_step(cpu_t *cpu)
{
    step(cpu->cpu);
    //print_state(cpu->cpu);
}

void
cpu_status(cpu_t *cpu)
{
    print_state(cpu->cpu);
}

