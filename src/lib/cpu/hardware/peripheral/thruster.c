#include <stdlib.h>

#include "src/lib/spacemmo.h"
#include "src/lib/cpu/hardware/port.h"
#include "src/lib/cpu/hardware/peripheral/thruster.h"

thruster_t *
init_thruster(port_t *port, vec3f *acc)
{
    thruster_t *thruster = calloc(1, sizeof(*thruster));
    thruster->port = port;
    thruster->acc = acc;
    port->handler = &thrust;
    port->hw = thruster;
    return thruster;
}

void
thrust(port_t *port, uint64_t c)
{
    thruster_t *thruster = (thruster_t *)port->hw;
    struct thruster_status status;

    switch ((thruster_command_t)c) {
        case THRUSTER_STATUS:
            status.x = (float)thruster->acc->x;
            status.y = (float)thruster->acc->y;
            status.z = (float)thruster->acc->z;
            write_client(port, &status, sizeof(status));
            break;

        case THRUSTER_INC_X:
            thruster->acc->x += 1;
            break;

        case THRUSTER_DEC_X:
            thruster->acc->x -= 1;
            break;

        case THRUSTER_INC_Y:
            thruster->acc->y += 1;
            break;

        case THRUSTER_DEC_Y:
            thruster->acc->y -= 1;
            break;

        case THRUSTER_INC_Z:
            thruster->acc->z += 1;
            break;

        case THRUSTER_DEC_Z:
            thruster->acc->z -= 1;
            break;
    }
}

