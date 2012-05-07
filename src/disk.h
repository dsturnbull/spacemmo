#ifndef __src_disk_h
#define __src_disk_h

#include <stdint.h>
#include <stdbool.h>

#define NAME_LEN    0x100
#define FAT_LEN     0x1000

/*
 * disk
 *  0x00000001  0x00109000  fat_entry_t[4096]
 */

typedef struct disk_st disk_t;

typedef struct disk_utility_st {
    disk_t *disk;
} disk_utility_t;

struct cmd {
    char *name;
    void (*fn)(disk_utility_t *, int, char **);
};

typedef struct fat_entry_st {
    uint8_t name_len;
    uint8_t head[8];
    char name[NAME_LEN];
} fat_entry_t;

void help();
bool parse_opts(disk_utility_t *, int, char **);
void cmd_status(disk_utility_t *, int, char **);
void cmd_format(disk_utility_t *, int, char **);
void cmd_list(disk_utility_t *, int, char **);
void cmd_put(disk_utility_t *, int, char **);

struct cmd cmds[] = {
    { "status",     &cmd_status },
    { "format",     &cmd_format },
    { "ls",         &cmd_list   },
    { "put",        &cmd_put    },
};

#endif

