#include "src/lib/spacemmo.h"
#include "src/lib/cpu/sasm.h"

int
main(int argc, char *argv[])
{
    sasm_t *sasm = init_sasm();
    assemble(sasm, argv[1]);
    //print_prog(sasm);
    free_sasm(sasm);
    return 0;
}

