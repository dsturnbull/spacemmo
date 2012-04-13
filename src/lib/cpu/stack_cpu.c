#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "src/lib/cpu/stack_cpu_asm.h"
#include "src/lib/cpu/stack_cpu.h"
#include "src/lib/cpu/hardware/keyboard.h"
#include "src/lib/cpu/hardware/tty.h"
#include "src/lib/cpu/hardware/comm_port.h"

stack_cpu_t *
init_stack_cpu()
{
    stack_cpu_t *cpu = calloc(1, sizeof(stack_cpu_t));
    cpu->code = &cpu->mem[CPU_CODE];
    cpu->ip = &cpu->code[0];
    cpu->data = &cpu->mem[CPU_DATA];
    cpu->stack = &cpu->mem[CPU_STACK];
    cpu->sp = &cpu->stack[0];
    cpu->frames = &cpu->mem[CPU_FRAMES];
    cpu->rp = &cpu->frames[0];
    cpu->kbd = init_keyboard();
    cpu->tty = init_tty();
    cpu->comm_ports = malloc(MAX_COMM_PORTS * sizeof(comm_port_t));
    cpu->halted = true;
    cpu->log = fopen("/tmp/cpu.log", "w");
    return cpu;
}

void
load_prog(stack_cpu_t *stack_cpu, uint32_t *prog, size_t len)
{
    for (size_t i = 0; i < len; i++)
        stack_cpu->mem[i] = prog[i];
}

void
run_prog(stack_cpu_t *cpu)
{
    while (*(cpu->ip) != HLT)
        step(cpu);
}

