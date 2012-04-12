#ifndef __src_lib_net_h
#define __src_lib_net_h

#include <sys/types.h>

#include "src/lib/spacemmo.h"
#include "src/lib/entity.h"

typedef enum packet_e {
    P_LOGIN_REQUEST,   
    P_LOGIN_RESPONSE,  
    P_ENTITY_REQUEST,  
    P_ENTITY_RESPONSE, 
} packet_e;            

typedef struct packet_st {
    packet_e type;
    size_t len;
} packet_t;

struct login_request_packet_st {
    packet_t packet;
    char username[60];
};

struct login_response_packet_st {
    packet_t packet;
    entity_id_t id;
};

struct entity_request_packet_st {
    packet_t packet;
    entity_id_t id;
};

struct entity_response_packet_st {
    packet_t packet;
    entity_t entity;
};

struct entity_update_request_packet_st {
    packet_t packet;
    entity_t entity;
};

struct entity_update_response_packet_st {
    packet_t packet;
};

void net_send(int, char *, int);

#endif

