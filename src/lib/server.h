#ifndef __src_lib_server_h
#define __src_lib_server_h

#define MAX_CLIENTS     24
#define LISTEN_BACKLOG  24
#define LISTEN_QUEUE    24
#define SV_UPDATE_HZ    50.0f
#define SV_TICK_HZ      50.0f

#include <netinet/in.h>
#include <sys/time.h>
#include <stdbool.h>
#include "src/lib/spacemmo.h"

struct server_st {
    world_t *world;
    client_conn_t *clients[MAX_CLIENTS];
    struct timeval t;
};

struct server_conn_st {
    int sock;
    struct in_addr addr;
};

void serve(server_t *, char *, unsigned short);
int find_slot(client_conn_t **);
int find_client(client_conn_t **, int);
client_conn_t * init_client_connection(server_t *, int, struct sockaddr_in *);
void update_server(server_t *);
void foreach_client(server_t *, void(^)(client_conn_t *));

int get_socket(char *, unsigned short);
void server_receive(server_t *, client_conn_t *, char *, int);

#endif

