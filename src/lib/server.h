#ifndef __src_lib_server_h
#define __src_lib_server_h

#define MAX_CLIENTS     24
#define LISTEN_BACKLOG  24
#define LISTEN_QUEUE    24
#define SV_UPDATE_HZ    50.0f
#define SV_TICK_HZ      50.0f

#include <netinet/in.h>
#include <stdbool.h>
#include <sys/event.h>
#include <sys/time.h>

#include "src/lib/spacemmo.h"

struct server_st {
    world_t *world;
    client_conn_t **clients;
    int client_count;

    char *host;
    unsigned short port;

    struct timeval t;

    struct kevent ke;
    int kq;
    int sock;
};

struct client_conn_st {
    int sock;
    struct in_addr addr;
    struct client_st *client;
};

server_t * init_server();
void init_server_kqueue(server_t *);
void server_loop(server_t *);
void handle_server_events(server_t *);
void update_server(server_t *);

client_conn_t * find_client(server_t *, int);
void foreach_client(server_t *, void(^)(client_conn_t *));

int get_socket(char *, unsigned short);
void handle_client_request(server_t *, client_conn_t *, char *, int);

#endif

