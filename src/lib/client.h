#ifndef __src_lib_client_h
#define __src_lib_client_h

#define LISTEN_BACKLOG  24
#define LISTEN_QUEUE    24
#define FRAMERATE_HZ    60
#define TICK_TIMER      0
#define FRAME_TIMER     1

#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/event.h>
#include <sys/time.h>

#include "src/lib/spacemmo.h"

typedef struct entity_st entity_t;
typedef struct ui_st ui_t;
typedef struct server_st server_t;
typedef struct server_conn_st server_conn_t;

typedef struct client_st {
    entity_t *entity;
    ui_t *ui;
    server_t *server;
    server_conn_t *server_conn;

    client_id_t id;
    char *username;

    struct timeval t;

    struct kevent ke;
    int kq;
    int server_sock;
    bool quit;
} client_t;

struct server_conn_st {
    int sock;
    struct in_addr addr;
};

client_t * init_client();
void init_client_kqueue(client_t *);
void client_loop(client_t *);
void handle_client_events(client_t *);
void update_client(client_t *);

bool connect_server(server_conn_t **, char *, unsigned short);
void handle_server_response(client_t *, char *, int);

#endif

