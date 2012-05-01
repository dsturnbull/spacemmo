#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "src/lib/cpu/hardware/port.h"
#include "src/lib/cpu/hardware/peripheral/disk.h"

disk_t *
init_disk(port_t *port, char *fn)
{
    disk_t *disk = calloc(1, sizeof(disk_t));

    disk->port = port;
    port->handler = &disk_handler;
    port->hw = disk;

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
disk_handler(port_t *port, uint8_t *data, size_t len)
{
    disk_t *disk = (disk_t *)port->hw;
    int response = 0;
    size_t pos = 0;

    //printf("disk <- ");
    //for (size_t i = 0; i < len; i++)
    //    printf("%02x ", data[i]);
    //printf("\n");

    disk_cmd_t cmd = 0;
    memcpy(&cmd, data, 1);
    data++;
    len -=1;

    switch (cmd) {
        case DISK_STATUS:
            printf("disk status\n");
            response = 0;
            write_client(port, &response, 1);
            break;

        case DISK_SET:
            memcpy(&pos, data, len);
            printf("disk set to %08lx\n", pos);
            set_disk_position(disk, pos);
            response = 0;
            write_client(port, &response, 1);
            break;

        case DISK_RD:
            memcpy(&len, data, len);
            printf("disk read %08lx bytes <- %08lx\n", len, disk->pos);
            response = 0;
            write_client(port, &response, 1);
            break;

        case DISK_WR:
            printf("disk write %08lx bytes -> %08lx\n", port->dma_len,
                    disk->pos);
            write_disk(disk, port->dma, port->dma_len);
            response = 0;
            write_client(port, &response, 1);
            break;
    }
}

void
set_disk_position(disk_t *disk, size_t position)
{
    disk->curpos = &disk->map[position];
    disk->pos = disk->curpos - disk->map;
}

size_t
read_disk(disk_t *disk, uint8_t **buf, size_t len)
{
    *buf = malloc(len);
    memcpy(*buf, disk->curpos, len);
    disk->curpos += len;
    disk->pos = disk->curpos - disk->map;
    return len;
}

void
write_disk(disk_t *disk, uint8_t *data, size_t len)
{
    memcpy(disk->curpos, data, len);
    disk->curpos += len;
    disk->pos = disk->curpos - disk->map;
    msync(disk->map, 0, CPU_DISK_SIZE - 1);
}

