#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/lib/spacemmo.h"
#include "src/lib/cpu/cpu.h"
#include "src/lib/cpu/hardware/tty.h"
#include "src/lib/cpu/hardware/mmu.h"
#include "src/lib/cpu/hardware/port.h"

uint8_t *prev;

cpu_t *
init_cpu()
{
    cpu_t *cpu = calloc(1, sizeof(*cpu));

    cpu->ip = cpu->mem;
    cpu->sp = &cpu->mem[STACK];
    cpu->rp = &cpu->mem[RET_STACK];

    // init kqueue
    if ((cpu->kq = kqueue()) == -1) {
        perror("kqueue");
        exit(1);
    }

    cpu->log = stdout;
    cpu->tty = init_tty();
    cpu->mmu = init_mmu();
    cpu->port0 = init_port(0, &cpu->mem[IRQ_P0_BUF], 0x100);
    cpu->port1 = init_port(1, &cpu->mem[IRQ_P1_BUF], 0x100);
    cpu->port2 = init_port(2, &cpu->mem[IRQ_P2_BUF], 0x100);
    cpu->port3 = init_port(3, &cpu->mem[IRQ_P3_BUF], 0x100);

    cpu->opmap0 = malloc(32 * sizeof(uintptr_t));
    cpu->opmap1 = malloc(32 * sizeof(uintptr_t));
    cpu->opmap2 = malloc(32 * sizeof(uintptr_t));

    cpu->opmap0[NOP]     = &handle_nop;
    cpu->opmap0[HLT]     = &handle_hlt;
    cpu->opmap1[CALL]    = &handle_call;
    cpu->opmap0[RET]     = &handle_ret;
    cpu->opmap0[LOAD]    = &handle_load;
    cpu->opmap1[STORE]   = &handle_store;
    cpu->opmap2[ADD]     = &handle_add;
    cpu->opmap2[SUB]     = &handle_sub;
    cpu->opmap2[MUL]     = &handle_mul;
    cpu->opmap2[DIV]     = &handle_div;
    cpu->opmap2[AND]     = &handle_and;
    cpu->opmap2[OR]      = &handle_or;
    cpu->opmap1[JMP]     = &handle_jmp;
    cpu->opmap1[JE]      = &handle_je;
    cpu->opmap1[JNE]     = &handle_jne;
    cpu->opmap1[JZ]      = &handle_jz;
    cpu->opmap1[JNZ]     = &handle_jnz;
    cpu->opmap0[PUSH]    = &handle_push;
    cpu->opmap1[DUP]     = &handle_dup;
    cpu->opmap1[POP]     = &handle_pop;
    cpu->opmap0[SWAP]    = &handle_swap;
    cpu->opmap0[INT]     = &handle_int;

    cpu->ts = calloc(1, sizeof(struct timespec));

    //wait_tty_slave(cpu->tty);

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
}

void
run_cpu(cpu_t *cpu)
{
    if (cpu->ts == NULL)
        handle_io(cpu);
    else
        step_cpu(cpu);
}

void
step_cpu(cpu_t *cpu)
{
    opcode_t opcode = *(opcode_t *)cpu->ip;
    prev = cpu->ip;

    LOG("%016lx cy: %08lx op:%02x ip: %016lx rp: %016lx sp: %016lx ",
            cpu->ip - cpu->mem, cpu->cycles, opcode.op,
            cpu->ip - cpu->mem,
            cpu->rp - &cpu->mem[RET_STACK],
            cpu->sp - &cpu->mem[STACK]);

    handle_op(cpu, &opcode);
    cpu->cycles++;
}

void
reset_cpu(cpu_t *cpu)
{
}

void
handle_io(cpu_t *cpu)
{
    memset(&cpu->ke, 0, sizeof(cpu->ke));
    if (kevent(cpu->kq, NULL, 0, &cpu->ke, 1, cpu->ts) > 0) {
        if (cpu->mem[IRQ_CLK] && cpu->ke.ident == IRQ_CLK)
            handle_timer(cpu);

        if (cpu->mem[IRQ_KBD] &&
                cpu->ke.ident == (uintptr_t)cpu->tty->master)
            handle_kbd(cpu);
    }
}

