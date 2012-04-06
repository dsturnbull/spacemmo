#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "src/lib/cpu.h"

cpu_t *
init_cpu()
{
    cpu_t *cpu = calloc(1, sizeof(cpu_t));

    const char *fn = "data/data.s";
    struct stat st;
    if (stat(fn, &st) < 0)
        return NULL;

    FILE *fp;
    if ((fp = fopen(fn, "r")) == NULL)
        return NULL;

    char *code = malloc(st.st_size);
    fread(code, st.st_size, 1, fp);
    fclose(fp);

    asmr_t *asmr = init_asmr();
    cpu_asm(asmr, code);
    load_prog(cpu, asmr->prog, asmr->prog_len);

    cpu->ip = &cpu->mem[0];
    return cpu;
}

asmr_t *
init_asmr()
{
    asmr_t *asmr = malloc(sizeof(asmr_t));
    asmr->labels_size = 1024;
    asmr->label_count = 0;
    asmr->labels = malloc(asmr->labels_size * sizeof(struct label *));

    asmr->missings_size = 1024;
    asmr->missing_count = 0;
    asmr->missing = malloc(asmr->labels_size * sizeof(struct label *));

    asmr->data_size = 1024;
    asmr->data_count = 0;
    asmr->data = malloc(asmr->data_size * sizeof(struct data *));

    asmr->prog = calloc(1024, sizeof(uint32_t));
    asmr->ip = asmr->prog;

    return asmr;
}

size_t
cpu_asm(asmr_t *asmr, const char *code)
{
    char *inp = strdup(code);
    char *line;
    size_t lineno = 0;

    while ((line = strsep(&inp, "\n")) != NULL) {
        lineno++;
        normalise_line(&line);

        char *ops = strsep(&line, "\t");

        if (ops[0] == '_') {
            make_label(asmr, ops);
            continue;
        }

        if (ops[0] == ';')
            continue;

        if (strlen(ops) == 0)
            continue;

        op_t op = parse_op(ops);

        char *data;
        uint32_t addr;

        switch (op) {
            case NOP:
            case OUT:
            case HLT:
                push(asmr, NULL, op, 0);
                break;

            case LDI:
            case LD0:
            case LDP:
            case ST0:
            case SUB:
            case JMP:
            case JZ:
            case JNZ:
            case ADD:
            case DIV:
            case MUL:
                push(asmr, &line, op, 1);
                break;

            case INC:
                push(asmr, &line, LD0, 1);
                push(asmr, NULL, ADD, 0);
                push(asmr, NULL, 1, 0);
                push(asmr, NULL, ST0, 0);
                push(asmr, NULL, *(asmr->ip - 4), 0);
                break;

            case DEC:
                push(asmr, &line, LD0, 1);
                push(asmr, NULL, SUB, 0);
                push(asmr, NULL, 1, 0);
                push(asmr, NULL, ST0, 0);
                push(asmr, NULL, *(asmr->ip - 4), 0);
                break;

            case DATA:
                sscanf(strsep(&line, "\t"), "%x", &addr);

                while ((data = strsep(&line, ",")) != NULL) {
                    if (data[0] == '"') {
                        // string
                        data++; data[strlen(data) - 1] = '\0';
                        size_t data_len = strlen(data);

                        for (int i = 0; i < data_len; i++) {
                            push(asmr, NULL, LDI, 0);
                            push(asmr, NULL, data[i], 0);
                            push(asmr, NULL, ST0, 0);
                            push(asmr, NULL, addr++, 0);
                        }

                    } else {

                        // hex value
                        uint32_t byte;
                        sscanf(data, "%x", &byte);
                        push(asmr, NULL, LDI, 0);
                        push(asmr, NULL, byte, 0);
                        push(asmr, NULL, ST0, 0);
                        push(asmr, NULL, addr++, 0);

                    }
                }
                break;

            }
    }

    for (int i = 0; i < CPU_MEMORY_SZ; i++)
        if (asmr->prog[i] == 0xdeadbeef)
            for (int j = 0; j < asmr->missing_count; j++)
                if (asmr->missing[j]->addr == i)
                    for (int n = 0; n < asmr->label_count; n++)
                        if (strcmp(asmr->labels[n]->name,
                                    asmr->missing[j]->name) == 0)
                            asmr->prog[i] = asmr->labels[n]->addr;

    uint32_t *p = asmr->prog;
    asmr->prog_len = asmr->ip - asmr->prog;
    for (int i = 0; i < asmr->prog_len; i += 16) {
        printf("%08x: ", i);
        for (int j = 0; j < 16; j++) {
            if (j % 4 == 0)
                printf(" ");
            printf("%08x ", *(p++));
        }
        printf("\n");
    }
    printf("\n");

    free(asmr->labels);
    free(asmr->missing);
    free(inp);

    return asmr->prog_len;
}

void
make_label(asmr_t *asmr, char *s)
{
    // chomp
    s++; s[strlen(s) - 1] = '\0';

    asmr->labels[asmr->label_count] = malloc(sizeof(struct label));
    asmr->labels[asmr->label_count]->name = strdup(s);
    asmr->labels[asmr->label_count]->addr = asmr->ip - asmr->prog;
    asmr->label_count++;
}

