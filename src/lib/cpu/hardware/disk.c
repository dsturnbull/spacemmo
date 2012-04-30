#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "src/lib/cpu/hardware/disk.h"

disk_t *
init_disk(char *fn)
{
    disk_t *disk = calloc(1, sizeof(disk_t));

    disk->fn = strdup(fn);

    if ((disk->fd = open(disk->fn, O_RDWR | O_CREAT, 0000600)) < 0) {
        perror("open");
        exit(1);
    }

    lseek(disk->fd, CPU_DISK_SIZE - 1, SEEK_SET);
    write(disk->fd, "", 1);
    lseek(disk->fd, 0, SEEK_SET);

    if ((disk->map = mmap(0, CPU_DISK_SIZE, PROT_READ | PROT_WRITE,
            MAP_SHARED, disk->fd, 0)) == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    disk->curpos = disk->map;
    disk->pos = disk->curpos - disk->map;

    return disk;
}

void
set_disk_position(disk_t *disk, size_t position)
{
    disk->curpos = &disk->map[position];
    disk->pos = disk->curpos - disk->map;
}

uint8_t
read_disk(disk_t *disk)
{
    uint8_t c = *(disk->curpos++);
    disk->pos = disk->curpos - disk->map;
    return c;
}

void
write_disk(disk_t *disk, uint8_t c)
{
    *(disk->curpos++) = c;
    disk->pos = disk->curpos - disk->map;
    msync(disk->map, 0, CPU_DISK_SIZE - 1);
}

