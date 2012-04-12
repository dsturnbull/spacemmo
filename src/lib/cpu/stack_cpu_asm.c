#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/stat.h>

#include "src/lib/cpu/stack_cpu_asm.h"

stack_cpu_asm_t *
init_stack_cpu_asm()
{
    stack_cpu_asm_t *as = malloc(sizeof(stack_cpu_asm_t));
    as->labels_size = 1024;
    as->label_count = 0;
    as->labels = malloc(as->labels_size *
            sizeof(struct label *));

    as->missings_size = 1024;
    as->missing_count = 0;
    as->missing = malloc(as->labels_size *
            sizeof(struct label *));

    as->data_size = 1024;
    as->data_count = 0;
    as->data = malloc(as->data_size *
            sizeof(struct data *));

    as->prog = calloc(CPU_DATA - 1, sizeof(uint32_t));
    as->ip = as->prog;

    return as;
}

size_t
stack_cpu_asm(stack_cpu_asm_t *as, const char *fn)
{
    // make room for jumping to _main
    //as->ip += 4;

    // assemble
    parse_file(as, fn);
    as->prog_len = as->ip - as->prog;

    // replace label locations etc
    replace_sentinels(as);

    // jump to main
    /*
    uint32_t main_loc;

    size_t i;
    for (i = 0; i < as->label_count; i++) {
        if (strcmp(as->labels[i]->name, "main") == 0) {
            main_loc = as->labels[i]->addr;
            break;
        }
    }

    as->ip = as->prog;
    push(as, NULL, PUSH, 0);
    push(as, NULL, main_loc, 0);
    push(as, NULL, CALL, 0);
    push(as, NULL, HLT, 0);
    */

    free(as->labels);
    free(as->missing);
    free(as->data);

    return as->prog_len;
}

void
parse_file(stack_cpu_asm_t *as, const char *fn)
{
    // load asm file
    struct stat st;
    if (stat(fn, &st) < 0) {
        perror(fn);
        return;
    }

    FILE *fp;
    if ((fp = fopen(fn, "r")) == NULL) {
        perror(fn);
        return;
    }

    char *inp = malloc(st.st_size);
    fread(inp, st.st_size, 1, fp);
    fclose(fp);

    char *line;
    size_t lineno = 0;

    define_constant(as, "KBD", IRQ_KBD);
    define_constant(as, "OUT", IRQ_TTY);
    define_constant(as, "CODE_PAGE", CPU_CODE);
    define_constant(as, "DATA_PAGE", CPU_DATA);
    define_constant(as, "STACK_PAGE", CPU_STACK);
    define_constant(as, "FRAMES_PAGE", CPU_FRAMES);
    define_constant(as, "IO_PAGE", CPU_IO);

    while ((line = strsep(&inp, "\n")) != NULL && inp != NULL) {
        lineno++;
        normalise_line(&line);

        char *ops = strsep(&line, "\t");

        if (ops[0] == '_') {
            make_label(as, ops);
            continue;
        }

        if (ops[0] == ';')
            continue;

        if (strcmp(ops, "%define") == 0) {
            char *name = strsep(&line, "\t");;
            uint32_t value;
            sscanf(strsep(&line, "\t"), "%x", &value);
            define_constant(as, name, value);
            continue;
        }

        if (strcmp(ops, "%import") == 0) {
            char *fn = strsep(&line, "\t");
            fn++; fn[strlen(fn) - 1] = '\0';
            parse_file(as, fn);
            continue;
        }

        if (strlen(ops) == 0)
            continue;

        op_t op = parse_op(ops);

        uint64_t value;
        ins_t ins;

        switch (op) {
            case PUSH:
                value = read_value(as, strsep(&line, "\t"), as->ip);

                if (value <= 0xff) {
                    ins.op = op;
                    ins.opt = 0;
                    memcpy(as->ip++, &ins, sizeof(ins_t));
                    memcpy(as->ip, &value, sizeof(uint8_t));
                    as->ip += sizeof(uint8_t);
                } else if (value <= 0xffff) {
                    ins.op = op;
                    ins.opt = 1;
                    memcpy(as->ip++, &ins, sizeof(ins_t));
                    memcpy(as->ip, &value, sizeof(uint16_t));
                    as->ip += sizeof(uint16_t);
                } else if (value <= 0xffffffff) {
                    ins.op = op;
                    ins.opt = 2;
                    memcpy(as->ip++, &ins, sizeof(ins_t));
                    memcpy(as->ip, &value, sizeof(uint32_t));
                    as->ip += sizeof(uint32_t);
                } else if (value <= 0xffffffffffffffff) {
                    ins.op = op;
                    ins.opt = 3;
                    memcpy(as->ip++, &ins, sizeof(ins_t));
                    memcpy(as->ip, &value, sizeof(uint64_t));
                    as->ip += sizeof(uint64_t);
                }
                break;

            default:
                ins.op = op;
                ins.opt = 0;
                memcpy(as->ip++, &ins, sizeof(ins_t));
                break;
            }
    }

    free(inp);
}

