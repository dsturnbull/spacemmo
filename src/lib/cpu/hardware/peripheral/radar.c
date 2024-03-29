#include <stdio.h>
#include <stdlib.h>

#include "src/lib/system.h"
#include "src/lib/entity.h"
#include "src/lib/cpu/hardware/port.h"
#include "src/lib/cpu/hardware/peripheral/radar.h"

radar_t *
init_radar(port_t *port, system_t *system)
{
    radar_t *radar = calloc(1, sizeof(*radar));
    radar->port = port;
    radar->system = system;
    port->handler = &handler;
    port->hw = radar;
    return radar;
}

void
handler(port_t *port, uint64_t c)
{
    radar_t *radar = (radar_t *)port->hw;
    struct radar_status status;
    struct radar_scan scan;
    __block int i = 0;

    switch ((radar_command_t)c) {
        case RADAR_STATUS:
            status.ok = 0;
            write_client(port, &status, sizeof(status));
            break;

        case RADAR_SCAN:
            foreach_entity(radar->system, ^(entity_t *e) {
                i++;
            });
            scan.count = i;
            write_client(port, &scan, sizeof(scan));
            break;
    }
}

