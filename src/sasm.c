#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/lib/cpu/sasm.h"

int
main(int argc, char *argv[])
{
    char *fn = argv[1];
    char *nfn = strdup(fn);
    char *nfn2 = nfn;
    char *obj_fn;
    asprintf(&obj_fn, "%s.o", strsep(&nfn2, "."));
    free(nfn);

    FILE *fp;
    if ((fp = fopen(obj_fn, "w")) == NULL) {
        perror("fopen");
        exit(1);
    }

    sasm_t *sasm = init_sasm();
    assemble(sasm, fn);
    print_prog(sasm);

    fwrite(sasm->prog, sasm->prog_len, 1, fp);
    fclose(fp);

    free(sasm->prog);
    free(sasm);

    return 0;
}

