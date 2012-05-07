#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "src/test.h"
#include "src/lib/cpu/cpu.h"

cpu_t *cpu;
instruction_t *op;

#define passert(fn) do {                                                    \
    cpu = init_cpu();                                                       \
    printf("%s: ", #fn);                                                    \
    if (fn()) {                                                             \
        printf("ok\n");                                                     \
    } else {                                                                \
        printf("fail, %s:%i\n", __FILE__, __LINE__);                        \
    }                                                                       \
    free_cpu(cpu);                                                          \
} while (0)

int
main(int argc, char *argv[])
{
    op = malloc(sizeof(*op));

    // TODO move passerts into tests, actually.
    passert(test_nop);
    passert(test_hlt);
    passert(test_call);
    passert(test_ret);
    passert(test_load);
    passert(test_store);
    passert(test_add);
    passert(test_sub);
    passert(test_mul);
    passert(test_div);
    passert(test_and);
    passert(test_or);
    passert(test_jmp);
    passert(test_je);
    passert(test_jne);
    passert(test_jz);
    passert(test_jnz);
    passert(test_dup);
    passert(test_push);
    passert(test_pop);
    passert(test_swap);
    passert(test_int);
    return 0;
}

bool
test_nop()
{
    op->op = NOP;
    uint8_t *ip = cpu->ip;
    handle_nop(cpu, op);
    return cpu->ip - ip == 1;
}

bool
test_hlt()
{
    op->op = HLT;
    handle_hlt(cpu, op);
    return cpu->ts == NULL;
}

bool
test_call()
{
    op->op = CALL;
    assert(cpu->ip - cpu->mem == 0);            // starts at 0
    handle_call(cpu, op, 20);                   // jump to 20
    assert(*(uint64_t *)(cpu->rp - 8) == 1);    // rp is next after origin
    assert(cpu->ip - cpu->mem == 20);           // jumped to 20
    handle_call(cpu, op, 40);                   // jump to 40
    assert(*(uint64_t *)(cpu->rp - 8) == 21);   // rp is next after origin
    assert(cpu->ip - cpu->mem == 40);           // jumped to 40
    return true;
}

bool
test_ret()
{
    op->op = CALL;
    assert(cpu->ip - cpu->mem == 0);            // starts at 0
    handle_call(cpu, op, 20);                   // jump to 20
    assert(*(uint64_t *)(cpu->rp - 8) == 1);    // rp is next after origin

    op->op = RET;
    handle_ret(cpu, op);
    assert(cpu->ip - cpu->mem == 1);            // returned to rp
	return true;
}

bool
test_load()
{
	return true;
}

bool
test_store()
{
	return true;
}

bool
test_add()
{
	return true;
}

bool
test_sub()
{
	return true;
}

bool
test_mul()
{
	return true;
}

bool
test_div()
{
	return true;
}

bool
test_and()
{
	return true;
}

bool
test_or()
{
	return true;
}

bool
test_jmp()
{
	return true;
}

bool
test_je()
{
	return true;
}

bool
test_jne()
{
	return true;
}

bool
test_jz()
{
	return true;
}

bool
test_jnz()
{
	return true;
}

bool
test_dup()
{
	return true;
}

bool
test_push()
{
	return true;
}

bool
test_pop()
{
	return true;
}

bool
test_swap()
{
	return true;
}

bool
test_int()
{
	return true;
}

