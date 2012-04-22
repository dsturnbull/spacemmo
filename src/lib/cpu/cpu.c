#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/event.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include "src/lib/cpu/cpu.h"
#include "src/lib/cpu/hardware/tty.h"
#include "src/lib/cpu/hardware/disk.h"

uint8_t *prev;

cpu_t *
init_cpu()
{
    cpu_t *cpu = calloc(1, sizeof(*cpu));

    cpu->ip = cpu->mem;
    cpu->sp = &cpu->mem[CPU_STACK];
    cpu->bp = &cpu->mem[CPU_RET_STACK];

    // init kqueue
    if ((cpu->kq = kqueue()) == -1) {
        perror("kqueue");
        exit(1);
    }

    // register update timer
    memset(&cpu->ke, 0, sizeof(struct kevent));
    EV_SET(&cpu->ke, 0, EVFILT_TIMER, EV_ADD, NOTE_USECONDS, 1000, NULL);

    if (kevent(cpu->kq, &cpu->ke, 1, NULL, 0, NULL) == -1) {
        perror("kevent");
        exit(1);
    }

    cpu->log = stdout;
    cpu->tty = init_tty();
    cpu->disk = init_disk("/tmp/disk");

    cpu->halted = true;

    return cpu;
}

void
load_cpu(cpu_t *cpu, uint8_t *code, size_t len)
{
    memcpy(cpu->mem, code, len);
}

void
run_cpu(cpu_t *cpu)
{
    while (true) {
        if (cpu->mem[IRQ_CLK] ||
            cpu->mem[IRQ_KBD]) {

            memset(&cpu->ke, 0, sizeof(cpu->ke));
            if (kevent(cpu->kq, NULL, 0, &cpu->ke, 1, NULL) > 0) {
                if (cpu->ke.ident == IRQ_CLK) {
                    handle_timer(cpu);
                } else if (cpu->ke.ident == (uintptr_t)cpu->tty->master) {
                    handle_kbd(cpu);
                } else if (cpu->ke.ident == 0) {
                    // timer tick
                } else {
                    assert(false);
                }
            }
        }

        if (cpu->halted)
            return;
            //continue;

        step_cpu(cpu);
    }
}

