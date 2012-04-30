#ifndef __src_lib_cpu_hardware_peripheral_radar_h
#define __src_lib_cpu_hardware_peripheral_radar_h

#include <stdint.h>

typedef struct port_st port_t;
typedef struct system_st system_t;
typedef struct entity_st entity_t;

typedef struct radar_st {
    port_t *port;
    system_t *system;
} radar_t;

typedef enum radar_command_e {
    RADAR_STATUS,
    RADAR_SCAN,
} radar_command_t;

struct radar_status {
    uint8_t ok;
};

struct radar_scan {
    uint32_t count;
};

radar_t * init_radar(port_t *, system_t *);
void free_radar(radar_t *);
void handler(port_t *, uint64_t);

#endif

