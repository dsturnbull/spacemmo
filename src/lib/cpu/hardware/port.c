#include <fcntl.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "src/lib/cpu/hardware/port.h"

port_t *
init_port(int n)
{
    port_t *port = calloc(1, sizeof(*port));
    port->n = n;

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

bool
read_port(port_t *port, char *c)
{
    if (read(port->r, c, 1) == 1)
        return true;
    return false;
}

void
write_port(port_t *port, char c)
{
    if (port->handler)
        port->handler(port, c);
}

void
write_client(port_t *port, void *data, size_t len)
{
    printf("port%i ->", port->n);
    for (size_t i = 0; i < len; i++)
        printf(" %02x", ((uint8_t *)data)[i]);
    printf("\n");
    write(port->w, data, len);
}

