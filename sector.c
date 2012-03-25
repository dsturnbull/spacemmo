#include <stdlib.h>

#include "src/lib/sector.h"
#include "src/lib/system.h"
#include "src/lib/entity.h"

sector_t *
init_sector(system_t *system)
{
    sector_t *sector = calloc(1, sizeof(sector_t));
    sector->system = system;
    sector->entities_size = 1024;
    sector->entities = calloc(1024, sizeof(sector_t));
    sector->entity_count = 0;
    add_sector(system, sector);
    return sector;
}

void
update_sector(sector_t *sector, double dt)
{
    foreach_entity(sector, ^(entity_t *entity) {
        update_entity(entity, dt);
    });
}

bool
add_entity(sector_t *sector, entity_t *entity)
{
    sector->entities[sector->entity_count++] = entity;
    return true;
}

bool
remove_entity(sector_t *sector, entity_t *entity)
{
    return true;
}

void
foreach_entity(sector_t *sector, void (^block)(entity_t *)) {
    for (size_t i = 0; i < sector->entity_count; i++)
        block((entity_t *)sector->entities[i]);
}