void
step_cpu(cpu_t *cpu)
{
    uint32_t t;
    irq_t irq;
    op_t op = *(cpu->ip);
    prev = cpu->ip;

    LOG("%s - ip: %08x sp: %08x rp: %08x op: %08x cy: %08lx ",
            cpu->tty->fn,
            (uint32_t)(cpu->ip - cpu->mem),
            (uint32_t)(cpu->sp - &cpu->mem[CPU_STACK]),
            (uint32_t)(cpu->bp - &cpu->mem[CPU_RET_STACK]),
            op, cpu->cycles);

    switch (op) {
        case NOP:
            cpu->ip++;
            LOG("nop\n");
            break;

        case HLT:
            cpu->halted = true;
            LOG("halt\n");
            return;

        case LOAD:
            LOG("load %08x from %08x\n",
                    cpu->mem[*((uint32_t *)(cpu->sp - 4))],
                    *((uint32_t *)(cpu->sp - 4)));
            memcpy(cpu->sp - 4, &cpu->mem[*((uint32_t *)(cpu->sp - 4))], 4);
            cpu->ip++;
            break;

        case STORE:
            LOG("store %08x at %08x\n",
                    *((uint32_t *)(cpu->sp - 4)),
                    *((uint32_t *)(cpu->sp - 8)));
            t = *((uint32_t *)(cpu->sp - 4));
            memcpy(&cpu->mem[*((uint32_t *)(cpu->sp - 8))], &t, 4);
            cpu->sp -= 8;
            cpu->ip++;
            break;

        case ADD:
            t = *((uint32_t *)(cpu->sp - 4)) +
                *((uint32_t *)(cpu->sp - 8));

            LOG("add %08x + %08x == %08x\n",
                    *((uint32_t *)(cpu->sp - 4)),
                    *((uint32_t *)(cpu->sp - 8)),
                    t);
            memcpy(cpu->sp - 8, &t, 4);
            cpu->sp -= 4;
            cpu->ip++;
            break;

        case SUB:
            t = *((uint32_t *)(cpu->sp - 8)) -
                *((uint32_t *)(cpu->sp - 4));

            LOG("sub %08x - %08x == %08x\n",
                    *((uint32_t *)(cpu->sp - 8)),
                    *((uint32_t *)(cpu->sp - 4)),
                    t);
            memcpy(cpu->sp - 8, &t, 4);
            cpu->sp -= 4;
            cpu->ip++;
            break;

        /*
        case MUL:
            LOG("mul %08x * %08x == %08x\n",
                    *(cpu->sp), *(cpu->sp - 1),
                    *(cpu->sp) * *(cpu->sp - 1));
            *(cpu->sp - 1) = *(cpu->sp) * *(cpu->sp - 1);
            cpu->sp--;
            cpu->ip++;
            break;

        case DIV:
            t = *(cpu->sp - 1);
            LOG("div %08x / %08x == %08x r %08x\n",
                    *(cpu->sp), *(cpu->sp - 1),
                    *(cpu->sp) / *(cpu->sp - 1),
                    *(cpu->sp) % *(cpu->sp - 1));
            *(cpu->sp - 1) = *(cpu->sp) % t;
            *(cpu->sp) = *(cpu->sp) / t;
            cpu->ip++;
            break;

        case AND:
            LOG("and %08x & %08x == %08x\n",
                    *(cpu->sp), *(cpu->sp - 1),
                    *(cpu->sp) & *(cpu->sp - 1));
            *(cpu->sp - 1) = *(cpu->sp) & *(cpu->sp - 1);
            cpu->sp--;
            cpu->ip++;
            break;

        case OR:
            LOG("or %08x | %08x == %08x\n",
                    *(cpu->sp), *(cpu->sp - 1),
                    *(cpu->sp) | *(cpu->sp - 1));
            *(cpu->sp - 1) = *(cpu->sp) | *(cpu->sp - 1);
            cpu->sp--;
            cpu->ip++;
            break;
        */

        case JMP:
            LOG("jmp to %08x\n", *((uint32_t *)(cpu->sp - 4)));
            cpu->ip = &cpu->mem[*((uint32_t *)(cpu->sp - 4))];
            cpu->sp -= 4;
            break;

        case JE:
            LOG("je: ");

            if (*((uint32_t *)(cpu->sp - 8)) ==
                *((uint32_t *)(cpu->sp - 12))) {
                LOG("%08x == %08x, jumping %08x\n",
                    *((uint32_t *)(cpu->sp - 8)),
                    *((uint32_t *)(cpu->sp - 12)),
                    *((uint32_t *)(cpu->sp - 4)));
                cpu->ip = &cpu->mem[*((uint32_t *)(cpu->sp - 4))];
            } else {
                LOG("%08x != %08x, not jumping\n",
                    *((uint32_t *)(cpu->sp - 8)),
                    *((uint32_t *)(cpu->sp - 12)));
                cpu->ip++;
            }
            cpu->sp -= 12;

            break;

        case JZ:
            LOG("jz %08x ", *((uint32_t *)(cpu->sp - 4)));
            if (*((uint32_t *)(cpu->sp - 4)) == 0) {
                LOG("== 00000000, jumping to %08x\n",
                        *((uint32_t *)(cpu->sp - 8)));
                cpu->ip = &cpu->mem[*(uint32_t *)(cpu->sp - 8)];
            } else {
                LOG("!= 00000000, not jumping\n");
                cpu->ip++;
            }
            cpu->sp -= 8;
            break;

        case JNZ:
            LOG("jnz %08x ", *((uint32_t *)(cpu->sp - 4)));
            if (*((uint32_t *)(cpu->sp - 4)) != 0) {
                LOG("!= 00000000, jumping to %08x\n",
                        *((uint32_t *)(cpu->sp - 8)));
                cpu->ip = &cpu->mem[*(uint32_t *)(cpu->sp - 8)];
            } else {
                LOG("== 00000000, not jumping\n");
                cpu->ip++;
            }
            cpu->sp -= 8;
            break;

        case CALL:
            LOG("call %08x from %08lx\n",
                    *((uint32_t *)(cpu->sp - 4)), cpu->ip - cpu->mem);
            t = cpu->ip - cpu->mem + 1;
            cpu->ip = &cpu->mem[*((uint32_t *)(cpu->sp - 4))];
            memcpy(cpu->bp, &t, 4);
            cpu->bp += 4;
            cpu->sp -= 4;
            break;

        case RET:
            LOG("ret from %08lx to %08x\n",
                    cpu->ip - cpu->mem, *((uint32_t *)(cpu->bp - 4)));
            cpu->ip = &cpu->mem[*((uint32_t *)(cpu->bp - 4))];
            cpu->bp -= 4;
            break;

        case DUP:
            LOG("dup %08x\n", *((uint32_t *)(cpu->sp - 4)));
            memcpy(cpu->sp, cpu->sp - 4, 4);
            cpu->sp += 4;
            cpu->ip++;
            break;

        case PUSH:
            LOG("push %08x\n", *((uint32_t *)(cpu->ip + 1)));
            memcpy(cpu->sp, cpu->ip + 1, 4);
            cpu->sp += 4;
            cpu->ip += 4;
            cpu->ip++;
            break;

        case POP:
            LOG("pop %08x\n", *((uint32_t *)(cpu->sp - 4)));
            cpu->sp -= 4;
            cpu->ip++;
            break;

        case SWAP:
            LOG("swap\n");
            t = *((uint32_t *)(cpu->sp - 8));
            memcpy(cpu->sp - 8, cpu->sp - 4, 4);
            memcpy(cpu->sp - 4, &t, 4);
            cpu->ip++;
            break;

        case INT:
            irq = *((uint32_t *)(cpu->sp - 4));
            cpu->sp -= 4;

            switch (irq) {
                case IRQ_CLK:
                    LOG("clk timer %08x -> %08x\n",
                            *((uint32_t *)(cpu->sp - 8)),
                            *((uint32_t *)(cpu->sp - 4)));
                    set_timer_isr(cpu,
                            *((uint32_t *)(cpu->sp - 4)),
                            *((uint32_t *)(cpu->sp - 8)));
                    break;

                case IRQ_KBD:
                    LOG("kbd int %08x\n", *((uint32_t *)(cpu->sp - 4)));
                    set_kbd_isr(cpu, *((uint32_t *)(cpu->sp - 4)));
                    break;

                case IRQ_TTY:
                    LOG("tty out %08x\n", *((uint32_t *)(cpu->sp - 4)));
                    write_tty(cpu->tty, *((char *)(cpu->sp - 4)));
                    cpu->sp -= 4;
                    break;

                case IRQ_DISK_SET:
                    LOG("disk set %08x.. ", *(cpu->sp));
                    set_disk_position(cpu->disk, *(cpu->sp--));
                    LOG("%08lx\n", cpu->disk->pos);
                    break;

                case IRQ_DISK_RD:
                    LOG("disk read from %08lx: ", cpu->disk->pos);
                    t = *(cpu->sp);
                    *(cpu->sp) = read_disk(cpu->disk);
                    LOG("%08x\n", *(cpu->sp));
                    //handle_irq(cpu, &cpu->mem[t]);
                    break;

                case IRQ_DISK_WR:
                    LOG("disk write -> %08x @ %08lx\n", *(cpu->sp), cpu->disk->pos);
                    write_disk(cpu->disk, *(cpu->sp--));
                    break;

                default:
                    LOG("unhandled irq %08x\n", *(cpu->sp));
                    break;
            }

            cpu->ip++;
            break;

        default:
            assert(false);
            LOG("unknown instruction 0x%02x\n", op);
    }

    cpu->cycles++;

    print_region(cpu, cpu->ip, cpu->mem, 0x300, "code", 34);
    print_region(cpu, cpu->sp, &cpu->mem[CPU_STACK], 0x40, "stack", 31);
    print_region(cpu, cpu->bp, &cpu->mem[CPU_RET_STACK], 0x20, "rstack", 32);
}

