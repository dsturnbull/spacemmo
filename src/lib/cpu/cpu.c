#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "src/lib/spacemmo.h"
#include "src/lib/cpu/cpu.h"
#include "src/lib/cpu/hardware/tty.h"
#include "src/lib/cpu/hardware/disk.h"
#include "src/lib/cpu/hardware/port.h"

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
    cpu->port0 = init_port(0);
    cpu->port1 = init_port(1);
    cpu->port2 = init_port(2);
    cpu->port3 = init_port(3);

    cpu->halted = true;

    return cpu;
}

void
free_cpu(cpu_t *cpu)
{
    // TODO free everything
    free(cpu);
}

void
load_cpu(cpu_t *cpu, char *sys_file)
{
    char *dbg_file = replace_ext(sys_file, ".dbg");
    FILE *sys_fp, *dbg_fp;
    struct stat st;

    // load code
    if ((sys_fp = fopen(sys_file, "r")) == NULL) {
        perror("sys_file");
        exit(1);
    }

    memset(&st, 0, sizeof(st));
    if (stat(sys_file, &st) < 0) {
        perror("sys_file");
        exit(1);
    }

    char *sys_buf = malloc(st.st_size);
    fread(sys_buf, st.st_size, 1, sys_fp);
    fclose(sys_fp);
    memcpy(cpu->mem, sys_buf, st.st_size);
    free(sys_buf);

    // load source lookup
    if ((dbg_fp = fopen(dbg_file, "r")) == NULL) {
        perror("dbg_file");
        exit(1);
    }

    cpu->src = malloc(st.st_size * sizeof(char *));
    while (true) {
        int pos, len;
        if (fread(&pos, sizeof(pos), 1, dbg_fp) == 0)
            break;
        fread(&len, sizeof(len), 1, dbg_fp);

        cpu->src[pos] = malloc(len + 1);
        fread(cpu->src[pos], len, 1, dbg_fp);
        cpu->src[pos][len] = '\0';
    }
}