uint64_t
read_value(stack_cpu_asm_t *as, char *s, uint8_t *ip)
{
    uint64_t arg = 0;

    if (s[0] == '0' && s[1] == 'x') {
        // memory address
        sscanf(s, "%llx", &arg);

    } else if (s[0] == 'C' && s[1] == 'P' && s[2] == 'U') {
        // const
        if (strcmp(s, "STACK_CPU_CLOCK") == 0) {
        }

    } else if (s[0] == '[') {
        // pointer
        s++;
        s[strlen(s)] = '\0';
        sscanf(s, "%llx", &arg);

    } else if (s[0] >= 'A' && s[0] <= 'Z') {
        // constant
        size_t i;
        for (i = 0; i < as->label_count; i++) {
            if (strcmp(as->labels[i]->name, s) == 0) {
                arg = as->labels[i]->addr;
                break;
            }
        }

    } else {
        // label
        bool variable = true;
        struct label *label = NULL;
        if (s[0] == '_') {
            s++;
            s[strlen(s)] = '\0';
            variable = false;
        }

        size_t i;
        for (i = 0; i < as->label_count; i++) {
            if (strcmp(as->labels[i]->name, s) == 0) {
                label = as->labels[i];
                break;
            }
        }

        if (label) {
            arg = as->labels[i]->addr;
        } else {
            as->missing[as->missing_count] =
                malloc(sizeof(struct label));
            as->missing[as->missing_count]->addr = ip - as->prog;
            as->missing[as->missing_count]->name = strdup(s);
            as->missing_count++;
            arg = 0xdeadbeef;
        }
    }

    return arg;
}

void
define_constant(stack_cpu_asm_t *as, char *name, uint32_t value)
{
    as->labels[as->label_count] = malloc(sizeof(struct label));
    as->labels[as->label_count]->name = strdup(name);
    as->labels[as->label_count]->addr = value;
    as->label_count++;
}

void
make_label(stack_cpu_asm_t *as, char *s)
{
    // chomp
    s++; s[strlen(s) - 1] = '\0';

    as->labels[as->label_count] = malloc(sizeof(struct label));
    as->labels[as->label_count]->name = strdup(s);
    as->labels[as->label_count]->addr = as->ip - as->prog;
    /* printf("%s == %x\n", s, */
    /*         as->labels[as->label_count]->addr); */
    as->label_count++;
}

op_t
parse_op(char *op)
{
    GET_CONST(HLT);
    GET_CONST(LOAD);
    GET_CONST(STORE);
    GET_CONST(ADD);
    GET_CONST(SUB);
    GET_CONST(MUL);
    GET_CONST(DIV);
    GET_CONST(JMP);
    GET_CONST(JZ);
    GET_CONST(JNZ);
    GET_CONST(CALL);
    GET_CONST(RET);
    GET_CONST(DUP);
    GET_CONST(POP);
    GET_CONST(SWAP);
    GET_CONST(INT);
    GET_CONST(DEBUG);
    GET_CONST(PUSH);

    printf("%s\n", op);
    assert(false);
}

void
normalise_line(char **line)
{
    // skip whitespace
    while (*(*line) == '\t' || *(*line) == ' ')
        (*line)++;

    // skip comments
    char *c;
    if ((c = strchr(*line, ';')) != NULL) {
        c--;
        while (*c == '\t' || *c == ' ')
            c--;
        *(c + 1) = '\0';
    }
}

void
replace_sentinels(stack_cpu_asm_t *as)
{
    for (size_t i = 0; i < as->prog_len; i++) {
        if (as->prog[i] == 0xdeadbeef) {
            bool found = false;
            for (size_t j = 0; j < as->missing_count; j++) {
                if (as->missing[j]->addr == (uint32_t)i) {
                    for (size_t n = 0; n < as->label_count; n++) {
                        if (strcmp(as->labels[n]->name,
                                    as->missing[j]->name) == 0) {
                            found = true;
                            as->prog[i] = as->labels[n]->addr;
                        }
                    }
                }

            }

            assert(found);
        }
    }
}

void
print_prog(stack_cpu_asm_t *as)
{
    uint8_t *p = as->prog;
    for (size_t i = 0; i < as->prog_len; i += 16) {
        printf("%08x: ", (uint32_t)i);
        for (int j = 0; j < 16; j++) {
            for (int i = 0; i < 8; i++) {
                printf("%i", ((*p << i) & 0x80) != 0);
            }
            printf(" ");
            p++;
        }
        printf("\n");
    }
    printf("\n");
}

