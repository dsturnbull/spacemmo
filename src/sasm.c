#include "src/lib/spacemmo.h"
#include "src/lib/cpu/sasm/sasm.h"

extern sasm_t *ysasm;

int
main(int argc, char *argv[])
{
    ysasm = init_sasm();
    assemble(ysasm, argv[1]);
    print_prog(ysasm);
    return 0;
}

