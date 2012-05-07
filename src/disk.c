#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "src/disk.h"
#include "src/lib/cpu/hardware/peripheral/disk.h"

int
main(int argc, char *argv[])
{
    disk_utility_t *util = malloc(sizeof(*util));
    util->disk = init_disk(NULL, "/tmp/disk0");

    // ignore invocation
    argc -= 1;
    argv += 1;

    if (!parse_opts(util, argc, argv)) {
        fprintf(stderr, "invalid options\n");
        help();
    }

    free_disk(util->disk);
    free(util);

    return 0;
}

void
help()
{
}

bool
parse_opts(disk_utility_t *util, int argc, char *argv[])
{
    if (argc == 0)
        return false;

    size_t ncmds = sizeof(cmds) / sizeof(struct cmd);
    for (size_t i = 0; i < ncmds; i++) {
        if (strcmp(argv[0], cmds[i].name) == 0) {
            argc -= 1;
            argv += 1;
            cmds[i].fn(util, argc, argv);
            return true;
        }
    }

    return false;
}

void
cmd_status(disk_utility_t *util, int argc, char *argv[])
{
    printf("pos: %lu\n", util->disk->pos);
    printf("size: %lu\n", util->disk->sz);
}

void
cmd_format(disk_utility_t *util, int argc, char *argv[])
{
    static const uint8_t empty[1024];

    for (size_t i = 0; i < util->disk->sz; i += 1024) {
        write_disk(util->disk, (uint8_t *)empty, 1024);
    }
}

void
cmd_list(disk_utility_t *util, int argc, char *argv[])
{
    fat_entry_t *fat;

    for (int i = 0; i < FAT_LEN; i++) {
        uint8_t *buf;

        size_t n = read_disk(util->disk, (uint8_t **)&buf, sizeof(*fat));
        fat = (fat_entry_t *)buf;

        if (fat->name_len)
            printf("%s\n", fat->name);
    }
}

void
cmd_put(disk_utility_t *util, int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: disk put src dst\n");
        return;
    }

    char *src = argv[0], *dst = argv[1];
    argc -= 2;
    argv += 2;

    printf("put %s -> %s\n", src, dst);

    char *fn = strrchr(dst, '/');
    if (fn == NULL)
        fn = dst;
    else
        fn++;

    // first, see if any entry already exists by the fn
    // look for free fat entry
    uint8_t *buf, *p;
    set_disk_position(util->disk, 0);
    read_disk(util->disk, &buf, FAT_LEN * sizeof(fat_entry_t));

    p = buf;
    fat_entry_t *fat;
    int i;
    for (i = 0; i < FAT_LEN; i++) {
        fat = (fat_entry_t *)p;
        p += sizeof(*fat);
        if (!fat->name_len)
            break;
    }

    strncpy(fat->name, fn, NAME_LEN);
    fat->name_len = strlen(fn);

    set_disk_position(util->disk, i * sizeof(fat_entry_t));
    write_disk(util->disk, (uint8_t *)fat, sizeof(*fat));

    // TODO do some shit

    free(buf);

    cmd_status(util, argc, argv);
}

void
cmd_create(disk_utility_t *util, int argc, char *argv[])
{
    fat_entry_t fat;
    memset(&fat, 0, sizeof(fat));
    fat.name_len = 0xf;
    size_t head = 0xffffffffffffffff;
    memcpy(fat.head, &head, 8);
    printf("fat\n");
    for (size_t i = 0; i < sizeof(fat); i++) {
        if (i % 16 == 0)
            printf("\n%04lx:", i);
        printf(" %02x", ((uint8_t *)&fat)[i]);
    }
    printf("\n");
}

