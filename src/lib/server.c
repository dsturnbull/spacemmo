#include <arpa/inet.h>
#include <err.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sysexits.h>
#include <unistd.h>

#include "src/lib/client.h"
#include "src/lib/server.h"
#include "src/lib/world.h"
#include "src/lib/net.h"

server_t *
init_server()
{
    server_t *server = calloc(1, sizeof(server_t));
    server->clients = calloc(16, sizeof(client_conn_t));
    server->client_count = 16;
    server->world = init_world();
    server->sock = -1;
    return server;
}

void
init_server_kqueue(server_t *server)
{
    // init kqueue
    if ((server->kq = kqueue()) == -1)
        err(EX_UNAVAILABLE, "kqueue");

    memset(&server->ke, 0, sizeof(struct kevent));

    // listen for events on socket
    if (server->host && server->port) {
        server->sock = get_socket(server->host, server->port);
        EV_SET(&server->ke, server->sock, EVFILT_READ, EV_ADD, 0,
                LISTEN_BACKLOG, NULL);
    }

    // register game update timer
    memset(&server->ke, 0, sizeof(struct kevent));
    EV_SET(&server->ke, 0, EVFILT_TIMER, EV_ADD, NOTE_USECONDS,
            1000 * 1000 / SV_TICK_HZ, NULL);

    if (kevent(server->kq, &server->ke, 1, NULL, 0, NULL) == -1)
        err(EX_UNAVAILABLE, "set game update kevent");

    // specify kqueue update timeout
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1000 * 1000 * 1000 / SV_UPDATE_HZ;

    if (kevent(server->kq, &server->ke, 1, NULL, 0, &ts) == -1)
        err(EX_UNAVAILABLE, "set update timeout kevent");
}

void
server_loop(server_t *server)
{
    while (true)
        handle_server_events(server);
}

void
handle_server_events(server_t *server)
{
    // respond to events
    memset(&server->ke, 0, sizeof(server->ke));
    if (kevent(server->kq, NULL, 0, &server->ke, 1, NULL) <= 0)
        err(EX_UNAVAILABLE, "handle_server_events");

    if (server->ke.ident == (uintptr_t)server->sock) {
        // client connection
        int cl_sock;
        struct sockaddr_in c;
        socklen_t len;

        if ((cl_sock = accept(server->sock,
                        (struct sockaddr *)&c, &len)) == -1)
            err(EX_UNAVAILABLE, "accept");

        // init client
        client_conn_t *conn = calloc(1, sizeof(client_conn_t));
        conn->sock = cl_sock;
        memcpy(&conn->addr, &c.sin_addr, sizeof(c.sin_addr));
        conn->client = init_client();
        server->clients[server->client_count++] = conn;

        // listen to the client socket
        EV_SET(&server->ke, conn->sock, EVFILT_READ, EV_ADD, 0, 0, NULL);
        if (kevent(server->kq, &server->ke, 1, NULL, 0, NULL) == -1)
            err(EX_UNAVAILABLE, "kevent add user");

        fprintf(stderr, "connection from %s\n", inet_ntoa(c.sin_addr));
    } else if (server->ke.ident == 0) {
        // game update timer fired
        update_server(server);
    } else {
        // client data
        client_conn_t *conn = find_client(server, server->ke.ident);

        // read the data
        char buf[1024];
        memset(&buf, 0, sizeof(buf));
        int n = read(conn->sock, buf, sizeof(buf));

        // not ready yet
        if (n == -1)
            return;

        // EOF - disconnect user
        if (n == 0) {
            EV_SET(&server->ke, conn->sock, EVFILT_READ, EV_DELETE, 0, 0,
                    NULL);
            if (kevent(server->kq, &server->ke, 1, 0, 0, NULL) == -1)
                err(EX_UNAVAILABLE, "disconnect user");

            fprintf(stderr, "%s logged out\n", conn->client->username);
            close(conn->sock);

            if (conn->client)
                if (conn->client->entity)
                    conn->client->entity->dead = true;

            free(conn);

            return;
        }

        // handle the data
        handle_client_request(server, conn, buf, n);
    }
}

client_conn_t *
find_client(server_t *server, int id)
{
    for (int i = 0; i < server->client_count; i++)
        if (server->clients[i] && server->clients[i]->sock == id)
            return server->clients[i];

    return NULL;
}

void
update_server(server_t *server)
{
    struct timeval t1;
    gettimeofday(&t1, NULL);

    double t0f = (double)server->t.tv_sec +
        (double)server->t.tv_usec / 1000 / 1000;
    double t1f = (double)t1.tv_sec +
        (double)t1.tv_usec / 1000 / 1000;
    double dt = t1f - t0f;

    if (t0f > 0)
        update_world(server->world, dt);

    gettimeofday(&server->t, NULL);
}

void
foreach_client(server_t *server, void(^block)(client_conn_t *))
{
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (server->clients[i] && server->clients[i]->sock)
            block(server->clients[i]);
}

int
get_socket(char *addr, unsigned short port)
{
    int error;
    int sock;
    struct sockaddr_in serv;

    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        err(EX_UNAVAILABLE, "unable to create socket");

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&error,
                sizeof(error)) == -1)
        warn("setsockopt");

    memset(&serv, 0, sizeof(struct sockaddr_in));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    serv.sin_addr.s_addr = inet_addr(addr);

    if (bind(sock, (struct sockaddr *)&serv, sizeof(serv)) == -1)
        err(EX_UNAVAILABLE, "bind");

    if (listen(sock, LISTEN_BACKLOG) == -1)
        err(EX_UNAVAILABLE, "listen");

    return sock;
}

void
handle_client_request(server_t *sv, client_conn_t *cl, char *buf, int len)
{
    packet_t *packet = (packet_t *)buf;

    switch (packet->type) {
        default:
            fprintf(stderr, "unhandled packet type %i\n", packet->type);
            break;
    }
}

