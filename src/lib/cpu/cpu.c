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
    cpu->rp = &cpu->mem[CPU_RET_STACK];

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

    cpu->opmap = malloc(32 * sizeof(uintptr_t));
    cpu->opmap[NOP]     = &handle_nop;
    cpu->opmap[HLT]     = &handle_hlt;
    cpu->opmap[CALL]    = &handle_call;
    cpu->opmap[RET]     = &handle_ret;
    cpu->opmap[LOAD]    = &handle_load;
    cpu->opmap[STORE]   = &handle_store;
    cpu->opmap[ADD]     = &handle_add;
    cpu->opmap[SUB]     = &handle_sub;
    cpu->opmap[MUL]     = &handle_mul;
    cpu->opmap[DIV]     = &handle_div;
    cpu->opmap[AND]     = &handle_and;
    cpu->opmap[OR]      = &handle_or;
    cpu->opmap[JMP]     = &handle_jmp;
    cpu->opmap[JE]      = &handle_je;
    cpu->opmap[JNE]     = &handle_jne;
    cpu->opmap[JZ]      = &handle_jz;
    cpu->opmap[JNZ]     = &handle_jnz;
    cpu->opmap[PUSH]    = &handle_push;
    cpu->opmap[DUP]     = &handle_dup;
    cpu->opmap[POP]     = &handle_pop;
    cpu->opmap[SWAP]    = &handle_swap;
    cpu->opmap[INT]     = &handle_int;

    wait_tty_slave(cpu->tty);

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
    //if ((dbg_fp = fopen(dbg_file, "r")) == NULL) {
    //    perror("dbg_file");
    //    exit(1);
    //}

    //cpu->src = calloc(st.st_size, sizeof(char *));
    //while (true) {
    //    int pos, len;
    //    if (fread(&pos, sizeof(pos), 1, dbg_fp) == 0)
    //        break;
    //    fread(&len, sizeof(len), 1, dbg_fp);

    //    cpu->src[pos] = malloc(len + 1);
    //    fread(cpu->src[pos], len, 1, dbg_fp);
    //    cpu->src[pos][len] = '\0';
    //}
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
                if (*(uint32_t *)(&cpu->mem[IRQ_P0_IN + i * 8])) {
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
    irq_t irq;
    opcode_t opcode = *(opcode_t *)cpu->ip;
    prev = cpu->ip;

    //char *line = cpu->src[(uint32_t)(cpu->ip - cpu->mem)];
    //LOG("[\033[36m%-97s\033[0m] ", line);

    LOG("%016lx op:%02x flags:%02x ip: %016lx rp: %016lx sp: %016lx ",
            cpu->ip - cpu->mem, opcode.op, opcode.flags,
            cpu->ip - cpu->mem,
            cpu->rp - &cpu->mem[CPU_RET_STACK],
            cpu->sp - &cpu->mem[CPU_STACK]);

    handle_op(cpu, &opcode);
    cpu->cycles++;
}

void
reset_cpu(cpu_t *cpu)
{
}

