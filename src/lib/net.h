#ifndef __src_lib_net_h
#define __src_lib_net_h

#include "src/lib/spacemmo.h"
#include "src/lib/entity.h"

struct packet_st {
    packet_e type;
};

struct login_request_packet_st {
    packet_e type;
    char username[60];
};

struct login_response_packet_st {
    packet_e type;
    entity_t entity;
};

struct entity_request_packet_st {
    packet_e type;
};

struct entity_response_packet_st {
    packet_e type;
    entity_t entity;
};

struct entity_update_request_packet_st {
    packet_e type;
    entity_t entity;
};

struct entity_update_response_packet_st {
    packet_e type;
};

void net_send(int, char *, int);

#endif

