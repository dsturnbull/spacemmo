#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/stat.h>

#include "src/lib/spacemmo.h"
#include "src/lib/cpu/cpu.h"
#include "src/lib/cpu/sasm/sasm.h"

FILE *yyin;

sasm_t *
init_sasm()
{
    sasm_t *sasm = calloc(1, sizeof(*sasm));

    sasm->prog_sz = 0x10000;
    sasm->prog = calloc(sasm->prog_sz, sizeof(sasm->prog));
    sasm->ip = sasm->prog;

    sasm->variables_sz = 1;
    sasm->variables = calloc(sasm->variables_sz, sizeof(variable_t));
    if (sasm->variables == NULL) {
        perror("calloc");
        exit(1);
    }

    return sasm;
}

void
free_sasm(sasm_t *sasm)
{
    free(sasm->prog);
    for (size_t i = 0; i < sasm->variables_len; i++)
        if(sasm->variables[i].name)
            free(sasm->variables[i].name);
    free(sasm->variables);
    free(sasm);
}

void
assemble(sasm_t *sasm, char *src_file)
{
    // make room for jumping to _main
    sasm->ip += 11;

    define_constant(sasm, "CLK",        IRQ_CLK);
    define_constant(sasm, "KBD",        IRQ_KBD);
    define_constant(sasm, "TTY",        IRQ_TTY);

    define_constant(sasm, "IO_0",       IRQ_P0);
    define_constant(sasm, "IO_0_OUT",   IRQ_P0_OUT);
    define_constant(sasm, "IO_1",       IRQ_P1);
    define_constant(sasm, "IO_1_OUT",   IRQ_P1_OUT);
    define_constant(sasm, "IO_2",       IRQ_P2);
    define_constant(sasm, "IO_2_OUT",   IRQ_P2_OUT);
    define_constant(sasm, "IO_3",       IRQ_P3);
    define_constant(sasm, "IO_3_OUT",   IRQ_P3_OUT);

    char *sys_file = replace_ext(src_file, ".sys");

    if ((sasm->src_fp = fopen(src_file, "r")) == NULL) {
        perror("src_fp");
        exit(1);
    }

    if ((sasm->sys_fp = fopen(sys_file, "w")) == NULL) {
        perror("sys_fp");
        exit(1);
    }

    // assemble
    yyin = sasm->src_fp;
    yyparse();

    // write data to end of prog
    write_data(sasm);
    sasm->prog_len = sasm->ip - sasm->prog;

    // jump to main
    write_prologue(sasm);

    fwrite(sasm->prog, sasm->prog_len, 1, sasm->sys_fp);
    fclose(sasm->sys_fp);
    free(sys_file);
}

void
push0(sasm_t *sasm, op_t op, size_t len)
{
    opcode_t opcode;
    opcode.op = op;
    opcode.flags = opflags(len);

    fprintf(stderr, "push0 %016lx = op:%02x\n", sasm->ip - sasm->prog, op);

    memcpy(sasm->ip, &opcode, sizeof(uint8_t));
    sasm->ip++;
}

void
push1(sasm_t *sasm, op_t op, void *data, size_t len)
{
    opcode_t opcode;
    opcode.op = op;
    opcode.flags = opflags(len);

    fprintf(stderr, "push1 %016lx = op:%02x data:%016llx\n",
            sasm->ip - sasm->prog, op, *(uint64_t *)data);

    memcpy(sasm->ip++, &opcode, sizeof(uint8_t));
    for (size_t i = 0; i < len; i++) {
        fprintf(stderr, "push1 %016lx = db:%02x\n",
                sasm->ip - sasm->prog, ((uint8_t *)data)[i]);
        *sasm->ip++ = ((char *)data)[i];
    }
}

variable_t *
define_variable(sasm_t *sasm, char *name, uint64_t value, size_t len)
{
    variable_t *var = find_or_create_variable(sasm, name);
    var->value = value;
    var->len = len;
    return var;
}

variable_t *
new_variable(sasm_t *sasm)
{
    if (sasm->variables_len == sasm->variables_sz) {
        sasm->variables_sz *= 2;
        sasm->variables = realloc(sasm->variables,
                sasm->variables_sz * sizeof(variable_t));
    }

    return &sasm->variables[sasm->variables_len++];
}

variable_t *
find_variable(sasm_t *sasm, char *name)
{
    for (size_t i = 0; i < sasm->variables_len; i++)
        if (strcmp(sasm->variables[i].name, name) == 0)
            return &sasm->variables[i];
    return NULL;
}

