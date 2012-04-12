#include <stdio.h>
#include <stdlib.h>

#include "src/lib/cpu/hardware/comm_port.h"

comm_port_t *
init_comm_port()
{
    comm_port_t *kbd = calloc(1, sizeof(comm_port_t));
    return kbd;
}

