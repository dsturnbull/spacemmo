#ifndef __src_lib_spacemmo_h
#define __src_lib_spacemmo_h

#include <stdint.h>

typedef enum packet_e {
    P_LOGIN_REQUEST,
    P_LOGIN_RESPONSE,
    P_ENTITY_REQUEST,
    P_ENTITY_RESPONSE,
    P_ENTITY_UPDATE_REQUEST,
    P_ENTITY_UPDATE_RESPONSE,
} packet_e;

typedef long client_id_t;
typedef long entity_id_t;

struct packet_st;
typedef struct packet_st packet_t;

struct client_st;
typedef struct client_st client_t;

struct client_conn_st;
typedef struct client_conn_st client_conn_t;

struct server_st;
typedef struct server_st server_t;

struct server_conn_st;
typedef struct server_conn_st server_conn_t;

struct world_st;
typedef struct world_st world_t;

struct entity_st;
typedef struct entity_st entity_t;

struct ui_st;
typedef struct ui_st ui_t;

struct gfx_st;
typedef struct gfx_st gfx_t;

struct console_st;
typedef struct console_st console_t;

struct input_st;
typedef struct input_st input_t;

struct login_request_packet_st;
typedef struct login_request_packet_st login_request_packet_t;

struct login_response_packet_st;
typedef struct login_response_packet_st login_response_packet_t;

struct entity_request_packet_t;
typedef struct entity_request_packet_st entity_request_packet_t;

struct entity_response_packet_t;
typedef struct entity_response_packet_st entity_response_packet_t;

struct entity_update_request_packet_t;
typedef struct entity_update_request_packet_st entity_update_request_packet_t;

struct entity_update_response_packet_t;
typedef struct entity_update_response_packet_st entity_update_response_packet_t;

typedef struct {
    double x, y, z;
} vec3d;

typedef struct {
    float x, y, z;
} vec3f;

typedef struct {
    int x, y, z;
} vec3i;

typedef uint8_t timer_t;

void init_spacemmo();
double time_delta(timer_t);

#endif

