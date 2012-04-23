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
#include "src/lib/world.h"
#include "src/lib/cluster.h"
#include "src/lib/system.h"
#include "src/lib/cpu/cpu.h"

client_t *
init_client()
{
    client_t *client = calloc(1, sizeof(client_t));
    client->server = init_server();
    return client;
}

void
init_client_kqueue(client_t *client)
{
    // init kqueue
    if ((client->kq = kqueue()) == -1)
        err(EX_UNAVAILABLE, "kqueue");

    // listen on server socket
    if (client->server_conn) {
        memset(&client->ke, 0, sizeof(struct kevent));
        EV_SET(&client->ke, client->server_conn->sock, EVFILT_READ, EV_ADD, 0,
                LISTEN_BACKLOG, NULL);

        // specify kqueue update timeout
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 25 * 1000 * 1000;

        if (kevent(client->kq, &client->ke, 1, NULL, 0, &ts) == -1)
            err(EX_UNAVAILABLE, "set server listen kevent");
    }

    // register game update timer
    memset(&client->ke, 0, sizeof(struct kevent));
    EV_SET(&client->ke, 0, EVFILT_TIMER, EV_ADD, NOTE_USECONDS,
            1000 * 1000 / SV_TICK_HZ, NULL);

    if (kevent(client->kq, &client->ke, 1, NULL, 0, NULL) == -1)
        err(EX_UNAVAILABLE, "set game update kevent");

    // register cpu update timer
    memset(&client->ke, 0, sizeof(struct kevent));
    EV_SET(&client->ke, 1, EVFILT_TIMER, EV_ADD, NOTE_NSECONDS, 1, NULL);

    if (kevent(client->kq, &client->ke, 1, NULL, 0, NULL) == -1)
        err(EX_UNAVAILABLE, "set cpu update kevent");
}

void
client_loop(client_t *client)
{
    while (!client->quit) {
        handle_client_events(client);
        //handle_server_events(client->server);
    }
}

void
handle_client_events(client_t *client)
{
    memset(&client->ke, 0, sizeof(client->ke));
    if (kevent(client->kq, NULL, 0, &client->ke, 1, NULL) <= 0)
        return;

    if (client->server_conn &&
            client->ke.ident == (uintptr_t)client->server_conn->sock) {
        // traffic on server socket
        struct sockaddr_in s;
        socklen_t len;
        char buf[1024];

        int n = read(client->server_conn->sock, buf, sizeof(buf));

        // not ready yet
        if (n == -1)
            return;

        // disconnect
        if (n == 0) {
            EV_SET(&client->ke, client->server_conn->sock, EVFILT_READ,
                    EV_DELETE, 0, 0, NULL);
            if (kevent(client->kq, &client->ke, 1, 0, 0, NULL) == -1)
                err(EX_UNAVAILABLE, "disconnect user");

            close(client->server_conn->sock);
            client->server_conn->sock = 0;

            return;
        }

        // handle packet
        handle_server_response(client, buf, n);
    } else if (client->ke.ident == 0) {
        // game update timer fired
        update_client(client);
    } else if (client->ke.ident == 1) {
        // cpu fired
    }

    update_cpus(client);

    if (client->quit)
        return;
}

void
update_client(client_t *client)
{
    update_server(client->server);
}

void
update_cpus(client_t *client)
{
    foreach_cluster(client->server->world, ^(cluster_t *cluster) {
        foreach_system(cluster, ^(system_t *system) {
            foreach_entity(system, ^(entity_t *entity) {
                if (entity->cpu) {
                    run_cpu(entity->cpu);
                }
            });
        });
    });
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
handle_server_response(client_t *client, char *buf, int len)
{
    int consumed;
    packet_t *packet = (packet_t *)buf;

    /*
    switch (packet->type) {
        default:
            fprintf(stderr, "unhandled packet %i\n", packet->type);
            consumed = len; // maybe fuck
            break;
    }

    if (len - consumed > 0)
        handle_server_response(client, buf + consumed, len - consumed);
    */
}

