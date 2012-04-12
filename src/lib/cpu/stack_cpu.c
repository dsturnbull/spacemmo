#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

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
    return cpu;
}

void
load_prog(stack_cpu_t *stack_cpu, uint8_t *prog, size_t len)
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
    ins_t *ins = (ins_t *)cpu->ip;
    uint32_t t;

    // when ip is set, make it point to the previous instruction,
    // because this loop will increment it at the end.

    if (cpu->debug) {
        printf( "ip: %02x "
                "sp: %02x "
                "rp: %02x "
                "op: %02x: "
                "cy: %lu ",
                (uint32_t)(cpu->ip - cpu->code),
                (uint32_t)(cpu->sp - cpu->stack),
                (uint32_t)(cpu->rp - cpu->frames),
                ins->op,
                cpu->cycles);
    }

    cpu->cycles++;
    //gettimeofday(&cpu->time, NULL);
    //cpu->mem[STACK_CPU_CLOCK] = cpu->time.tv_sec;

    switch (ins->op) {
        case PUSH:
            if (cpu->debug)
                printf("push\n");
            t = (int)pow(2, ins->opt);
            cpu->ip++;
            memcpy(cpu->sp, cpu->ip, t);
            cpu->sp += t;
            cpu->ip += t;
            break;

        case POP:
            if (cpu->debug)
                printf("pop\n");
            break;

        default:
            fprintf(stderr, "unknown instruction %i\n", ins->op);
            exit(1);
    }

    /*
        case HLT:
            if (cpu->debug)
                printf("halt\n");
            cpu->halted = true;
            return;

        case LOAD:
            if (cpu->debug)
                printf("loading %02x from %02x\n",
                        cpu->data[*(cpu->sp)], *(cpu->sp));
            *(cpu->sp) = cpu->data[*(cpu->sp)];
            break;

        case STORE:
            if (cpu->debug)
                printf("storing %02x at %02x\n",
                        *(cpu->sp),
                        *(cpu->sp - 1));
            cpu->data[*(cpu->sp--)] = *(cpu->sp--);
            break;

        case ADD:
            if (cpu->debug)
                printf("%02x + %02x == %02x\n",
                        *(cpu->sp - 1),
                        *(cpu->sp),
                        *(cpu->sp - 1) + *(cpu->sp));
            *(cpu->sp - 1) = *(cpu->sp - 1) + *(cpu->sp);
            cpu->sp--;
            break;

        case SUB:
            if (cpu->debug)
                printf("%02x - %02x == %02x\n",
                        *(cpu->sp - 1),
                        *(cpu->sp),
                        *(cpu->sp - 1) - *(cpu->sp));
            *(cpu->sp - 1) = *(cpu->sp - 1) - *(cpu->sp);
            cpu->sp--;
            break;

        case MUL:
            assert(false);

        case DIV:
            if (cpu->debug)
                printf("%02x / %02x\n",
                    *(cpu->sp - 1), *(cpu->sp));
            t = *(cpu->sp);
            *(cpu->sp) = *(cpu->sp - 1) % *(cpu->sp);
            *(cpu->sp - 1) = *(cpu->sp - 1) / t;
            break;

        case JMP:
            if (cpu->debug)
                printf("jump %02x\n", *(cpu->sp));
            cpu->ip = &cpu->code[*(cpu->sp--)] - 1;
            break;

        case JZ:
            if (cpu->debug)
                printf("%02x == 0 ", *(cpu->sp - 1));
            if (*(cpu->sp - 1) == 0) {
                if (cpu->debug)
                    printf("jumping to %02x\n", *(cpu->sp));
                cpu->ip = &cpu->code[*(cpu->sp)] - 1;
            } else {
                if (cpu->debug)
                    printf("not jumping\n");
            }
            cpu->sp -= 2;
            break;

        case JNZ:
            if (cpu->debug)
                printf("%02x != 0 ", *(cpu->sp - 1));
            if (*(cpu->sp - 1) != 0) {
                if (cpu->debug)
                    printf("jumping to %02x\n", *(cpu->sp));
                cpu->ip = &cpu->code[*(cpu->sp)] - 1;
            } else {
                if (cpu->debug)
                    printf("not jumping\n");
            }
            cpu->sp -= 2;
            break;

        case CALL:
            if (cpu->debug)
                printf("calling %02x from %02x\n",
                        *(cpu->sp),
                        (uint32_t)(cpu->ip - cpu->code));
            *(cpu->rp++) = cpu->ip - cpu->code;
            cpu->ip = &cpu->code[*(cpu->sp--)] - 1;
            break;

        case RET:
            if (cpu->debug)
                printf("returning from %02x to %02x\n",
                        (uint32_t)(cpu->ip - cpu->code),
                        *(cpu->rp - 1));
            cpu->ip = cpu->code + *(--cpu->rp);
            break;

        case DUP:
            if (cpu->debug)
                printf("dup of %02x\n", *(cpu->sp));
            *(++cpu->sp) = *(cpu->sp);
            break;

        case POP:
            if (cpu->debug)
                printf("pop\n");
            cpu->sp--;
            break;

        case SWAP:
            if (cpu->debug)
                printf("swap\n");
            t = *(cpu->sp);
            *(cpu->sp) = *(cpu->sp - 1);
            *(cpu->sp - 1) = t;
            break;

        case INT:
            if (cpu->debug)
                printf("int\n");
            handle_interrupt(cpu);
            break;

        case DEBUG:
            printf("debugging on\n");
            cpu->debug = true;
            break;

        case PUSH:
            if (cpu->debug)
                printf("pushing %02x\n", *(cpu->ip + 1));
            *(++cpu->sp) = *(++cpu->ip);
            break;
    }
    */

    //if (cpu->debug)
    //    print_state(cpu);

    cpu->ip++;
}