void
step(stack_cpu_t *cpu)
{
    op_t op = *(cpu->ip);

    uint32_t t;

    // then ip is set, make it point to the previous instruction,
    // because this loop will increment it at the end.

    if (cpu->debug) {
        fprintf(cpu->log,
                "ip: " MEM_FMT " "
                "sp: " MEM_FMT " "
                "rp: " MEM_FMT " "
                "op: " MEM_FMT ": "
                "cy: %lu ",
                (uint32_t)(cpu->ip - cpu->code),
                (uint32_t)(cpu->sp - cpu->stack),
                (uint32_t)(cpu->rp - cpu->frames),
                op,
                cpu->cycles);
    }

    cpu->cycles++;
    //gettimeofday(&cpu->time, NULL);
    //cpu->mem[STACK_CPU_CLOCK] = cpu->time.tv_sec;

    switch (op) {
        case HLT:
            if (cpu->debug)
                fprintf(cpu->log, "halt\n");
            cpu->halted = true;
            return;

        case LOAD:
            if (cpu->debug)
                fprintf(cpu->log, "loading " MEM_FMT " from " MEM_FMT "\n",
                        cpu->data[*(cpu->sp)], *(cpu->sp));
            *(cpu->sp) = cpu->data[*(cpu->sp)];
            break;

        case STORE:
            if (cpu->debug)
                fprintf(cpu->log, "storing " MEM_FMT " at " MEM_FMT "\n",
                        *(cpu->sp),
                        *(cpu->sp - 1));
            cpu->data[*(cpu->sp--)] = *(cpu->sp--);
            break;

        case ADD:
            if (cpu->debug)
                fprintf(cpu->log, MEM_FMT " + " MEM_FMT " == " MEM_FMT "\n",
                        *(cpu->sp - 1),
                        *(cpu->sp),
                        *(cpu->sp - 1) + *(cpu->sp));
            *(cpu->sp - 1) = *(cpu->sp - 1) + *(cpu->sp);
            cpu->sp--;
            break;

        case SUB:
            if (cpu->debug)
                fprintf(cpu->log, MEM_FMT " - " MEM_FMT " == " MEM_FMT "\n",
                        *(cpu->sp - 1),
                        *(cpu->sp),
                        *(cpu->sp - 1) - *(cpu->sp));
            *(cpu->sp - 1) = *(cpu->sp - 1) - *(cpu->sp);
            cpu->sp--;
            break;

        case MUL:
            if (cpu->debug)
                fprintf(cpu->log, MEM_FMT " * " MEM_FMT "\n",
                    *(cpu->sp - 1), *(cpu->sp));
            *(cpu->sp) = *(cpu->sp - 1) * *(cpu->sp--);
            break;

        case DIV:
            if (cpu->debug)
                fprintf(cpu->log, MEM_FMT " / " MEM_FMT "\n",
                    *(cpu->sp - 1), *(cpu->sp));
            t = *(cpu->sp);
            *(cpu->sp) = *(cpu->sp - 1) % *(cpu->sp);
            *(cpu->sp - 1) = *(cpu->sp - 1) / t;
            break;

        case AND:
            if (cpu->debug)
                fprintf(cpu->log, MEM_FMT " & " MEM_FMT "\n",
                    *(cpu->sp - 1), *(cpu->sp));
            t = *(cpu->sp--);
            *(cpu->sp) = *(cpu->sp) & t;
            break;

        case JMP:
            if (cpu->debug)
                fprintf(cpu->log, "jump " MEM_FMT "\n", *(cpu->sp));
            cpu->ip = &cpu->code[*(cpu->sp--)] - 1;
            break;

        case JZ:
            if (cpu->debug)
                fprintf(cpu->log, MEM_FMT " == 0 ", *(cpu->sp - 1));
            if (*(cpu->sp - 1) == 0) {
                if (cpu->debug)
                    fprintf(cpu->log, "jumping to " MEM_FMT "\n", *(cpu->sp));
                cpu->ip = &cpu->code[*(cpu->sp)] - 1;
            } else {
                if (cpu->debug)
                    fprintf(cpu->log, "not jumping\n");
            }
            cpu->sp -= 2;
            break;

        case JNZ:
            if (cpu->debug)
                fprintf(cpu->log, MEM_FMT " != 0 ", *(cpu->sp - 1));
            if (*(cpu->sp - 1) != 0) {
                if (cpu->debug)
                    fprintf(cpu->log, "jumping to " MEM_FMT "\n", *(cpu->sp));
                cpu->ip = &cpu->code[*(cpu->sp)] - 1;
            } else {
                if (cpu->debug)
                    fprintf(cpu->log, "not jumping\n");
            }
            cpu->sp -= 2;
            break;

        case CALL:
            if (cpu->debug)
                fprintf(cpu->log, "calling " MEM_FMT " from " MEM_FMT "\n",
                        *(cpu->sp),
                        (uint32_t)(cpu->ip - cpu->code));
            *(cpu->rp++) = cpu->ip - cpu->code;
            cpu->ip = &cpu->code[*(cpu->sp--)] - 1;
            break;

        case RET:
            if (cpu->debug)
                fprintf(cpu->log, "returning from " MEM_FMT " to " MEM_FMT "\n",
                        (uint32_t)(cpu->ip - cpu->code),
                        *(cpu->rp - 1));
            cpu->ip = cpu->code + *(--cpu->rp);
            break;

        case DUP:
            if (cpu->debug)
                fprintf(cpu->log, "dup of " MEM_FMT "\n", *(cpu->sp));
            *(++cpu->sp) = *(cpu->sp);
            break;

        case POP:
            if (cpu->debug)
                fprintf(cpu->log, "pop\n");
            cpu->sp--;
            break;

        case SWAP:
            if (cpu->debug)
                fprintf(cpu->log, "swap\n");
            t = *(cpu->sp);
            *(cpu->sp) = *(cpu->sp - 1);
            *(cpu->sp - 1) = t;
            break;

        case INT:
            handle_interrupt(cpu);
            break;

        case PUSH:
            if (cpu->debug)
                fprintf(cpu->log, "pushing " MEM_FMT "\n", *(cpu->ip + 1));
            *(++cpu->sp) = *(++cpu->ip);
            break;

        case DEBUG:
            fprintf(cpu->log, "debugging on\n");
            cpu->debug = true;
            break;
    }

    if (cpu->debug)
        print_state(cpu);

    cpu->ip++;
}