op_t
parse_op(char *op)
{
    CHECK_OP(NOP);
    CHECK_OP(HLT);
    CHECK_OP(LD0);
    CHECK_OP(LDI);
    CHECK_OP(LDP);
    CHECK_OP(ST0);
    CHECK_OP(OUT);
    CHECK_OP(JMP);
    CHECK_OP(JZ);
    CHECK_OP(JNZ);
    CHECK_OP(SUB);
    CHECK_OP(ADD);
    CHECK_OP(DIV);
    CHECK_OP(MUL);
    CHECK_OP(INC);
    CHECK_OP(DEC);
    CHECK_OP(DATA);
    return NOP;
}

void
normalise_line(char **line)
{
    // skip whitespace
    while (*(*line) == '\t')
        (*line)++;

    // skip comments
    char *c;
    if ((c = strchr(*line, ';')) != NULL) {
        c--;
        while (*c == '\t')
            c--;
        *(c + 1) = '\0';
    }
}

uint32_t
read_value(asmr_t *asmr, char *s, uint32_t *ip)
{
    uint32_t arg = 0;

    if (s[0] == '0' && s[1] == 'x') {
        // memory address
        sscanf(s, "%x", &arg);

    } else if (s[0] == 'C' && s[1] == 'P' && s[2] == 'U') {
        // const
        if (strcmp(s, "CPU_CLOCK") == 0) {
            arg = CPU_CLOCK;
        }

    } else if (s[0] == 'r' && s[1] >= 0x30 && s[1] <= 0x37) {
        // register
        arg = R0 + (s[1] - 0x30) * R0;

    } else {
        // label
        bool variable = true;
        struct label *label = NULL;
        if (s[0] == '_') {
            s++;
            s[strlen(s)] = '\0';
            variable = false;
        }

        int i;
        for (i = 0; i < asmr->label_count; i++) {
            if (strcmp(asmr->labels[i]->name, s) == 0) {
                label = asmr->labels[i];
                break;
            }
        }

        if (label) {
            arg = asmr->labels[i]->addr;
        } else {
            asmr->missing[asmr->missing_count] =
                malloc(sizeof(struct label));
            asmr->missing[asmr->missing_count]->addr = ip - asmr->prog;
            asmr->missing[asmr->missing_count]->name = strdup(s);
            asmr->missing_count++;
            arg = 0xdeadbeef;
        }
    }

    return arg;
}

void
push(asmr_t *asmr, char **line, uint32_t op, size_t n)
{
    *(asmr->ip)++ = op;
    for (int i = 0; i < n; i++) {
        char *s  = strsep(line, "\t");
        uint32_t arg = read_value(asmr, s, asmr->ip);
        *(asmr->ip)++ = arg;
    }
}

void
load_prog(cpu_t *cpu, uint32_t *prog, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        cpu->mem[i] = prog[i];
    }
}

void
run_prog(cpu_t *cpu)
{
    uint32_t op;
    uint32_t addr;
    uint32_t d, r;

    while ((op = *(cpu->ip++)) != 0) {
        gettimeofday(&cpu->time, NULL);
        cpu->mem[CPU_CLOCK] = cpu->time.tv_sec;
        cpu->mem[R0] = cpu->r0;
        cpu->mem[R1] = cpu->r1;

        /* printf("%04lx: %04x %04x\n", cpu->ip - cpu->mem, op, *cpu->ip); */
        switch (op) {
            case HLT:
                /* printf("HLT\n"); */
                return;

            case LDI:
                /* printf("LDI\n"); */
                cpu->r0 = *(cpu->ip++);
                break;

            case LD0:
                /* printf("LD0\n"); */
                cpu->r0 = cpu->mem[*(cpu->ip++)];
                break;

            case LDP:
                /* printf("LDP\n"); */
                cpu->r0 = cpu->mem[cpu->mem[*(cpu->ip++)]];
                break;

            case ST0:
                /* printf("ST0\n"); */
                cpu->mem[*(cpu->ip++)] = cpu->r0;
                break;

            case OUT:
                /* printf("OUT\n"); */
                printf("%c", cpu->r0);
                /* printf("%08x\n", cpu->r0); */
                break;

            case JMP:
                /* printf("JMP\n"); */
                cpu->ip = &cpu->mem[*(cpu->ip++)];
                break;

            case JZ:
                /* printf("JZ\n"); */
                addr = *(cpu->ip++);
                if (cpu->r0 == 0)
                    cpu->ip = &cpu->mem[addr];
                break;

            case JNZ:
                /* printf("JNZ\n"); */
                addr = *(cpu->ip++);
                if (cpu->r0 != 0)
                    cpu->ip = &cpu->mem[addr];
                break;

            case SUB:
                /* printf("SUB\n"); */
                cpu->r0 -= *(cpu->ip++);
                break;

            case ADD:
                /* printf("ADD\n"); */
                cpu->r0 += *(cpu->ip++);
                break;

            case DIV:
                /* printf("DIV\n"); */
                d = *(cpu->ip++);
                r = cpu->r0 % d;
                cpu->r0 = cpu->r0 / d;
                cpu->r1 = r;
                break;

            default:
                /* printf("DATA\n"); */
                break;
        }
    }
}