void
handle_interrupt(stack_cpu_t *cpu)
{
    uint32_t t = *(cpu->sp--);
    uint32_t c;
    irq_t rsrc = *(cpu->sp--);
    uint8_t kbd_buf[UINT8_MAX];

    switch (rsrc) {
        case IRQ_TTY:
            ttyp(cpu->tty, t);
            break;

        case IRQ_KBD:
            keyboard_state(cpu->kbd, kbd_buf);
            //*(++cpu->sp) = c;
            *(cpu->rp++) = cpu->ip - cpu->code;
            cpu->ip = &cpu->code[t] - 1;

            break;

        default:
            if (cpu->debug)
                printf("unknown resource %02x\n", rsrc);
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
    for (uint32_t i = 0; i < 160; i++) {
        if (i % w == 0)
            printf("\ncode   %06x: ", i);

        status[0] = ' ';
        status[1] = ' ';

        if (i == cpu->ip - cpu->code)
            status[1] = '*';

        printf("%s", status);
        printf("%02x", cpu->code[i]);
    }

    // data
    for (int i = 0; i < 128; i++) {
        if (i % w == 0)
            printf("\ndata   %06x: ", i);

        status[0] = ' ';
        status[1] = ' ';

        if (i == cpu->ip - cpu->data)
            status[1] = '*';

        printf("%s", status);
        printf("%02x", cpu->data[i]);
    }

    // stack
    for (uint32_t i = 0; i < 32; i++) {
        if (i % w == 0)
            printf("\nstack  %06x: ", i);

        status[0] = ' ';
        status[1] = ' ';

        if (i == cpu->sp - cpu->stack)
            status[1] = '*';

        printf("%s", status);
        printf("%02x", cpu->stack[i]);
    }

    // frames
    for (uint32_t i = 0; i < 32; i++) {
        if (i % w == 0)
            printf("\nframes %06x: ", i);
        if (i == cpu->rp - cpu->frames)
            printf(" *");
        else
            printf("  ");
        printf("%02x", cpu->frames[i]);
    }

    printf("\n");

    /*
    printf("ip: " MEM_FMT "\n", (uint32_t)(cpu->ip - cpu->code));
    printf("sp: " MEM_FMT "\n", (uint32_t)(cpu->sp - cpu->stack));
    printf("rp: " MEM_FMT "\n", (uint32_t)(cpu->rp - cpu->frames));
    printf("cy: %lu\n", cpu->cycles);
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

