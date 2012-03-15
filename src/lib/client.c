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

client_t *
init_client()
{
    client_t *client = calloc(1, sizeof(client_t));
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
        EV_SET(&ke, client->server_conn->sock, EVFILT_READ, EV_ADD, 0, LISTEN_BACKLOG, NULL);

        // specify kqueue update timeout
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 25 * 1000 * 1000;

        if (kevent(kq, &ke, 1, NULL, 0, &ts) == -1)
            err(EX_UNAVAILABLE, "set kevent");
    }

    // register game update timer
    memset(&ke, 0, sizeof(struct kevent));
    EV_SET(&ke, 0, EVFILT_TIMER, EV_ADD, NOTE_USECONDS, 1000 * 1000 / TICK_HZ, NULL);

    if (kevent(kq, &ke, 1, NULL, 0, NULL) == -1)
        err(EX_UNAVAILABLE, "set kevent");

    // register gfx update timer
    memset(&ke, 0, sizeof(struct kevent));
    EV_SET(&ke, 1, EVFILT_TIMER, EV_ADD, NOTE_USECONDS, 1000 * 1000 / FRAMERATE_HZ, NULL);

    if (kevent(kq, &ke, 1, NULL, 0, NULL) == -1)
        err(EX_UNAVAILABLE, "set kevent");

    while (true) {
        memset(&ke, 0, sizeof(ke));
        if (kevent(kq, NULL, 0, &ke, 1, NULL) <= 0)
            continue;

        if (client->server_conn && ke.ident == (uintptr_t)client->server_conn->sock) {
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
				EV_SET(&ke, client->server_conn->sock, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                if (kevent(kq, &ke, 1, 0, 0, NULL) == -1)
                    err(EX_UNAVAILABLE, "disconnect user");

                close(client->server_conn->sock);
                client->server_conn->sock = 0;

                continue;
            }

            // handle packet
            client_receive(client, buf, n);
        } else if (ke.ident == 0) {
            // timer fired
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
    struct sockaddr_in server;
    struct hostent *server_addr;

    (*s) = calloc(1, sizeof(client_t));

    if (((*s)->sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        err(EX_UNAVAILABLE, "socket");

    if ((server_addr = gethostbyname(addr)) == NULL)
        err(EX_UNAVAILABLE, "gethostbyname");

    server.sin_family = PF_INET;
    memcpy(&server.sin_addr.s_addr, server_addr->h_addr, server_addr->h_length);
    server.sin_port = htons(port);

    if (connect((*s)->sock, (struct sockaddr *)&server, len) == -1)
        err(EX_UNAVAILABLE, "connect");

    printf("connected\n");
    return true;
}

void
client_send(client_t *client, packet_e type)
{
    switch (type) {
        case P_LOGIN_REQUEST:
            send_login_request(client);
            break;

        case P_ENTITY_UPDATE_REQUEST:
            send_entity_update_request(client);
            break;

        default:
            printf("unhandled packet\n");
            break;
    }
}

void
client_receive(client_t *server, char *buf, int len)
{
    int consumed;
    packet_t *packet = (packet_t *)buf;

    switch (packet->type) {
        case P_LOGIN_RESPONSE:
            receive_login_response(server, (login_response_packet_t *)buf);
            consumed = sizeof(login_response_packet_t);
            break;

        case P_ENTITY_RESPONSE:
            receive_entity_response(server, (entity_response_packet_t *)buf);
            consumed = sizeof(entity_response_packet_t);
            break;

        default:
            printf("unhandled packet\n");
            break;
    }

    if (len - consumed > 0)
        client_receive(server, buf + consumed, len - consumed);
}

void
send_login_request(client_t *client)
{
    login_request_packet_t p;
    memset(&p, 0, sizeof(p));
    p.type = P_LOGIN_REQUEST;
    strncpy(p.username, client->username, sizeof(p.username));
    net_send(client->server_conn->sock, (char *)&p, sizeof(p));
}

void
receive_login_response(client_t *client, login_response_packet_t *packet)
{
    client->entity = malloc(sizeof(entity_t));
    memcpy(client->entity, &packet->entity, sizeof(entity_t));
}

void
send_entity_request(client_t *client, entity_id_t id)
{
}

void
receive_entity_response(client_t *client, entity_response_packet_t *packet)
{
    if (client->entity && packet->entity.id == client->entity->id) {
        update_entity_state(client->entity, &packet->entity);
        entity_t *e = client->entity;
        static bool initialised = false;
        if (!initialised) {
            init_gfx_ship_ui(client->ui->gfx);
            initialised = true;
        }
    }
}

void
send_entity_update_request(client_t *client)
{
    entity_update_request_packet_t p;
    memset(&p, 0, sizeof(p));
    p.type = P_ENTITY_UPDATE_REQUEST;
    memcpy(&p.entity, client->entity, sizeof(p.entity));
    net_send(client->server_conn->sock, (char *)&p, sizeof(p));
}

void
receive_entity_update_response(client_t *client, entity_update_response_packet_t *packet)
{
}