void
handle_op(cpu_t *cpu, opcode_t *opcode)
{
    LOG("(%02x ", *(uint8_t *)opcode);
    for (int i = 0; i < 8; i++)
        LOG("%i", (*(uint8_t *)opcode & (1 << i)) != 0);
    LOG(") ");

    uint64_t arg = 0;
    uint64_t a = 0, b = 0;

    instruction_t instruction;
    memcpy(&instruction, opcode, sizeof(*opcode));
    instruction.len = 1 << opcode->flags;

    LOG("len: %lu ", instruction.len);

    void (*op0)(cpu_t *, instruction_t *);
    void (*op1)(cpu_t *, instruction_t *, uint64_t);
    void (*op2)(cpu_t *, instruction_t *, uint64_t, uint64_t);

    if ((op0 = cpu->opmap0[opcode->op])) {
        op0(cpu, &instruction);
    } else if ((op1 = cpu->opmap1[opcode->op])) {
        memcpy(&arg, cpu->sp - instruction.len, instruction.len);
        cpu->sp -= instruction.len;
        op1(cpu, &instruction, arg);
    } else if ((op2 = cpu->opmap2[opcode->op])) {
        memcpy(&a, cpu->sp - instruction.len, instruction.len);
        cpu->sp -= instruction.len;
        memcpy(&b, cpu->sp - instruction.len, instruction.len);
        cpu->sp -= instruction.len;
        op2(cpu, &instruction, a, b);
    }

    if (!(op0 || op1 || op2))
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
    if (cpu->ts)
        free(cpu->ts);
    cpu->ts = NULL;
    LOG("halt\n");
}

void
handle_call(cpu_t *cpu, instruction_t *op, uint64_t arg)
{
    LOG("call %016lx -> %016llx\n", cpu->ip - cpu->mem, arg);
    uint64_t ret = cpu->ip - cpu->mem + 1;
    cpu->ip = &cpu->mem[arg];
    memcpy(cpu->rp, &ret, sizeof(ret));
    cpu->rp += 8;
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
    cpu->sp -= 8;
    uint64_t loc = *(uint64_t *)(cpu->sp);
    uint64_t val = 0;
    memcpy(&val, &cpu->mem[loc], op->len);
    memcpy(cpu->sp, &val, op->len);
    cpu->sp += op->len;

    LOG("load %016llx <- %016llx\n", val, loc);
    cpu->ip++;
}

void
handle_store(cpu_t *cpu, instruction_t *op, uint64_t arg)
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
handle_add(cpu_t *cpu, instruction_t *op, uint64_t a, uint64_t b)
{
    uint64_t r = b + a;
    LOG("add  %016llx + %016llx == %016llx\n", a, b, r);
    memcpy(cpu->sp, &r, op->len);
    cpu->sp += op->len;
    cpu->ip++;
}

void
handle_sub(cpu_t *cpu, instruction_t *op, uint64_t a, uint64_t b)
{
    uint64_t r = b - a;
    LOG("sub  %016llx - %016llx == %016llx\n", a, b, r);
    memcpy(cpu->sp, &r, op->len);
    cpu->sp += op->len;
    cpu->ip++;
}

void
handle_mul(cpu_t *cpu, instruction_t *op, uint64_t a, uint64_t b)
{
    uint64_t r = b * a;
    LOG("mul  %016llx * %016llx == %016llx\n", a, b, r);
    memcpy(cpu->sp, &r, op->len);
    cpu->sp += op->len;
    cpu->ip++;
}

void
handle_div(cpu_t *cpu, instruction_t *op, uint64_t a, uint64_t b)
{
    uint64_t q = b / a;
    uint64_t r = b % a;
    LOG("div  %016llx / %016llx == %016llx r %016llx\n", a, b, q, r);
    memcpy(cpu->sp, &q, op->len);
    cpu->sp += op->len;
    memcpy(cpu->sp, &r, op->len);
    cpu->sp += op->len;
    cpu->ip++;
}

void
handle_and(cpu_t *cpu, instruction_t *op, uint64_t a, uint64_t b)
{
    uint64_t r = b & a;
    LOG("and  %016llx & %016llx == %016llx\n", a, b, r);
    memcpy(cpu->sp, &r, op->len);
    cpu->sp += op->len;
    cpu->ip++;
}

void
handle_or(cpu_t *cpu, instruction_t *op, uint64_t a, uint64_t b)
{
    uint64_t r = b | a;
    LOG("or   %016llx | %016llx == %016llx\n", a, b, r);
    memcpy(cpu->sp, &r, op->len);
    cpu->sp += op->len;
    cpu->ip++;
}

void
handle_jmp(cpu_t *cpu, instruction_t *op, uint64_t loc)
{
    LOG("jmp  %016llx\n", loc);
    cpu->ip = &cpu->mem[loc];
}

