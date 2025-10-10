#include "stdio_devtool_query.h"

#include <stdint.h>
#include <stdio.h>

#include "ecs_debug.h"
#include "ecs_query.h"
#include "ecs_vec.h"

void stdio_devtool_execute_query(ecs_world_t *world, const char *query_str)
{
    ecs_query_t *query = ecs_query_from_str(world, query_str);
    if (!query) {
        puts("Invalid Query");
        return;
    }

    ecs_iter_t iter = ecs_query(world, query);
    bool found = false;

    while (ecs_iter_next(&iter)) {
        for (int i = 0; i < iter.count; i++) {
            found = true;
            ecs_entity_t entity = *ECS_VEC_GET(
                ecs_entity_t,
                &iter.archetype_p->entities,
                i
            );
            ecs_print_entity(world, entity);
        }
    }

    if (!found)
        puts("No entities found");
}
