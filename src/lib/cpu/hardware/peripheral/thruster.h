#ifndef __src_lib_cpu_hardware_peripheral_thruster_h
#define __src_lib_cpu_hardware_peripheral_thruster_h

typedef struct port_st port_t;
typedef struct vec3f_st vec3f;

typedef struct thruster_st {
    port_t *port;
    vec3f *acc;
} thruster_t;

typedef enum thrust_command_e {
    THRUSTER_STATUS,
    THRUSTER_INC_X,
    THRUSTER_DEC_X,
    THRUSTER_INC_Y,
    THRUSTER_DEC_Y,
    THRUSTER_INC_Z,
    THRUSTER_DEC_Z,
} thruster_command_t;

struct thruster_status {
    float x, y, z;
};

thruster_t * init_thruster(port_t *, vec3f *);
void thrust(port_t *, char);

#endif

