#include <arpa/inet.h>
#include <assert.h>
#include <dispatch/dispatch.h>
#include <err.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sysexits.h>
#include <unistd.h>

#include "src/lib/client.h"
#include "src/lib/server.h"
#include "src/lib/net.h"
#include "src/lib/ui.h"
#include "src/lib/ui/gfx.h"
#include "src/lib/ui/input.h"
#include "src/lib/world.h"

client_t *
init_client()
{
    client_t *client = calloc(1, sizeof(client_t));
    client->server = calloc(1, sizeof(server_t));
    client->server->world = init_world();
    client->entity = calloc(1, sizeof(entity_t));
    add_entity(client->server->world, client->entity);
    return client;
}

void
shutdown_client(client_t *client)
{
    free(client->entity);
    free(client);
}

void
update_client(client_t *client)
{
    update_server(client->server);
}

void
client_loop(client_t *client)
{
    struct kevent ke;
    int kq;
    int server_sock;

    // init kqueue
    if ((kq = kqueue()) == -1)
        err(EX_UNAVAILABLE, "kqueue");

    // listen on server socket
    if (client->server_conn) {
        memset(&ke, 0, sizeof(struct kevent));
        EV_SET(&ke, client->server_conn->sock, EVFILT_READ, EV_ADD, 0,
                LISTEN_BACKLOG, NULL);

        // specify kqueue update timeout
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 25 * 1000 * 1000;

        if (kevent(kq, &ke, 1, NULL, 0, &ts) == -1)
            err(EX_UNAVAILABLE, "set kevent");
    }

    // register game update timer
    memset(&ke, 0, sizeof(struct kevent));
    EV_SET(&ke, 0, EVFILT_TIMER, EV_ADD, NOTE_USECONDS,
            1000 * 1000 / SV_TICK_HZ, NULL);

    if (kevent(kq, &ke, 1, NULL, 0, NULL) == -1)
        err(EX_UNAVAILABLE, "set kevent");

    // register gfx update timer
    memset(&ke, 0, sizeof(struct kevent));
    EV_SET(&ke, 1, EVFILT_TIMER, EV_ADD, NOTE_USECONDS,
            1000 * 1000 / FRAMERATE_HZ, NULL);

    if (kevent(kq, &ke, 1, NULL, 0, NULL) == -1)
        err(EX_UNAVAILABLE, "set kevent");

    while (true) {
        memset(&ke, 0, sizeof(ke));
        if (kevent(kq, NULL, 0, &ke, 1, NULL) <= 0)
            continue;

        if (client->server_conn &&
                ke.ident == (uintptr_t)client->server_conn->sock) {
            // traffic on server socket
            struct sockaddr_in s;
            socklen_t len;
            char buf[1024];

            int n = read(client->server_conn->sock, buf, sizeof(buf));

            // not ready yet
            if (n == -1)
                continue;

            // disconnect
            if (n == 0) {
				EV_SET(&ke, client->server_conn->sock, EVFILT_READ, EV_DELETE,
                        0, 0, NULL);
                if (kevent(kq, &ke, 1, 0, 0, NULL) == -1)
                    err(EX_UNAVAILABLE, "disconnect user");

                close(client->server_conn->sock);
                client->server_conn->sock = 0;

                continue;
            }

            // handle packet
            client_receive(client, buf, n);
        } else if (ke.ident == 0) {
            // game update timer fired
            update_client(client);
        } else if (ke.ident == 1) {
            // gfx fired
            update_ui(client->ui, time_delta(FRAME_TIMER));
        }

        if (client->quit)
            return;
    }
}

bool
connect_server(server_conn_t **s, char *addr, unsigned short port)
{
    socklen_t len = sizeof(struct sockaddr);
    struct sockaddr_in sv;
    struct hostent *server_addr;

    (*s) = calloc(1, sizeof(client_t));

    if (((*s)->sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        err(EX_UNAVAILABLE, "socket");

    if ((server_addr = gethostbyname(addr)) == NULL)
        err(EX_UNAVAILABLE, "gethostbyname");

    sv.sin_family = PF_INET;
    memcpy(&sv.sin_addr.s_addr, server_addr->h_addr, server_addr->h_length);
    sv.sin_port = htons(port);

    if (connect((*s)->sock, (struct sockaddr *)&sv, len) == -1)
        err(EX_UNAVAILABLE, "connect");

    fprintf(stderr, "connected\n");
    return true;
}

void
client_receive(client_t *server, char *buf, int len)
{
    int consumed;
    packet_t *packet = (packet_t *)buf;

    switch (packet->type) {
        default:
            fprintf(stderr, "unhandled packet %i\n", packet->type);
            consumed = len; // maybe fuck
            break;
    }

    if (len - consumed > 0)
        client_receive(server, buf + consumed, len - consumed);
}

