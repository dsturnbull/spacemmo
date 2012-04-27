#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/stat.h>

#include "src/lib/spacemmo.h"
#include "src/lib/cpu/sasm.h"

#define GET_CONST(c) {                                                      \
    if (strcasecmp(op, #c) == 0) {                                          \
        return c;                                                           \
    }                                                                       \
} while (0)

static char *orig = NULL;

sasm_t *
init_sasm()
{
    sasm_t *sasm;
    if ((sasm = malloc(sizeof(sasm_t))) == NULL) {
        perror("init_sasm");
        return NULL;
    }

    sasm->prog_sz = 0x10000;
    sasm->prog = calloc(sasm->prog_sz, sizeof(*sasm->prog));
    sasm->ip = sasm->prog;
    sasm->lineno = -1;

    return sasm;
}

void
free_sasm(sasm_t *sasm)
{
    free(sasm->prog);
    free(sasm);
}

void
assemble(sasm_t *sasm, char *src_file)
{
    define_constant(sasm, "DBG",        IRQ_DBG);
    define_constant(sasm, "CLK",        IRQ_CLK);
    define_constant(sasm, "KBD",        IRQ_KBD);
    define_constant(sasm, "TTY",        IRQ_TTY);

    define_constant(sasm, "DISK_SET",   IRQ_DISK_SET);
    define_constant(sasm, "DISK_RD",    IRQ_DISK_RD);
    define_constant(sasm, "DISK_WR",    IRQ_DISK_WR);

    define_constant(sasm, "IO_0_IN",    IRQ_P0_IN);
    define_constant(sasm, "IO_0_OUT",   IRQ_P0_OUT);
    define_constant(sasm, "IO_1_IN",    IRQ_P1_IN);
    define_constant(sasm, "IO_1_OUT",   IRQ_P1_OUT);
    define_constant(sasm, "IO_2_IN",    IRQ_P2_IN);
    define_constant(sasm, "IO_2_OUT",   IRQ_P2_OUT);
    define_constant(sasm, "IO_3_IN",    IRQ_P3_IN);
    define_constant(sasm, "IO_3_OUT",   IRQ_P3_OUT);

    char *sys_file = replace_ext(src_file, ".sys");
    char *dbg_file = replace_ext(src_file, ".dbg");

    if ((sasm->src_fp = fopen(src_file, "r")) == NULL) {
        perror("src_fp");
        exit(1);
    }

    if ((sasm->sys_fp = fopen(sys_file, "w")) == NULL) {
        perror("sys_fp");
        exit(1);
    }

    if ((sasm->dbg_fp = fopen(dbg_file, "w")) == NULL) {
        perror("dbg_fp");
        exit(1);
    }

    // make room for jumping to _main
    sasm->ip += 7;

    // assemble
    parse_file(sasm, src_file);
    sasm->prog_len = sasm->ip - sasm->prog;

    // write data to end of prog
    write_data_labels(sasm);

    // replace label locations etc
    replace_sentinels(sasm);

    // jump to main
    uint32_t main_loc;

    size_t i;
    for (i = 0; i < MAX_LABELS; i++) {
        if (sasm->labels[i].name &&
                strcmp(sasm->labels[i].name, "_main") == 0) {
            main_loc = sasm->labels[i].addr;
            break;
        }
    }

    if (i == MAX_LABELS) {
        fprintf(stderr, "can't find _main\n");
        exit(1);
    }

    sasm->ip = sasm->prog;
    orig = NULL;
    push(sasm, NULL, PUSH, 0);
    push(sasm, NULL, main_loc >>  0 & 0xff, 0);
    push(sasm, NULL, main_loc >>  8 & 0xff, 0);
    push(sasm, NULL, main_loc >> 16 & 0xff, 0);
    push(sasm, NULL, main_loc >> 24 & 0xff, 0);
    push(sasm, NULL, CALL, 0);
    push(sasm, NULL, HLT, 0);

    fwrite(sasm->prog, sasm->prog_len, 1, sasm->sys_fp);

    fclose(sasm->sys_fp);
    free(sys_file);

    fclose(sasm->dbg_fp);
    free(dbg_file);
}