void
print_region(cpu_t *cpu, uint8_t *p, uint8_t *data, size_t len, int c)
{
    for (size_t i = 0; i < len; i++) {
        if (i % 32 == 0) {
            if (i)
                LOG("\n");
            LOG("%04lx: ", data - cpu->mem + i);
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
handle_op(cpu_t *cpu, opcode_t *opcode)
{
    LOG("(%02x ", *(uint8_t *)opcode);
    for (int i = 0; i < 8; i++)
        LOG("%i", (*(uint8_t *)opcode & (1 << i)) != 0);
    LOG(") ");

    instruction_t instruction;
    memcpy(&instruction, opcode, sizeof(*opcode));
    instruction.len = 1 << opcode->flags;

    LOG("len: %lu ", instruction.len);

    void (*op)(cpu_t *, instruction_t *) = cpu->opmap[opcode->op];
    if (op)
        op(cpu, &instruction);
    else
        exit(1);
}

void
handle_nop(cpu_t *cpu, instruction_t *op)
{
    cpu->ip++;
    LOG("nop\n");
}

void
handle_hlt(cpu_t *cpu, instruction_t *op)
{
    cpu->halted = true;
    LOG("halt\n");
}

void
handle_call(cpu_t *cpu, instruction_t *op)
{
    uint64_t addr = *(uint64_t *)(cpu->sp - 8);
    LOG("call %016lx -> %016llx\n", cpu->ip - cpu->mem, addr);
    uint64_t ret = cpu->ip - cpu->mem + 1;
    cpu->ip = &cpu->mem[addr];
    memcpy(cpu->rp, &ret, sizeof(ret));
    cpu->rp += 8;
    cpu->sp -= 8;
}

void
handle_ret(cpu_t *cpu, instruction_t *op)
{
    uint64_t ret = *((uint64_t *)(cpu->rp - 8));
    LOG("ret  %016llx <- %016lx\n", ret, cpu->ip - cpu->mem);
    cpu->ip = &cpu->mem[ret];
    cpu->rp -= sizeof(ret);
}

void
handle_load(cpu_t *cpu, instruction_t *op)
{
    uint64_t loc = 0, val = 0;
    memcpy(&loc, cpu->sp - 8, 8);
    cpu->sp -= 8;

    memcpy(&val, &cpu->mem[loc], op->len);
    memcpy(cpu->sp, &val, op->len);
    cpu->sp += op->len;

    LOG("load %016llx <- %016llx\n", val, loc);
    cpu->ip++;
}

void
handle_store(cpu_t *cpu, instruction_t *op)
{
    uint64_t val = 0, loc = 0;

    cpu->sp -= op->len;
    memcpy(&val, cpu->sp, op->len);

    cpu->sp -= 8;
    memcpy(&loc, cpu->sp, 8);

    LOG("stor %016llx -> %016llx\n", val, loc);
    memcpy(&cpu->mem[loc], &val, op->len);
    cpu->ip++;
}

void
handle_add(cpu_t *cpu, instruction_t *op)
{
    uint64_t a = 0, b = 0;
    memcpy(&a, cpu->sp - op->len * 1, op->len);
    memcpy(&b, cpu->sp - op->len * 2, op->len);
    uint64_t r = b + a;
    LOG("add  %016llx + %016llx == %016llx\n", a, b, r);
    memcpy(cpu->sp - op->len * 2, &r, op->len);
    cpu->sp -= op->len;
    cpu->ip++;
}

void
handle_sub(cpu_t *cpu, instruction_t *op)
{
    uint64_t a = 0, b = 0;
    memcpy(&a, cpu->sp - op->len * 1, op->len);
    memcpy(&b, cpu->sp - op->len * 2, op->len);
    uint64_t r = b - a;
    LOG("sub  %016llx - %016llx == %016llx\n", a, b, r);
    memcpy(cpu->sp - op->len * 2, &r, op->len);
    cpu->sp -= op->len;
    cpu->ip++;
}

void
handle_mul(cpu_t *cpu, instruction_t *op)
{
    uint64_t a = 0, b = 0;
    memcpy(&a, cpu->sp - op->len * 1, op->len);
    memcpy(&b, cpu->sp - op->len * 2, op->len);
    uint64_t r = b * a;
    LOG("mul  %016llx * %016llx == %016llx\n", a, b, r);
    memcpy(cpu->sp - op->len * 2, &r, op->len);
    cpu->sp -= op->len;
    cpu->ip++;
}

void
handle_div(cpu_t *cpu, instruction_t *op)
{
    uint64_t a = 0, b = 0;
    memcpy(&a, cpu->sp - op->len * 1, op->len);
    memcpy(&b, cpu->sp - op->len * 2, op->len);
    uint64_t q = b / a;
    uint64_t r = b % a;
    LOG("div  %016llx / %016llx == %016llx r %016llx\n", a, b, q, r);
    memcpy(cpu->sp - op->len * 1, &q, op->len);
    memcpy(cpu->sp - op->len * 2, &r, op->len);
    cpu->ip++;
}

void
handle_and(cpu_t *cpu, instruction_t *op)
{
    uint64_t a = 0, b = 0;
    memcpy(&a, cpu->sp - op->len * 1, op->len);
    memcpy(&b, cpu->sp - op->len * 2, op->len);
    uint64_t r = b & a;
    LOG("and  %016llx & %016llx == %016llx\n", a, b, r);
    memcpy(cpu->sp - op->len * 2, &r, op->len);
    cpu->sp -= op->len;
    cpu->ip++;
}

void
handle_or(cpu_t *cpu, instruction_t *op)
{
    uint64_t a = 0, b = 0;
    memcpy(&a, cpu->sp - op->len * 1, op->len);
    memcpy(&b, cpu->sp - op->len * 2, op->len);
    uint64_t r = b | a;
    LOG("or   %016llx | %016llx == %016llx\n", a, b, r);
    memcpy(cpu->sp - op->len * 2, &r, op->len);
    cpu->sp -= op->len;
    cpu->ip++;
}

void
handle_jmp(cpu_t *cpu, instruction_t *op)
{
    uint64_t loc;
    cpu->sp -= 8;
    memcpy(&loc, cpu->sp, 8);
    LOG("jmp  %016llx\n", loc);
    cpu->ip = &cpu->mem[loc];
}

void
handle_je(cpu_t *cpu, instruction_t *op)
{
    uint64_t a = 0, b = 0, loc = 0;

    cpu->sp -= 8;
    memcpy(&loc, cpu->sp, 8);

    cpu->sp -= op->len;
    memcpy(&a,   cpu->sp, op->len);

    cpu->sp -= op->len;
    memcpy(&b,   cpu->sp, op->len);

    if (a == b) {
        LOG("je   %016llx == %016llx -> %016llx\n", a, b, loc);
        cpu->ip = &cpu->mem[loc];
    } else {
        LOG("je   %016llx != %016llx\n", a, b);
        cpu->ip++;
    }
}

void
handle_jne(cpu_t *cpu, instruction_t *op)
{
    uint64_t a = 0, b = 0, loc = 0;

    cpu->sp -= 8;
    memcpy(&loc, cpu->sp, 8);

    cpu->sp -= op->len;
    memcpy(&a,   cpu->sp, op->len);

    cpu->sp -= op->len;
    memcpy(&b,   cpu->sp, op->len);

    if (a != b) {
        LOG("jne  %016llx != %016llx -> %016llx\n", a, b, loc);
        cpu->ip = &cpu->mem[loc];
    } else {
        LOG("jne  %016llx == %016llx\n", a, b);
        cpu->ip++;
    }
}

void
handle_jz(cpu_t *cpu, instruction_t *op)
{
    uint64_t arg = 0, loc = 0;

    cpu->sp -= 8;
    memcpy(&loc, cpu->sp, 8);

    cpu->sp -= op->len;
    memcpy(&arg, cpu->sp, op->len);

    if (arg == 0) {
        LOG("jz   %016llx == 0 -> %016llx\n", arg, loc);
        cpu->ip = &cpu->mem[loc];
    } else {
        LOG("jz   %016llx != 0\n", arg);
        cpu->ip++;
    }
}

void
handle_jnz(cpu_t *cpu, instruction_t *op)
{
    uint64_t arg = 0, loc = 0;

    cpu->sp -= 8;
    memcpy(&loc, cpu->sp, 8);

    cpu->sp -= op->len;
    memcpy(&arg, cpu->sp, op->len);

    if (arg != 0) {
        LOG("jz   %016llx != 0 -> %016llx\n", arg, loc);
        cpu->ip = &cpu->mem[loc];
    } else {
        LOG("jz   %016llx == 0\n", arg);
        cpu->ip++;
    }
}

void
handle_dup(cpu_t *cpu, instruction_t *op)
{
    uint64_t arg = 0;
    memcpy(&arg, cpu->sp - op->len, op->len);
    LOG("dup  %016llx\n", arg);
    memcpy(cpu->sp, &arg, op->len);
    cpu->sp += op->len;
    cpu->ip++;
}

void
handle_push(cpu_t *cpu, instruction_t *op)
{
    uint64_t arg = 0;
    memcpy(&arg, cpu->ip + 1, op->len);
    LOG("push %016llx\n", arg);
    memcpy(cpu->sp, cpu->ip + 1, op->len);
    cpu->sp += op->len;
    cpu->ip += op->len;
    cpu->ip++;
}

void
handle_pop(cpu_t *cpu, instruction_t *op)
{
    uint64_t arg = 0;
    memcpy(&arg, cpu->sp - op->len, op->len);
    LOG("pop  %016llx\n", arg);
    cpu->sp -= op->len;
    cpu->ip++;
}

void
handle_swap(cpu_t *cpu, instruction_t *op)
{
    uint64_t a = 0, b = 0;
    memcpy(&a, cpu->sp - op->len * 1, op->len);
    memcpy(&b, cpu->sp - op->len * 2, op->len);
    LOG("swap %016llx <> %016llx\n", a, b);
    memcpy(cpu->sp - op->len * 1, &b, op->len);
    memcpy(cpu->sp - op->len * 2, &a, op->len);
    cpu->ip++;
}

void
handle_int(cpu_t *cpu, instruction_t *op)
{
    irq_t irq;
    uint64_t isr;
    switch (op->op) {
        case INT:
            irq = *((uint64_t *)(cpu->sp - 8));
            isr = *((uint64_t *)(cpu->sp - 16));
            cpu->sp -= 16;

            switch (irq) {
                /*
                case IRQ_DBG:
                    LOG("debug %llu\n", t);
                    cpu->debug = t == 1;
                    break;
                */

                case IRQ_CLK:
                    LOG("clk timer %04x -> %016llx\n",
                            *(uint16_t *)(cpu->sp - 2), isr);
                    set_timer_isr(cpu, isr, *(uint16_t *)(cpu->sp - 2));
                    cpu->sp -= 2;
                    break;

                case IRQ_KBD:
                    LOG("kbd int %016llx\n", isr);
                    set_kbd_isr(cpu, isr);
                    break;

                case IRQ_TTY:
                    // we don't have an isr
                    cpu->sp += 8;

                    LOG("tty -> %02x\n", *(uint8_t *)(cpu->sp - 1));
                    write_tty(cpu->tty, *(uint8_t *)(cpu->sp - 1));
                    cpu->sp -= 1;
                    break;

                /*
                case IRQ_DISK_SET:
                    LOG("disk set %016llx.. ", t);
                    set_disk_position(cpu->disk, t);
                    LOG("%08lx\n", cpu->disk->pos);
                    break;

                case IRQ_DISK_RD:
                    LOG("disk read from %08lx: ", cpu->disk->pos);
                    u = read_disk(cpu->disk);
                    memcpy(cpu->sp - 4, &u, 4);
                    LOG("%016llx\n", u);
                    //handle_irq(cpu, &cpu->mem[t]);
                    break;

                case IRQ_DISK_WR:
                    LOG("disk write -> %016llx @ %08lx\n", t, cpu->disk->pos);
                    write_disk(cpu->disk, t);
                    break;

                case IRQ_P0_IN:
                case IRQ_P1_IN:
                case IRQ_P2_IN:
                case IRQ_P3_IN:
                    LOG("port%i @ %016llx\n", (irq - IRQ_P0_IN) / 8, t);
                    set_port_isr(cpu,      (irq - IRQ_P0_IN) / 8, t);
                    cpu->sp -= 4;
                    break;

                case IRQ_P0_OUT:
                case IRQ_P1_OUT:
                case IRQ_P2_OUT:
                case IRQ_P3_OUT:
                    //LOG("port%i <- %02x\n", (irq - IRQ_P0_OUT) / 8, t);
                    write_port(cpu->port0 + (irq - IRQ_P0_OUT) / 8, t);
                    cpu->sp -= 4;
                    break;
                */

                default:
                    LOG("not supported\n");
                    exit(1);
            }

            cpu->ip++;
            break;

        default:
            LOG("unhandled\n");
            exit(1);
    }
}

void
handle_irq(cpu_t *cpu, uint8_t *isr)
{
    uint8_t *ret = cpu->rp;
    uint64_t addr = cpu->ip - cpu->mem;
    memcpy(cpu->rp, &addr, 8);
    cpu->rp += 8;
    cpu->ip = isr;

    uint8_t *old_ip;
    while (cpu->rp != ret) {
        old_ip = cpu->ip;
        step_cpu(cpu);
    }

    //LOG("\n");
}

void
set_timer_isr(cpu_t *cpu, uint64_t isr, uint64_t t)
{
    // register run timer
    memset(&cpu->ke, 0, sizeof(struct kevent));
    EV_SET(&cpu->ke, IRQ_CLK, EVFILT_TIMER, EV_ADD | EV_CLEAR, 0, t, NULL);

    if (kevent(cpu->kq, &cpu->ke, 1, NULL, 0, NULL) == -1) {
        perror("setting timer");
        exit(1);
    }

    memcpy(&cpu->mem[IRQ_CLK], &isr, 4);
}

void
handle_timer(cpu_t *cpu)
{
    LOG("timer, jumping to %016llx\n", *(uint64_t *)(&cpu->mem[IRQ_CLK]));
    handle_irq(cpu, &cpu->mem[cpu->mem[IRQ_CLK]]);
}

void
set_kbd_isr(cpu_t *cpu, uint64_t isr)
{
    // watch for input
    memset(&cpu->ke, 0, sizeof(struct kevent));
    EV_SET(&cpu->ke, cpu->tty->master, EVFILT_READ, EV_ADD | EV_CLEAR,
            0, 0, NULL);

    if (kevent(cpu->kq, &cpu->ke, 1, NULL, 0, NULL) == -1) {
        perror("kbd read");
        exit(1);
    }

    memcpy(&cpu->mem[IRQ_KBD], &isr, 8);
}

void
handle_kbd(cpu_t *cpu)
{
    char c;
    if (read_tty(cpu->tty, &c)) {
        *(cpu->sp++) = c;
        LOG("tty <- %02x, jumping to %016llx\n",
                c, *(uint64_t *)(&cpu->mem[IRQ_KBD]));
        handle_irq(cpu, &cpu->mem[*(uint64_t *)(&cpu->mem[IRQ_KBD])]);
    }
}

void
set_port_isr(cpu_t *cpu, int portno, uint64_t isr)
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
        uint32_t ptr = *(uint32_t *)(&cpu->mem[IRQ_P0_IN + port->n * 8]);
        LOG("port%i -> %02x, jumping to %08x\n", port->n, c, ptr);
        handle_irq(cpu, &cpu->mem[ptr]);
    }
}

