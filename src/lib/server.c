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

void
serve(server_t *server, char *addr, unsigned short port)
{
    int sock = get_socket(addr, port);
    struct kevent ke;
    int kq;
    char buf[1024];

    // init kqueue
    if ((kq = kqueue()) == -1)
        err(EX_UNAVAILABLE, "kqueue");

    // listen for events on socket
    memset(&ke, 0, sizeof(struct kevent));
    EV_SET(&ke, sock, EVFILT_READ, EV_ADD, 0, LISTEN_BACKLOG, NULL);

    // specify kqueue update timeout
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1000 * 1000 * 1000 / SV_UPDATE_HZ;

    if (kevent(kq, &ke, 1, NULL, 0, &ts) == -1)
        err(EX_UNAVAILABLE, "set kevent");

    // register game update timer
    memset(&ke, 0, sizeof(struct kevent));
    EV_SET(&ke, 0, EVFILT_TIMER, EV_ADD, NOTE_USECONDS,
            1000 * 1000 / SV_TICK_HZ, NULL);

    if (kevent(kq, &ke, 1, NULL, 0, NULL) == -1)
        err(EX_UNAVAILABLE, "set kevent");

    // respond to events
    while (true) {
        memset(&ke, 0, sizeof(ke));
        if (kevent(kq, NULL, 0, &ke, 1, NULL) <= 0)
            err(EX_UNAVAILABLE, "kevent");

        if (ke.ident == (uintptr_t)sock) {
            // client connection
            int cl_sock;
            struct sockaddr_in c;
            socklen_t len;

            if ((cl_sock = accept(sock, (struct sockaddr *)&c, &len)) == -1)
                err(EX_UNAVAILABLE, "accept");

            // find free slot
            int slot;
            if ((slot = find_slot(server->clients)) < 0)
                continue;

            // init client
            client_conn_t *conn = init_client_connection(server, cl_sock, &c);
            conn->client = init_client();
            server->clients[slot] = conn;

            // listen to the client socket
            EV_SET(&ke, server->clients[slot]->sock, EVFILT_READ, EV_ADD, 0, 0,
                    NULL);
            if (kevent(kq, &ke, 1, NULL, 0, NULL) == -1)
                err(EX_UNAVAILABLE, "kevent add user");

            fprintf(stderr, "connection from %s\n", inet_ntoa(c.sin_addr));
        } else if (ke.ident == 0) {
            // game update timer fired
            update_server(server);
        } else {
            // client data
            int slot;
            if ((slot = find_client(server->clients, ke.ident)) < 0)
                continue;

            // read the data
            char buf[1024];
            memset(&buf, 0, sizeof(buf));
            int n = read(server->clients[slot]->sock, buf, sizeof(buf));

            // not ready yet
            if (n == -1)
                continue;

            // EOF - disconnect user
            if (n == 0) {
				EV_SET(&ke, server->clients[slot]->sock, EVFILT_READ,
                        EV_DELETE, 0, 0, NULL);
                if (kevent(kq, &ke, 1, 0, 0, NULL) == -1)
                    err(EX_UNAVAILABLE, "disconnect user");

                fprintf(stderr, "%s logged out\n",
                        server->clients[slot]->client->username);
                close(server->clients[slot]->sock);

                if (server->clients[slot]->client) {
                    if (server->clients[slot]->client->entity)
                        server->clients[slot]->client->entity->dead = true;
                    shutdown_client(server->clients[slot]->client);
                }

                free(server->clients[slot]);
                server->clients[slot] = NULL;

                continue;
            }

            // handle the data
            server_receive(server, server->clients[slot], buf, n);
        }
    }
}

int
find_slot(client_conn_t **connections)
{
    int slot = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!connections[i] || !connections[i]->sock) {
            slot = i;
            break;
        }
    }

    return slot;
}

int
find_client(client_conn_t **connections, int id)
{
    int client = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (connections[i] && connections[i]->sock == id) {
            client = i;
            break;
        }
    }

    return client;
}

client_conn_t *
init_client_connection(server_t *server, int sock, struct sockaddr_in *addr)
{
    client_conn_t *conn = malloc(sizeof(client_conn_t));
    conn->sock = sock;
    memcpy(&conn->addr, &addr->sin_addr, sizeof(addr->sin_addr));
    return conn;
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

    if (t0f > 0) {
        update_world(server->world, dt);

        foreach_entity(server->world, ^(entity_t *e) {
            foreach_client(server, ^(client_conn_t *client) {
            });
        });
    }

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
server_receive(server_t *sv, client_conn_t *cl_conn, char *buf, int len)
{
    packet_t *packet = (packet_t *)buf;

    switch (packet->type) {
        default:
            fprintf(stderr, "unhandled packet type %i\n", packet->type);
            break;
    }
}