void
parse_file(sasm_t *sasm, char *fn)
{
    // load asm file
    struct stat st;
    if (stat(fn, &st) < 0) {
        perror(fn);
        return;
    }

    char *inp;
    if ((inp = malloc(st.st_size)) == NULL) {
        perror("malloc inp");
        exit(1);
    }

    FILE *fp;
    if ((fp = fopen(fn, "r")) == NULL) {
        perror(fn);
        return;
    }

    fread(inp, st.st_size, 1, fp);
    fclose(fp);

    char *line;

    while ((line = strsep(&inp, "\n")) != NULL && inp != NULL) {
        if (orig)
            free(orig);
        orig = strdup(line);
        normalise_line(&line);

        char *ops = strsep(&line, " ");
        sasm->lineno++;

        // blank line
        if (!ops) {
            continue;

        // comment
        } else if (ops[0] == ';') {
            continue;

        // label
        } else if (ops[0] == '_') {
            make_label(sasm, ops);
            continue;

        // define a preprocessor constant
        } else if (strcmp(ops, "%define") == 0) {
            char *name = strsep(&line, " ");
            uint32_t value;
            sscanf(strsep(&line, " "), "%x", &value);
            define_constant(sasm, name, value);
            continue;

        // declare a variable
        } else if (strcmp(ops, "dw") == 0) {
            char *name = strsep(&line, " ");
            char *size = strsep(&line, " ");

            uint32_t width = 4;
            if (size)
                sscanf(size, "%x", &width);

            define_variable(sasm, name, width);
            continue;

        // include another file
        } else if (strcmp(ops, "%include") == 0) {
            char *fn = strsep(&line, " ");
            fn++; fn[strlen(fn) - 1] = '\0';
            parse_file(sasm, fn);
            continue;

        // empty line
        } else if (strlen(ops) == 0) {
            continue;

        // insert data at end of code
        } else {
            char *name = ops;

            // if the line consists of more than 1 element
            if (line) {
                // if the next element is db or equ then we look at the 3rd
                ops = strsep(&line, " ");
                if (strcmp(ops, "db")  == 0 ||
                    strcmp(ops, "equ") == 0) {
                    char *data = line;
                    define_data(sasm, name, data);
                    continue;
                }
            }

            // otherwise reverse the previous manipulation
            line = ops;
            ops = name;
        }

        op_t op = parse_op(ops);

        switch (op) {
            case PUSH:
                push(sasm, &line, op, 1);
                break;

            default:
                push(sasm, NULL, op, 0);
                break;
            }
    }

    free(inp);
}

uint32_t
read_value(sasm_t *sasm, char *s, uint8_t *ip)
{
    uint32_t arg = 0;

    if (s[0] == '0' && s[1] == 'x') {
        // memory address
        sscanf(s, "%x", &arg);

    } else if (s[0] == '\'') {
        // quoted char
        arg = s[1];

    } else if ((s[0] >= 'A' && s[0] <= 'Z') ||
               (s[0] >= 'a' && s[0] <= 'z')) {
        // constant immediate
        int i;
        for (i = 0; i < MAX_LABELS; i++) {
            if (sasm->labels[i].name && strcmp(sasm->labels[i].name, s) == 0) {
                if (!sasm->labels[i].data_len > 0) {
                    arg = sasm->labels[i].addr;
                    break;
                }
            }
        }

        if (i == MAX_LABELS) {
            int i;
            for (i = 0; i < MAX_MISSINGS; i++)
                if (sasm->missing[i].name == NULL)
                    break;

            if (i == MAX_MISSINGS) {
                fprintf(stderr, "max missings reached\n");
                exit(1);
            }

            sasm->missing[i].addr = ip - sasm->prog;
            sasm->missing[i].name = strdup(s);
            arg = 0xdeadbeef;
        }

    } else {
        // label
        int i;
        for (i = 0; i < MAX_LABELS; i++)
            if (sasm->labels[i].name && strcmp(sasm->labels[i].name, s) == 0)
                break;

        if (i == MAX_LABELS) {
            int i;
            for (i = 0; i < MAX_MISSINGS; i++)
                if (sasm->missing[i].name == NULL)
                    break;

            if (i == MAX_MISSINGS) {
                fprintf(stderr, "max missings reached\n");
                exit(1);
            }

            sasm->missing[i].addr = ip - sasm->prog;
            sasm->missing[i].name = strdup(s);
            arg = 0xdeadbeef;
        } else {
            arg = sasm->labels[i].addr;
        }
    }

    return arg;
}

void
write_debug_line(sasm_t *sasm)
{
    uint32_t t = sasm->ip - sasm->prog;
    if (orig) {
        fwrite(&t, sizeof(t), 1, sasm->dbg_fp);
        t = strlen(orig);
        fwrite(&t, sizeof(t), 1, sasm->dbg_fp);
        fwrite(orig, t, 1, sasm->dbg_fp);
    }
}

void
push(sasm_t *sasm, char **line, uint8_t op, size_t n)
{
    write_debug_line(sasm);
    *(sasm->ip)++ = op;
    for (size_t i = 0; i < n; i++) {
        char *s = strsep(line, " ");
        uint32_t arg = read_value(sasm, s, sasm->ip);
        memcpy(sasm->ip, &arg, 4);
        for (int i = 0; i < 4; i++) {
            write_debug_line(sasm);
            sasm->ip++;
        }
    }
}

void
define_constant(sasm_t *sasm, char *name, uint32_t value)
{
    int i;
    for (i = 0; i < MAX_LABELS; i++)
        if (sasm->labels[i].name == NULL)
            break;

    if (i == MAX_LABELS) {
        fprintf(stderr, "too many labels\n");
        exit(1);
    }

    sasm->labels[i].name = strdup(name);
    sasm->labels[i].addr = value;
}

