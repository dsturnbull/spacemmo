#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "src/lib/cpu/cpu.h"
#include "src/lib/cpu/hardware/peripheral/disk.h"

int
main(int argc, char *argv[])
{
    int ch;
    cpu_t *cpu = init_cpu();
    char *sys_file;

    while ((ch = getopt(argc, argv, "df:")) != EOF) {
        switch (ch) {
            case 'd':
                cpu->debug = true;
                break;

            case 'f':
                sys_file = strdup(optarg);
                break;
        }
    }

    argc -= optind;
    argv += optind;

    disk_t *disk = init_disk(cpu->port0, "/tmp/disk");

    cpu->halted = false;
    load_cpu(cpu, sys_file);

    while (true)
        run_cpu(cpu);

    free_cpu(cpu);
    return 0;
}