void
handle_je(cpu_t *cpu, instruction_t *op, uint64_t loc)
{
    uint64_t a = 0, b = 0;

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
handle_jne(cpu_t *cpu, instruction_t *op, uint64_t loc)
{
    uint64_t a = 0, b = 0;

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
handle_jz(cpu_t *cpu, instruction_t *op, uint64_t loc)
{
    uint64_t val = 0;

    cpu->sp -= op->len;
    memcpy(&val, cpu->sp, op->len);

    if (val == 0) {
        LOG("jz   %016llx == 0 -> %016llx\n", val, loc);
        cpu->ip = &cpu->mem[loc];
    } else {
        LOG("jz   %016llx != 0\n", val);
        cpu->ip++;
    }
}

void
handle_jnz(cpu_t *cpu, instruction_t *op, uint64_t loc)
{
    uint64_t val = 0;

    cpu->sp -= op->len;
    memcpy(&val, cpu->sp, op->len);

    if (val != 0) {
        LOG("jz   %016llx != 0 -> %016llx\n", val, loc);
        cpu->ip = &cpu->mem[loc];
    } else {
        LOG("jz   %016llx == 0\n", val);
        cpu->ip++;
    }
}

void
handle_dup(cpu_t *cpu, instruction_t *op, uint64_t arg)
{
    LOG("dup  %016llx\n", arg);
    memcpy(cpu->sp, &arg, op->len);
    cpu->sp += op->len * 2;
    cpu->ip++;
}

void
handle_push(cpu_t *cpu, instruction_t *op)
{
    uint64_t val = 0;
    memcpy(&val, cpu->ip + 1, op->len);
    LOG("push %016llx\n", val);
    memcpy(cpu->sp, cpu->ip + 1, op->len);
    cpu->sp += op->len;
    cpu->ip += op->len;
    cpu->ip++;
}

void
handle_pop(cpu_t *cpu, instruction_t *op, uint64_t arg)
{
    LOG("pop  %016llx\n", arg);
    cpu->ip++;
}

void
handle_swap(cpu_t *cpu, instruction_t *op)
{
    uint64_t a = 0, b = 0;

    cpu->sp -= op->len;
    memcpy(&a, cpu->sp, op->len);

    cpu->sp -= 8;
    memcpy(&b, cpu->sp, 8);

    LOG("swap %016llx <> %016llx (top: %08lx)\n", a, b, op->len);

    memset(cpu->sp, 0, op->len);
    memcpy(cpu->sp, &a, op->len);
    cpu->sp += op->len;

    memset(cpu->sp, 0, 8);
    memcpy(cpu->sp, &b, 8);
    cpu->sp += 8;

    cpu->ip++;
}

void
handle_int(cpu_t *cpu, instruction_t *op)
{
    uint64_t isr = 0;
    uint8_t c = 0;
    uint16_t dt = 0;
    port_t *port = NULL;
    size_t len = 0;
    uint8_t *data = NULL;

    cpu->sp -= 8;
    irq_t irq = *((uint64_t *)(cpu->sp));

    switch (irq) {
        case IRQ_CLK:
        case IRQ_KBD:
            cpu->sp -= 8;
            isr = *((uint64_t *)(cpu->sp));
            break;

        default:
            break;
    }

    switch (irq) {
        case IRQ_CLK:
            cpu->sp -= 2;
            dt = *((uint16_t *)(cpu->sp));
            LOG("clk  %016llx (%ims)\n", isr, dt);
            set_timer_isr(cpu, isr, dt);
            break;

        case IRQ_KBD:
            LOG("kbd  %016llx\n", isr);
            set_kbd_isr(cpu, isr);
            break;

        case IRQ_TTY:
            cpu->sp--;
            c = *(uint8_t *)(cpu->sp);
            LOG("tty  %02x\n", c);
            write_tty(cpu->tty, c);
            break;

        case IRQ_MMU:
            LOG("mmu\n");
            mmu_int(cpu->mmu, &cpu->sp);
            break;

        case IRQ_P0:
        case IRQ_P1:
        case IRQ_P2:
        case IRQ_P3:
            port = find_port(cpu, irq);
            len = *(uint8_t *)(cpu->sp - 1);
            cpu->sp -= 1;
            data = malloc(len);
            memcpy(data, cpu->sp - len, len);
            cpu->sp -= len;
            LOG("prt%i <- %02lx bytes\n", port->n, len);
            write_port(port, data, len);
            handle_port_read(cpu, port);
            //set_port_isr(cpu, port, isr);
            break;
    }

    cpu->ip++;
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

    memcpy(&cpu->mem[IRQ_CLK], &isr, 8);
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

port_t *
find_port(cpu_t *cpu, irq_t irq)
{
    return cpu->port0 + (irq - IRQ_P0) / 8;
}

void
handle_port_read(cpu_t *cpu, port_t *port)
{
    uint8_t buf[0x100];
    size_t len;
    uint64_t ptr = &cpu->mem[IRQ_P0 + port->n * 8] - cpu->mem;
    uint8_t *out = &cpu->mem[IRQ_P0 + port->n * 8 + 8];

    len = read_port(port, buf, 0x100);
    memcpy(out, &buf, len);
    out += len;
    LOG("prt%i wrote %08lx bytes -> %016llx\n", port->n, len, ptr);
    //handle_irq(cpu, &cpu->mem[ptr]);
}