void
handle_interrupt(stack_cpu_t *cpu)
{
    irq_t rsrc = *(cpu->sp--);
    uint32_t arg1;
    uint32_t arg2;

    switch (rsrc) {
        case IRQ_TTY:
            arg1 = *(cpu->sp--);
            if (cpu->debug)
                fprintf(cpu->log, "tty <- " MEM_FMT "\n", arg1);
            ttyp(cpu->tty, arg1);
            break;

        case IRQ_KBD:
            arg1 = *(cpu->sp--);
            arg2 = *(cpu->sp--);
            if (cpu->debug)
                fprintf(cpu->log, "storing kbd state at " MEM_FMT "\n", arg2);
            keyboard_state(cpu->kbd, (uint8_t *)&cpu->data[arg2]);
            *(cpu->rp++) = cpu->ip - cpu->code;
            cpu->ip = &cpu->code[arg1] - 1;

            break;

        default:
            if (cpu->debug)
                fprintf(cpu->log, "unknown resource " MEM_FMT "\n", rsrc);
    }
}

void
print_state(stack_cpu_t *cpu)
{
    // print final interesting things

    //printf("\033[2J");
    const int w = 16;

    char status[3];
    status[0] = ' ';
    status[1] = ' ';
    status[2] = '\0';

    // code
    /*
    for (uint32_t i = 0; i < 128; i++) {
        if (i % w == 0)
            fprintf(cpu->log, "\ncode   %06x: ", i);

        status[0] = ' ';
        status[1] = ' ';

        if (i == cpu->ip - cpu->code)
            status[1] = '*';

        fprintf(cpu->log, "%s", status);
        fprintf(cpu->log, MEM_FMT, cpu->code[i]);
    }
    */

    // data
    for (int i = 0; i < 0x120; i++) {
        if (i % w == 0)
            fprintf(cpu->log, "\ndata   %06x: ", i);

        status[0] = ' ';
        status[1] = ' ';

        if (i == cpu->ip - cpu->data)
            status[1] = '*';

        fprintf(cpu->log, "%s", status);
        fprintf(cpu->log, MEM_FMT, cpu->data[i]);
    }

    // stack
    for (uint32_t i = 0; i < 32; i++) {
        if (i % w == 0)
            fprintf(cpu->log, "\nstack  %06x: ", i);

        status[0] = ' ';
        status[1] = ' ';

        if (i == cpu->sp - cpu->stack)
            status[1] = '*';

        fprintf(cpu->log, "%s", status);
        fprintf(cpu->log, MEM_FMT, cpu->stack[i]);
    }

    // frames
    for (uint32_t i = 0; i < 32; i++) {
        if (i % w == 0)
            fprintf(cpu->log, "\nframes %06x: ", i);
        if (i == cpu->rp - cpu->frames)
            fprintf(cpu->log, " *");
        else
            fprintf(cpu->log, "  ");
        fprintf(cpu->log, MEM_FMT, cpu->frames[i]);
    }

    fprintf(cpu->log, "\n");

    /*
    fprintf(cpu->log, "ip: " MEM_FMT "\n", (uint32_t)(cpu->ip - cpu->code));
    fprintf(cpu->log, "sp: " MEM_FMT "\n", (uint32_t)(cpu->sp - cpu->stack));
    fprintf(cpu->log, "rp: " MEM_FMT "\n", (uint32_t)(cpu->rp - cpu->frames));
    fprintf(cpu->log, "cy: %lu\n", cpu->cycles);
    */
}

void
reset(stack_cpu_t *cpu)
{
    cpu->ip = &cpu->code[0];
    cpu->sp = &cpu->stack[0];
    cpu->rp = &cpu->frames[0];
    cpu->cycles = 0;
}

