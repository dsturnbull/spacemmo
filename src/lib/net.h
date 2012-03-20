#ifndef __src_lib_net_h
#define __src_lib_net_h

#include <sys/types.h>

#include "src/lib/spacemmo.h"
#include "src/lib/entity.h"

struct packet_st {
    packet_e type;
    size_t len;
};

struct login_request_packet_st {
    packet_e type;
    size_t len;
    char username[60];
};

struct login_response_packet_st {
    packet_e type;
    size_t len;
    entity_t entity;
};

struct entity_request_packet_st {
    packet_e type;
    size_t len;
};

struct entity_response_packet_st {
    packet_e type;
    size_t len;
    entity_t entity;
};

struct entity_update_request_packet_st {
    packet_e type;
    size_t len;
    entity_t entity;
};

struct entity_update_response_packet_st {
    packet_e type;
    size_t len;
};

void net_send(int, char *, int);

#endif

