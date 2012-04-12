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
    stack_cpu_asm_t *stack_cpu_asm = malloc(sizeof(stack_cpu_asm_t));
    stack_cpu_asm->labels_size = 1024;
    stack_cpu_asm->label_count = 0;
    stack_cpu_asm->labels = malloc(stack_cpu_asm->labels_size *
            sizeof(struct label *));

    stack_cpu_asm->missings_size = 1024;
    stack_cpu_asm->missing_count = 0;
    stack_cpu_asm->missing = malloc(stack_cpu_asm->labels_size *
            sizeof(struct label *));

    stack_cpu_asm->data_size = 1024;
    stack_cpu_asm->data_count = 0;
    stack_cpu_asm->data = malloc(stack_cpu_asm->data_size *
            sizeof(struct data *));

    stack_cpu_asm->prog = calloc(CPU_DATA - 1, sizeof(uint32_t));
    stack_cpu_asm->ip = stack_cpu_asm->prog;

    return stack_cpu_asm;
}

size_t
stack_cpu_asm(stack_cpu_asm_t *stack_cpu_asm, const char *fn)
{
    // make room for jumping to _main
    stack_cpu_asm->ip += 4;

    // assemble
    parse_file(stack_cpu_asm, fn);
    stack_cpu_asm->prog_len = stack_cpu_asm->ip - stack_cpu_asm->prog;

    // replace label locations etc
    replace_sentinels(stack_cpu_asm);

    // jump to main
    uint32_t main_loc;

    size_t i;
    for (i = 0; i < stack_cpu_asm->label_count; i++) {
        if (strcmp(stack_cpu_asm->labels[i]->name, "main") == 0) {
            main_loc = stack_cpu_asm->labels[i]->addr;
            break;
        }
    }

    stack_cpu_asm->ip = stack_cpu_asm->prog;
    push(stack_cpu_asm, NULL, PUSH, 0);
    push(stack_cpu_asm, NULL, main_loc, 0);
    push(stack_cpu_asm, NULL, CALL, 0);
    push(stack_cpu_asm, NULL, HLT, 0);

    free(stack_cpu_asm->labels);
    free(stack_cpu_asm->missing);
    free(stack_cpu_asm->data);

    return stack_cpu_asm->prog_len;
}

void
parse_file(stack_cpu_asm_t *stack_cpu_asm, const char *fn)
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

    define_constant(stack_cpu_asm, "KBD", IRQ_KBD);
    define_constant(stack_cpu_asm, "OUT", IRQ_TTY);
    define_constant(stack_cpu_asm, "CODE_PAGE", CPU_CODE);
    define_constant(stack_cpu_asm, "DATA_PAGE", CPU_DATA);
    define_constant(stack_cpu_asm, "STACK_PAGE", CPU_STACK);
    define_constant(stack_cpu_asm, "FRAMES_PAGE", CPU_FRAMES);
    define_constant(stack_cpu_asm, "IO_PAGE", CPU_IO);

    while ((line = strsep(&inp, "\n")) != NULL && inp != NULL) {
        lineno++;
        normalise_line(&line);

        char *ops = strsep(&line, "\t");

        if (ops[0] == '_') {
            make_label(stack_cpu_asm, ops);
            continue;
        }

        if (ops[0] == ';')
            continue;

        if (strcmp(ops, "%define") == 0) {
            char *name = strsep(&line, "\t");;
            uint32_t value;
            sscanf(strsep(&line, "\t"), "%x", &value);
            define_constant(stack_cpu_asm, name, value);
            continue;
        }

        if (strcmp(ops, "%import") == 0) {
            char *fn = strsep(&line, "\t");
            fn++; fn[strlen(fn) - 1] = '\0';
            parse_file(stack_cpu_asm, fn);
            continue;
        }

        if (strlen(ops) == 0)
            continue;

        op_t op = parse_op(ops);

        char *data;
        uint32_t addr;

        switch (op) {
            case PUSH:
                push(stack_cpu_asm, &line, op, 1);
                break;

            default:
                push(stack_cpu_asm, NULL, op, 0);
                break;
            }
    }

    free(inp);
}

