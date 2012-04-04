#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "src/lib/cpu.h"

cpu_t *
init_cpu()
{
    cpu_t *cpu = calloc(1, sizeof(cpu_t));

    const char *fn = "data/vars.s";
    struct stat st;
    if (stat(fn, &st) < 0)
        return NULL;

    FILE *fp;
    if ((fp = fopen(fn, "r")) == NULL)
        return NULL;

    char *code = malloc(st.st_size);
    fread(code, st.st_size, 1, fp);
    fclose(fp);

    cpu_asm(code);
    load_prog(cpu, asmr.prog, asmr.prog_len);
    exit(0);

    cpu->ip = &cpu->mem[0];
    return cpu;
}

size_t
cpu_asm(const char *code)
{
    char *inp = strdup(code);
    char *line;
    asmr.prog = calloc(1024, sizeof(uint32_t));
    uint32_t *ip = &asmr.prog[0];
    size_t lineno = 0;

    asmr.labels_size = 1024;
    asmr.label_count = 0;
    asmr.labels = malloc(asmr.labels_size * sizeof(struct label *));

    asmr.missings_size = 1024;
    asmr.missing_count = 0;
    asmr.missing = malloc(asmr.labels_size * sizeof(struct label *));

    asmr.data_size = 1024;
    asmr.data_count = 0;
    asmr.data = malloc(asmr.data_size * sizeof(struct data *));

    while ((line = strsep(&inp, "\n")) != NULL) {
        lineno++;

        // skip whitespace
        while (*(line) == '\t')
            line++;

        // skip comments
        char *c;
        if ((c = strchr(line, ';')) != NULL) {
            c--;
            while (*c == '\t')
                c--;
            *(c + 1) = '\0';
        }

        char *op = strsep(&line, "\t");

        if (op[0] == '_') {
            // chomp
            op++;
            op[strlen(op) - 1] = '\0';

            asmr.labels[asmr.label_count] = malloc(sizeof(struct label));
            asmr.labels[asmr.label_count]->name = strdup(op);
            asmr.labels[asmr.label_count]->addr = ip - asmr.prog;
            asmr.label_count++;
        }

        if (strcmp(op, "HLT") == 0) {
            push(&ip, &line, HLT, 0);
        }

        if (strcmp(op, "LDI") == 0) {
            push(&ip, &line, LDI, 1);
        }

        if (strcmp(op, "LDX") == 0) {
            push(&ip, &line, LDX, 1);
        }

        if (strcmp(op, "LDP") == 0) {
            push(&ip, &line, LDP, 1);
        }

        if (strcmp(op, "STX") == 0) {
            push(&ip, &line, STX, 1);
        }

        if (strcmp(op, "SUB") == 0) {
            push(&ip, &line, SUB, 1);
        }

        if (strcmp(op, "JMP") == 0) {
            push(&ip, &line, JMP, 1);
        }

        if (strcmp(op, "JZ") == 0) {
            push(&ip, &line, JZ, 1);
        }

        if (strcmp(op, "JNZ") == 0) {
            push(&ip, &line, JNZ, 1);
        }

        if (strcmp(op, "ADD") == 0) {
            push(&ip, &line, ADD, 1);
        }

        if (strcmp(op, "OUT") == 0) {
            push(&ip, &line, OUT, 0);
        }

        if (strcmp(op, "INC") == 0) {
            push(&ip, &line, LDX, 1);
            push(&ip, NULL, ADD, 0);
            push(&ip, NULL, 1, 0);
            push(&ip, NULL, STX, 0);
            push(&ip, NULL, *(ip - 4), 0);
        }

        if (strcmp(op, "DEC") == 0) {
            push(&ip, &line, LDX, 1);
            push(&ip, NULL, SUB, 0);
            push(&ip, NULL, 1, 0);
            push(&ip, NULL, STX, 0);
            push(&ip, NULL, *(ip - 4), 0);
        }

        if (strcmp(op, "DATA") == 0) {
            char *name = strsep(&line, "\t");

            int i;
            for (i = 0; i < asmr.data_count; i++)
                if (asmr.data[i] == NULL)
                    break;

            asmr.data[i] = malloc(sizeof(struct data));
            asmr.data[i]->name = strdup(name);

            char *data;

            while ((data = strsep(&line, ",")) != NULL) {
                if (data[0] == '"') {
                    // string
                    data++; data[strlen(data) - 1] = '\0';
                    size_t data_len = strlen(data);

                    /* for (int i = 0; i < data_len; i++) { */
                    /*     push(&ip, NULL, LDI, 0); */
                    /*     push(&ip, NULL, data[i], 0); */
                    /*     push(&ip, NULL, STX, 0); */
                    /*     push(&ip, NULL, addr++, 0); */
                    /* } */

                } else {

                    // hex value
                    /* uint32_t byte; */
                    /* sscanf(data, "%x", &byte); */
                    /* push(&ip, NULL, LDI, 0); */
                    /* push(&ip, NULL, byte, 0); */
                    /* push(&ip, NULL, STX, 0); */
                    /* push(&ip, NULL, addr++, 0); */

                }
            }
        }
    }

    for (int i = 0; i < CPU_MEMORY_SZ; i++) {
        if (asmr.prog[i] == 0xdeadbeef) {
            for (int j = 0; j < asmr.missing_count; j++) {
                if (asmr.missing[j]->addr == i) {
                    for (int n = 0; n < asmr.label_count; n++) {
                        if (strcmp(asmr.labels[n]->name,
                                    asmr.missing[j]->name) == 0) {
                            asmr.prog[i] = asmr.labels[n]->addr;
                        }
                    }
                }
            }
        }
    }

    uint32_t *p = asmr.prog;
    size_t len = ip - asmr.prog;
    for (int i = 0; i < len; i += 16) {
        printf("%02x: ", i);
        for (int j = 0; j < 16; j++) {
            if (j % 4 == 0)
                printf(" ");
            printf("%02x ", *(p++));
        }
        printf("\n");
    }
    printf("\n");

    free(asmr.labels);
    free(asmr.missing);
    free(inp);

    asmr.prog_len = ip - asmr.prog;
    return ip - asmr.prog;
}