void
define_data(sasm_t *sasm, char *name, char *data)
{
    if (data[0] == '"') {
        // get rid of quotes
        data++; data[strlen(data) - 1] = '\0';

        int i;
        for (i = 0; i < MAX_LABELS; i++)
            if (sasm->labels[i].name == NULL)
                break;

        if (i == MAX_LABELS) {
            fprintf(stderr, "too many labels\n");
            exit(1);
        }

        sasm->labels[i].name = strdup(name);
        sasm->labels[i].data_len = strlen(data) * sizeof(uint32_t);
        sasm->labels[i].data = malloc(sasm->labels[i].data_len);

        for (size_t j = 0; j < strlen(data); j++) {
            sasm->labels[i].data[j] = data[j];
        }

    } else if (data[0] == '$') {
        data += 2; // get rid of '$-'
        //define_constant(sasm, name, data);
        // the length of the previous named data label
        int i;
        for (i = 0; i < MAX_LABELS; i++)
            if (sasm->labels[i].name &&
                    strcmp(sasm->labels[i].name, data) == 0)
                break;

        if (i == MAX_LABELS) {
            fprintf(stderr, "can't find the data label %s\n", data);
            exit(1);
        }

        // the new label
        int j;
        for (j = 0; j < MAX_LABELS; j++)
            if (sasm->labels[j].name == NULL)
                break;

        sasm->labels[j].name = strdup(name);
        sasm->labels[j].addr = sasm->labels[i].data_len / sizeof(uint32_t);
    }
}

void
define_variable(sasm_t *sasm, char *name, size_t width)
{
    // make room for a word
    int i;
    for (i = 0; i < MAX_LABELS; i++)
        if (sasm->labels[i].name == NULL)
            break;

    if (i == MAX_LABELS) {
        fprintf(stderr, "too many labels\n");
        exit(1);
    }

    sasm->labels[i].name = strdup(name);
    sasm->labels[i].data_len = width;
    sasm->labels[i].data = NULL;
}

void
make_label(sasm_t *sasm, char *s)
{
    // chomp
    s[strlen(s) - 1] = '\0';
    /* fprintf(stderr, "%s = %08lx\n", s, sasm->ip - sasm->prog); */
    define_constant(sasm, s, sasm->ip - sasm->prog);
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
    GET_CONST(AND);
    GET_CONST(OR);
    GET_CONST(JMP);
    GET_CONST(JE);
    GET_CONST(JZ);
    GET_CONST(JNZ);
    GET_CONST(CALL);
    GET_CONST(RET);
    GET_CONST(DUP);
    GET_CONST(PUSH);
    GET_CONST(POP);
    GET_CONST(SWAP);
    GET_CONST(INT);

    fprintf(stderr, "%s\n", op);
    assert(false);
}

void
normalise_line(char **line)
{
    char *out = calloc(strlen(*line), 1);

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

    // write each whitespace-delimited field
    char *field;
    while ((field = strsep(line, "\t ")) != NULL) {
        if (field[0] == '\0')
            continue;
        strcat(out, field);
        strcat(out, " ");
    }
    out[strlen(out) - 1] = '\0';

    // copy temp string to src
    *line = strdup(out);

    free(out);
}

void
write_data_labels(sasm_t *sasm)
{
    sasm->data = sasm->prog + sasm->prog_len;
    for (size_t i = 0; i < MAX_LABELS; i++) {
        if (sasm->labels[i].data_len > 0) {
            if (sasm->labels[i].data)
                memcpy(sasm->data, sasm->labels[i].data,
                        sasm->labels[i].data_len);
            sasm->labels[i].addr = sasm->data - sasm->prog;
            sasm->data += sasm->labels[i].data_len;
            sasm->prog_len += sasm->labels[i].data_len;
        }
    }
}

void
replace_sentinels(sasm_t *sasm)
{
    for (int i = 0; i < sasm->prog_len; i++) {
        if (sasm->prog[i + 0] == 0xef &&
            sasm->prog[i + 1] == 0xbe &&
            sasm->prog[i + 2] == 0xad &&
            sasm->prog[i + 3] == 0xde) {
            bool found = false;
            for (size_t j = 0; j < MAX_MISSINGS; j++) {
                if (sasm->missing[j].addr == (uint32_t)i) {
                    for (size_t n = 0; n < MAX_LABELS; n++) {
                        // TODO strncmp ?
                        if (sasm->labels[n].name
                                && strcmp(sasm->labels[n].name,
                                    sasm->missing[j].name) == 0) {
                            found = true;
                            memcpy(&sasm->prog[i], &sasm->labels[n].addr, 4);
                        }
                    }
                }
            }
            if (!found) {
                fprintf(stderr, "unresolved: %08x\n", i);
                print_prog(sasm);
                exit(1);
            }
        }
    }
}

void
print_prog(sasm_t *sasm)
{
    uint8_t *p = sasm->prog;
    for (int i = 0; i < sasm->prog_len; i += 16) {
        fprintf(stderr, "%08x: ", i);
        for (int j = 0; j < 16; j++) {
            fprintf(stderr, "%02x ", *(p++));
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}