uint32_t
read_value(stack_cpu_asm_t *stack_cpu_asm, char *s, uint32_t *ip)
{
    uint32_t arg = 0;

    if (s[0] == '0' && s[1] == 'x') {
        // memory address
        sscanf(s, "%x", &arg);

    } else if (s[0] == 'C' && s[1] == 'P' && s[2] == 'U') {
        // const
        if (strcmp(s, "STACK_CPU_CLOCK") == 0) {
        }

    } else if (s[0] == '[') {
        // pointer
        s++;
        s[strlen(s)] = '\0';
        sscanf(s, "%x", &arg);

    } else if (s[0] >= 'A' && s[0] <= 'Z') {
        // constant
        size_t i;
        for (i = 0; i < stack_cpu_asm->label_count; i++) {
            if (strcmp(stack_cpu_asm->labels[i]->name, s) == 0) {
                arg = stack_cpu_asm->labels[i]->addr;
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
        for (i = 0; i < stack_cpu_asm->label_count; i++) {
            if (strcmp(stack_cpu_asm->labels[i]->name, s) == 0) {
                label = stack_cpu_asm->labels[i];
                break;
            }
        }

        if (label) {
            arg = stack_cpu_asm->labels[i]->addr;
        } else {
            stack_cpu_asm->missing[stack_cpu_asm->missing_count] =
                malloc(sizeof(struct label));
            stack_cpu_asm->missing[stack_cpu_asm->missing_count]->addr = ip - stack_cpu_asm->prog;
            stack_cpu_asm->missing[stack_cpu_asm->missing_count]->name = strdup(s);
            stack_cpu_asm->missing_count++;
            arg = 0xdeadbeef;
        }
    }

    return arg;
}

void
push(stack_cpu_asm_t *stack_cpu_asm, char **line, uint32_t op, size_t n)
{
    *(stack_cpu_asm->ip)++ = op;
    for (size_t i = 0; i < n; i++) {
        char *s  = strsep(line, "\t");
        uint32_t arg = read_value(stack_cpu_asm, s, stack_cpu_asm->ip);
        *(stack_cpu_asm->ip)++ = arg;
    }
}

void
define_constant(stack_cpu_asm_t *stack_cpu_asm, char *name, uint32_t value)
{
    stack_cpu_asm->labels[stack_cpu_asm->label_count] = malloc(sizeof(struct label));
    stack_cpu_asm->labels[stack_cpu_asm->label_count]->name = strdup(name);
    stack_cpu_asm->labels[stack_cpu_asm->label_count]->addr = value;
    stack_cpu_asm->label_count++;
}

void
make_label(stack_cpu_asm_t *stack_cpu_asm, char *s)
{
    // chomp
    s++; s[strlen(s) - 1] = '\0';

    stack_cpu_asm->labels[stack_cpu_asm->label_count] = malloc(sizeof(struct label));
    stack_cpu_asm->labels[stack_cpu_asm->label_count]->name = strdup(s);
    stack_cpu_asm->labels[stack_cpu_asm->label_count]->addr = stack_cpu_asm->ip - stack_cpu_asm->prog;
    /* printf("%s == " MEM_FMT "\n", s, */
    /*         stack_cpu_asm->labels[stack_cpu_asm->label_count]->addr); */
    stack_cpu_asm->label_count++;
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
replace_sentinels(stack_cpu_asm_t *stack_cpu_asm)
{
    for (size_t i = 0; i < stack_cpu_asm->prog_len; i++) {
        if (stack_cpu_asm->prog[i] == 0xdeadbeef) {
            bool found = false;
            /* printf(MEM_FMT " is deadbeef\n", i); */
            for (size_t j = 0; j < stack_cpu_asm->missing_count; j++) {
                if (stack_cpu_asm->missing[j]->addr == (uint32_t)i) {
                    /* printf(MEM_FMT " found\n", stack_cpu_asm->missing[j]->addr); */
                    for (size_t n = 0; n < stack_cpu_asm->label_count; n++) {
                        if (strcmp(stack_cpu_asm->labels[n]->name,
                                    stack_cpu_asm->missing[j]->name) == 0) {
                            /* printf("%s == %s\n", */
                            /*         stack_cpu_asm->labels[n]->name, */
                            /*         stack_cpu_asm->missing[j]->name); */
                            found = true;
                            stack_cpu_asm->prog[i] = stack_cpu_asm->labels[n]->addr;
                        }
                    }
                }

            }

            assert(found);
        }
    }
}

void
print_prog(stack_cpu_asm_t *stack_cpu_asm)
{
    uint32_t *p = stack_cpu_asm->prog;
    for (size_t i = 0; i < stack_cpu_asm->prog_len; i += 16) {
        printf(MEM_FMT ": ", (uint32_t)i);
        for (int j = 0; j < 16; j++) {
            printf(MEM_FMT " ", *(p++));
        }
        printf("\n");
    }
    printf("\n");
}

