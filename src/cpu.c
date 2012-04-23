#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <getopt.h>

#include "src/lib/cpu/cpu.h"

int
main(int argc, char *argv[])
{
    int ch;
    cpu_t *cpu = init_cpu();

    while ((ch = getopt(argc, argv, "df:")) != EOF) {
        switch (ch) {
            case 'd':
                cpu->debug = true;
                break;

            case 'f':
                cpu->log = fopen(optarg, "a");
                break;
        }
    }

    argc -= optind;
    argv += optind;

    char *fn = argv[0];

    struct stat st;
    if (stat(fn, &st) < 0) {
        perror("stat");
        exit(1);
    }

    FILE *fp;
    if ((fp = fopen(fn, "r")) == NULL) {
        perror("fopen");
        exit(1);
    }

    char *buf = malloc(st.st_size);
    fread(buf, st.st_size, 1, fp);
    fclose(fp);

    cpu->halted = false;
    load_cpu(cpu, (uint8_t *)buf, st.st_size);
    free(buf);

    while (true)
        run_cpu(cpu);

    return 0;
}