void
reset_cpu(cpu_t *cpu)
{
}

void
print_region(cpu_t *cpu, uint8_t *p, uint8_t *data, size_t len,
        char *title, int c)
{
    for (size_t i = 0; i < len; i++) {
        if (i % 16 == 0) {
            if (i)
                LOG("\n");
            LOG("%12s %08lx: ", title, i);
        }

        if (p == cpu->ip) {
            if (&data[i] == prev) {
                printf("\033[36m");
            } else if (&data[i] == cpu->ip) {
                printf("\033[%im", c);
            }
        } else if (&data[i] - p == -4) {
            printf("\033[%im", c);
        }

        LOG(" %02x", data[i]);

        if (p == cpu->ip) {
            if (&data[i] == prev) {
                printf("\033[0m");
            } else if (&data[i] == cpu->ip) {
                printf("\033[0m");
            }
        } else if (&data[i] - p == -1) {
            printf("\033[0m");
        }
    }

    LOG("\n");
}

void
handle_irq(cpu_t *cpu, uint8_t *isr)
{
    uint8_t *ret = cpu->bp;
    uint32_t addr = cpu->ip - cpu->mem;
    memcpy(cpu->bp, &addr, 4);
    cpu->bp += 4;
    cpu->ip = isr;
    while (cpu->bp != ret)
        step_cpu(cpu);
}