uint32_t
read_value(char *s, uint32_t *ip)
{
    uint32_t arg = 0;

    if (s[0] == '0' && s[1] == 'x') {
        // memory address
        sscanf(s, "%x", &arg);
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
        for (i = 0; i < asmr.label_count; i++) {
            if (strcmp(asmr.labels[i]->name, s) == 0) {
                label = asmr.labels[i];
                break;
            }
        }

        if (label) {
            arg = asmr.labels[i]->addr;
        } else {
            asmr.missing[asmr.missing_count] =
                malloc(sizeof(struct label));
            asmr.missing[asmr.missing_count]->addr = ip - asmr.prog;
            asmr.missing[asmr.missing_count]->name = strdup(s);
            asmr.missing_count++;
            arg = 0xdeadbeef;
        }
    }

    return arg;
}

void
push(uint32_t **ip, char **line, uint32_t op, size_t n)
{
    *(*ip)++ = op;
    for (int i = 0; i < n; i++) {
        char *s  = strsep(line, "\t");
        uint32_t arg = read_value(s, *ip);
        *(*ip)++ = arg;
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

    while ((op = *(cpu->ip++)) != 0) {
        /* printf("%li: %x %x\n", cpu->ip - cpu->mem, op, *cpu->ip); */
        switch (op) {
            case HLT:
                /* printf("HLT\n"); */
                return;

            case LDI:
                /* printf("LDI\n"); */
                cpu->ax = *(cpu->ip++);
                break;

            case LDX:
                /* printf("LDX\n"); */
                cpu->ax = cpu->mem[*(cpu->ip++)];
                break;

            case LDP:
                /* printf("LDP\n"); */
                cpu->ax = cpu->mem[cpu->mem[*(cpu->ip++)]];
                break;

            case STX:
                /* printf("STX\n"); */
                cpu->mem[*(cpu->ip++)] = cpu->ax;
                break;

            case OUT:
                /* printf("OUT\n"); */
                printf("%c", cpu->ax);
                //printf("%08x\n", cpu->ax);
                break;

            case JMP:
                /* printf("JMP\n"); */
                cpu->ip = &cpu->mem[*(cpu->ip++)];
                break;

            case JZ:
                /* printf("JZ\n"); */
                addr = *(cpu->ip++);
                if (cpu->ax == 0)
                    cpu->ip = &cpu->mem[addr];
                break;

            case JNZ:
                /* printf("JNZ\n"); */
                addr = *(cpu->ip++);
                if (cpu->ax != 0)
                    cpu->ip = &cpu->mem[addr];
                break;

            case SUB:
                /* printf("SUB\n"); */
                cpu->ax -= *(cpu->ip++);
                break;

            case ADD:
                /* printf("ADD\n"); */
                cpu->ax += *(cpu->ip++);
                break;

            default:
                /* printf("DATA\n"); */
                break;
        }
    }
}

