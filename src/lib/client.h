#ifndef __src_lib_client_h
#define __src_lib_client_h

#define LISTEN_BACKLOG  24
#define LISTEN_QUEUE    24
#define FRAMERATE_HZ    60

#define TICK_TIMER      0
#define FRAME_TIMER     1

#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "src/lib/spacemmo.h"

struct client_st {
    bool quit;
    client_id_t id;
    server_t *server;
    server_conn_t *server_conn;
    struct timeval t;
    entity_t *entity;
    char *username;
    struct ui_st *ui;
};

struct client_conn_st {
    int sock;
    struct in_addr addr;
    struct client_st *client;
};

client_t * init_client();
void shutdown_client(client_t *);
void update_client(client_t *);

void client_loop(client_t *);
bool connect_server(server_conn_t **, char *, unsigned short);
void client_send(client_t *, packet_e);
void client_receive(client_t *, char *, int);

void send_login_request(client_t *);
void receive_login_response(client_t *, login_response_packet_t *);
void send_entity_request(client_t *, entity_id_t);
void receive_entity_response(client_t *, entity_response_packet_t *);
void send_entity_update_request(client_t *);
void receive_entity_update_response(client_t *, 
        entity_update_response_packet_t *);

#endif