void
run_cpu(cpu_t *cpu)
{
    while (true) {
        memset(&cpu->ke, 0, sizeof(cpu->ke));
        if (kevent(cpu->kq, NULL, 0, &cpu->ke, 1, NULL) > 0) {
            if (cpu->mem[IRQ_CLK] && cpu->ke.ident == IRQ_CLK)
                handle_timer(cpu);

            if (cpu->mem[IRQ_KBD] &&
                    cpu->ke.ident == (uintptr_t)cpu->tty->master)
                handle_kbd(cpu);

            for (int i = 0; i < 4; i++) {
                port_t *port = cpu->port0 + i;
                if (cpu->mem[IRQ_P0_IN + i * 8]) {
                    if (cpu->ke.ident == (uintptr_t)port->r) {
                        LOG("(w) %li bytes on port %i\n", cpu->ke.data, i);
                        handle_port_read(cpu, port);
                    }
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
    uint32_t t, v, u;
    irq_t irq;
    op_t op = *(cpu->ip);
    prev = cpu->ip;

    LOG("%08lx ", cpu->ip - cpu->mem);

    t = *((uint32_t *)(cpu->sp - 4));
    v = *((uint32_t *)(cpu->sp - 8));

    char *line = cpu->src[(uint32_t)(cpu->ip - cpu->mem)];
    LOG("[\033[36m%-80s\033[0m] ", line);

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
            LOG("load %08x from %08x\n", cpu->mem[t], t);
            memcpy(cpu->sp - 4, &cpu->mem[t], 4);
            cpu->ip++;
            break;

        case STORE:
            LOG("store %08x at %08x\n", t, v);
            memcpy(&cpu->mem[v], &t, 4);
            cpu->sp -= 8;
            cpu->ip++;
            break;

        case ADD:
            u = t + v;
            LOG("add %08x + %08x == %08x\n", t, v, u);
            memcpy(cpu->sp - 8, &u, 4);
            cpu->sp -= 4;
            cpu->ip++;
            break;

        case SUB:
            u = v - t;
            LOG("sub %08x - %08x == %08x\n", v, t, u);
            memcpy(cpu->sp - 8, &u, 4);
            cpu->sp -= 4;
            cpu->ip++;
            break;

        case MUL:
            u = t * v;
            LOG("mul %08x * %08x == %08x\n", t, v, u);
            memcpy(cpu->sp - 8, &u, 4);
            cpu->sp -= 8;
            cpu->ip++;
            break;

        case DIV:
            LOG("div %08x / %08x == %08x r %08x\n", t, v, t / v, t % v);
            u = t % v;
            memcpy(cpu->sp - 4, &u, 4);
            u = t / v;
            memcpy(cpu->sp - 8, &u, 4);
            cpu->ip++;
            break;

        case AND:
            u = t & v;
            LOG("and %08x & %08x == %08x\n", t, v, u);
            memcpy(cpu->sp - 8, &u, 4);
            cpu->sp -= 8;
            cpu->ip++;
            break;

        case OR:
            u = t | v;
            LOG("or %08x | %08x == %08x\n", t, v, u);
            memcpy(cpu->sp - 8, &u, 4);
            cpu->sp -= 8;
            cpu->ip++;
            break;

        case JMP:
            LOG("jmp to %08x\n", t);
            cpu->ip = &cpu->mem[t];
            cpu->sp -= 4;
            break;

        case JE:
            LOG("je: ");
            u = *((uint32_t *)(cpu->sp - 12));
            if (v == u) {
                LOG("%08x == %08x, jumping %08x\n", v, u, t);
                cpu->ip = &cpu->mem[t];
            } else {
                LOG("%08x != %08x, not jumping\n", v, u);
                cpu->ip++;
            }
            cpu->sp -= 12;
            break;

        case JZ:
            LOG("jz %08x ", t);
            if (t == 0) {
                LOG("== 00000000, jumping to %08x\n", v);
                cpu->ip = &cpu->mem[v];
            } else {
                LOG("!= 00000000, not jumping\n");
                cpu->ip++;
            }
            cpu->sp -= 8;
            break;

        case JNZ:
            LOG("jnz %08x ", t);
            if (t != 0) {
                LOG("!= 00000000, jumping to %08x\n", v);
                cpu->ip = &cpu->mem[v];
            } else {
                LOG("== 00000000, not jumping\n");
                cpu->ip++;
            }
            cpu->sp -= 8;
            break;

        case CALL:
            LOG("call %08x from %08lx\n", t, cpu->ip - cpu->mem);
            u = cpu->ip - cpu->mem + 1;
            cpu->ip = &cpu->mem[t];
            memcpy(cpu->bp, &u, 4);
            cpu->bp += 4;
            cpu->sp -= 4;
            break;

        case RET:
            u = *((uint32_t *)(cpu->bp - 4));
            LOG("ret from %08lx to %08x\n", cpu->ip - cpu->mem, u);
            cpu->ip = &cpu->mem[u];
            cpu->bp -= 4;
            break;

        case DUP:
            LOG("dup %08x\n", t);
            memcpy(cpu->sp, cpu->sp - 4, 4);
            cpu->sp += 4;
            cpu->ip++;
            break;

        case PUSH:
            u = *((uint32_t *)(cpu->ip + 1));
            LOG("push %08x\n", u);
            memcpy(cpu->sp, cpu->ip + 1, 4);
            cpu->sp += 4;
            cpu->ip += 4;
            cpu->ip++;
            break;

        case POP:
            LOG("pop %08x\n", t);
            cpu->sp -= 4;
            cpu->ip++;
            break;

        case SWAP:
            LOG("swap\n");
            memcpy(cpu->sp - 8, cpu->sp - 4, 4);
            memcpy(cpu->sp - 4, &v, 4);
            cpu->ip++;
            break;

        case INT:
            irq = *((uint32_t *)(cpu->sp - 4));
            cpu->sp -= 4;
            t = *((uint32_t *)(cpu->sp - 4));
            v = *((uint32_t *)(cpu->sp - 8));

            switch (irq) {
                case IRQ_DBG:
                    LOG("debug %i\n", t);
                    cpu->debug = t == 1;
                    break;

                case IRQ_CLK:
                    LOG("clk timer %08x -> %08x\n", v, t);
                    set_timer_isr(cpu, t, v);
                    cpu->sp -= 8;
                    break;

                case IRQ_KBD:
                    LOG("kbd int %08x\n", t);
                    set_kbd_isr(cpu, t);
                    cpu->sp -= 4;
                    break;

                case IRQ_TTY:
                    LOG("tty out %08x\n", t);
                    write_tty(cpu->tty, t);
                    cpu->sp -= 4;
                    break;

                case IRQ_DISK_SET:
                    LOG("disk set %08x.. ", t);
                    set_disk_position(cpu->disk, t);
                    LOG("%08lx\n", cpu->disk->pos);
                    break;

                case IRQ_DISK_RD:
                    LOG("disk read from %08lx: ", cpu->disk->pos);
                    u = read_disk(cpu->disk);
                    memcpy(cpu->sp - 4, &u, 4);
                    LOG("%08x\n", u);
                    //handle_irq(cpu, &cpu->mem[t]);
                    break;

                case IRQ_DISK_WR:
                    LOG("disk write -> %08x @ %08lx\n", t, cpu->disk->pos);
                    write_disk(cpu->disk, t);
                    break;

                case IRQ_P0_IN:
                case IRQ_P1_IN:
                case IRQ_P2_IN:
                case IRQ_P3_IN:
                    LOG("port %i -> %08x\n", (irq - IRQ_P0_IN) / 8, t);
                    set_port_isr(cpu,        (irq - IRQ_P0_IN) / 8, t);
                    cpu->sp -= 4;
                    break;

                case IRQ_P0_OUT:
                case IRQ_P1_OUT:
                case IRQ_P2_OUT:
                case IRQ_P3_OUT:
                    LOG("port %i < %02x\n", (irq - IRQ_P0_OUT) / 8, t);
                    write_port(cpu->port0 + (irq - IRQ_P0_OUT) / 8, t);
                    cpu->sp -= 4;
                    break;
            }

            cpu->ip++;
            break;
    }

    cpu->cycles++;

    //print_region(cpu, cpu->ip, cpu->mem, 0x60, "code", 34);
    //print_region(cpu, cpu->sp, &cpu->mem[CPU_STACK], 0x40, "stack", 31);
    //print_region(cpu, cpu->bp, &cpu->mem[CPU_RET_STACK], 0x20, "rstack", 32);
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
                LOG("\033[36m");
            } else if (&data[i] == cpu->ip) {
                LOG("\033[%im", c);
            }
        } else if (&data[i] - p == -4) {
            LOG("\033[%im", c);
        }

        LOG(" %02x", data[i]);

        if (p == cpu->ip) {
            if (&data[i] == prev) {
                LOG("\033[0m");
            } else if (&data[i] == cpu->ip) {
                LOG("\033[0m");
            }
        } else if (&data[i] - p == -1) {
            LOG("\033[0m");
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

    // faffery - go backwards in order to find the label that the
    // irq handler is in
    int c = 0;
    char *prev = cpu->src[(uint32_t)(cpu->ip - cpu->mem) - 1];
    char *curr = cpu->src[(uint32_t)(cpu->ip - cpu->mem)];
    while (true) {
        if (cpu->src[c] == prev) {
            while (true) {
                if (cpu->src[c] != prev && cpu->src[c] != curr &&
                    strlen(cpu->src[c]) > 0) {
                    LOG("%08lx ", cpu->ip - cpu->mem);
                    LOG("[\033[36m%-80s\033[0m]\n", cpu->src[c]);
                }
                if (cpu->src[c++] == curr)
                    break;
            }
            break;
        }
        c++;
    }

    uint8_t *old_ip;
    while (cpu->bp != ret) {
        old_ip = cpu->ip;
        step_cpu(cpu);
    }
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

void
set_port_isr(cpu_t *cpu, int portno, uint32_t isr)
{
    port_t *port = cpu->port0 + portno;
    // watch for input
    memset(&cpu->ke, 0, sizeof(struct kevent));
    EV_SET(&cpu->ke, port->r, EVFILT_READ, EV_ADD | EV_CLEAR,
            0, 0, NULL);

    if (kevent(cpu->kq, &cpu->ke, 1, NULL, 0, NULL) == -1) {
        perror("port read");
        exit(1);
    }

    // watch for input
    memset(&cpu->ke, 0, sizeof(struct kevent));
    EV_SET(&cpu->ke, port->w, EVFILT_READ, EV_ADD | EV_CLEAR,
            0, 0, NULL);

    if (kevent(cpu->kq, &cpu->ke, 1, NULL, 0, NULL) == -1) {
        perror("port read");
        exit(1);
    }

    memcpy(&cpu->mem[IRQ_P0_IN + portno * 8], &isr, 4);
}

void
handle_port_read(cpu_t *cpu, port_t *port)
{
    char c;
    while (read_port(port, &c)) {
        memset(cpu->sp, 0, 4);
        *(cpu->sp) = c;
        cpu->sp += 4;
        uint32_t ptr = *(uint32_t *)(&cpu->mem[IRQ_P0_IN + port->n]);
        LOG("port char: %02x, jumping to %08x\n", c, ptr);
        handle_irq(cpu, &cpu->mem[ptr]);
    }
}

