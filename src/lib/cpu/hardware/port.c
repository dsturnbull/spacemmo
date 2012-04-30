#include <fcntl.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "src/lib/cpu/hardware/port.h"

port_t *
init_port(int n, uint8_t *dma, size_t len)
{
    port_t *port = calloc(1, sizeof(*port));
    port->n = n;
    port->dma = dma;
    port->dma_len = len;

    int fds[2];
    if (socketpair(PF_LOCAL, SOCK_STREAM, 0, fds) == -1) {
        perror("socketpair");
        exit(1);
    }

    port->r = fds[0];
    port->w = fds[1];

    fcntl(port->r, F_SETFL, O_NONBLOCK);
    fcntl(port->w, F_SETFL, O_NONBLOCK);

    return port;
}

size_t
read_port(port_t *port, uint8_t *data, size_t len)
{
    int r;
    if ((r = read(port->r, data, len)) > 0)
        return r;
    return 0;
}

void
write_port(port_t *port, uint8_t *data, size_t len)
{
    if (port->handler)
        port->handler(port, data, len);
}

void
write_client(port_t *port, void *data, size_t len)
{
    write(port->w, data, len);
}