void
set_timer_isr(cpu_t *cpu, uint32_t isr, uint32_t t)
{
    // register run timer
    memset(&cpu->ke, 0, sizeof(struct kevent));
    EV_SET(&cpu->ke, IRQ_CLK, EVFILT_TIMER, EV_ADD | EV_CLEAR,
            NOTE_USECONDS, t, NULL);

    if (kevent(cpu->kq, &cpu->ke, 1, NULL, 0, NULL) == -1) {
        perror("setting timer");
        exit(1);
    }

    memcpy(&cpu->mem[IRQ_CLK], &isr, 4);
}

void
handle_timer(cpu_t *cpu)
{
    LOG("timer, jumping to %08x\n", *(uint32_t *)(&cpu->mem[IRQ_CLK]));
    handle_irq(cpu, &cpu->mem[cpu->mem[IRQ_CLK]]);
}

void
set_kbd_isr(cpu_t *cpu, uint32_t isr)
{
    // watch for input
    memset(&cpu->ke, 0, sizeof(struct kevent));
    EV_SET(&cpu->ke, cpu->tty->master, EVFILT_READ, EV_ADD | EV_CLEAR,
            0, 0, NULL);

    if (kevent(cpu->kq, &cpu->ke, 1, NULL, 0, NULL) == -1) {
        perror("kbd read");
        exit(1);
    }

    memcpy(&cpu->mem[IRQ_KBD], &isr, 4);
}

void
handle_kbd(cpu_t *cpu)
{
    char c;
    if (read_tty(cpu->tty, &c)) {
        memset(cpu->sp, 0, 4);
        *(cpu->sp) = c;
        cpu->sp += 4;
        LOG("char: %02x, jumping to %08x\n",
                c, *(uint32_t *)(&cpu->mem[IRQ_KBD]));
        handle_irq(cpu, &cpu->mem[*(uint32_t *)(&cpu->mem[IRQ_KBD])]);
    }
}