variable_t *
find_or_create_variable(sasm_t *sasm, char *name)
{
    variable_t *variable;
    
    if ((variable = find_variable(sasm, name)) == NULL) {
        variable = new_variable(sasm);
        variable->name = strdup(name);
    }

    return variable;
}

void
add_variable_ref(sasm_t *sasm, variable_t *var, uint64_t ref)
{
    if (var->refs_sz == var->refs_len) {
        var->refs_sz *= 2;
        if (var->refs_sz == 0)
            var->refs_sz = 1;

        var->refs = realloc(var->refs, var->refs_sz * sizeof(uint8_t *));

        if (var->refs == NULL) {
            perror("calloc");
            exit(1);
        }
    }

    var->refs[var->refs_len++] = ref;
}

variable_t *
define_constant(sasm_t *sasm, char *name, uint64_t addr)
{
    variable_t *var = new_variable(sasm);
    var->name = strdup(name);
    var->addr = addr;
    return var;
}

variable_t *
define_data(sasm_t *sasm, char *name, char *data)
{
    variable_t *var = new_variable(sasm);

    var->name = strdup(name);
    var->len = strlen(data);
    var->data = malloc(var->len);

    for (size_t j = 0; j < strlen(data); j++)
        var->data[j] = data[j];

    return var;
}

void
define_relative(sasm_t *sasm)
{
    //if (data[0] == '$') {
    //    data += 2; // get rid of '$-'
    //    //define_constant(sasm, name, data);
    //    // the length of the previous named data label
    //    int i;
    //    for (i = 0; i < MAX_LABELS; i++)
    //        if (sasm->labels[i].name &&
    //                strcmp(sasm->labels[i].name, data) == 0)
    //            break;

    //    // the new label
    //    int j;
    //    for (j = 0; j < MAX_LABELS; j++)
    //        if (sasm->labels[j].name == NULL)
    //            break;

    //    sasm->labels[j].name = strdup(name);
    //    sasm->labels[j].addr = sasm->labels[i].len / sizeof(uint64_t);
    //}
}

variable_t *
define_label(sasm_t *sasm, char *s)
{
    variable_t *var = find_variable(sasm, s);
    if (var) {
        printf("already found %s, new addr (%016lx), ref %016llx\n",
                var->name, sasm->ip - sasm->prog, var->addr);
        var->addr = sasm->ip - sasm->prog;
    } else {
        printf("haven't seen %s, adding constant @ %016lx\n",
                s, sasm->ip - sasm->prog);
        var = define_constant(sasm, s, sasm->ip - sasm->prog);
    }

    return var;
}

void
write_prologue(sasm_t *sasm)
{
    // jump to main
    variable_t *main_loc = find_variable(sasm, "_main");
    sasm->ip = sasm->prog;
    push1(sasm, PUSH, &main_loc->addr, 8);
    push0(sasm, CALL, 1);
    push0(sasm, HLT, 1);
}

void
write_data(sasm_t *sasm)
{
    for (size_t i = 0; i < sasm->variables_len; i++) {
        variable_t *var = &sasm->variables[i];

        if (var->len > 0) {
            if (var->data) {
                printf("writing %s ", var->data);
                memcpy(sasm->ip, var->data, var->len);
            } else {
                printf("writing %016llx ", var->value);
                memcpy(sasm->ip, &var->value, var->len);
            }

            printf("@ %016lx for %s\n", sasm->ip - sasm->prog,
                    var->name);

            var->addr = sasm->ip - sasm->prog;
        }

        for (size_t j = 0; j < var->refs_len; j++) {
            printf("%s replacing ref at %016llx with %016llx\n",
                    var->name,
                    var->refs[j],
                    var->addr);

            memcpy(sasm->prog + var->refs[j], &var->addr, 8);
        }

        sasm->ip += var->len;
    }
}

uint8_t
opflags(size_t sz)
{
    // TODO do this smartly
    switch (sz) {
        case 1:
            return 0;
        case 2:
            return 1;
        case 4:
            return 2;
        case 8:
            return 3;

        default:
            fprintf(stderr, "invalid size: %lu\n", sz);
            exit(1);
    }
}

void
print_prog(sasm_t *sasm)
{
    uint8_t *p = sasm->prog;
    for (int i = 0; i < sasm->prog_len; i += 16) {
        fprintf(stderr, "%016x: ", i);
        for (int j = 0; j < 16; j++) {
            fprintf(stderr, "%02x ", *(p++));
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}

