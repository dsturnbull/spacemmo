#include <err.h>
#include <stdio.h>
#include <unistd.h>

#include "src/lib/net.h"

void
net_send(int sock, char *buf, int len)
{
    if ((len = write(sock, buf, len)) == -1)
        warn("write failed");

    /*
    for (int i = 0; i < len; i++)
        printf("%02x ", buf[i]);
    printf("\n");
    */
}

