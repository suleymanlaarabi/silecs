#include "ecs_query.h"
#include "ecs_types.h"
#include "ecs_vec.h"
#include "ecs_system.h"
#include <assert.h>
#include <criterion/criterion.h>
#include <ecs_world.h>
#include <stdio.h>
#include "test.h"

ecs_world_t *bootstrap(void) {
    ecs_world_t *world = ecs_init();

    ECS_REGISTER_COMPONENT(world, Position);
    ECS_REGISTER_COMPONENT(world, Health);
    ECS_REGISTER_COMPONENT(world, Jump);
    return world;
}

Test(query, ecs_query_simple) {
    ecs_world_t *world = bootstrap();

    ecs_query_t pos_health_query = query({
        .terms = {
            { .id = ecs_id(Position), .oper = EcsQueryOperEqual },
            { .id = ecs_id(Health), .oper = EcsQueryOperEqual },
        },
    });

    EcsQueryId queryID = ecs_query_register(world, &pos_health_query);

    ecs_query_cache_t *matches = ECS_VEC_GET(ecs_query_cache_t, &world->queries, queryID);

    cr_assert(matches->archetypes.count == 0);

    ecs_entity_t player = ecs_new(world);

    ecs_add(world, player, ecs_id(Position));

    cr_assert(matches->archetypes.count == 0);

    ecs_add(world, player, ecs_id(Health));

    cr_assert(matches->archetypes.count == 1);

    ecs_add(world, player, ecs_id(Jump));

    cr_assert(matches->archetypes.count == 2);
}

Test(query, medium) {
    ecs_world_t *world = bootstrap();

    ecs_entity_t entity = ecs_new(world);

    ecs_query_t dependsQuery = query({
        .terms = {
            { .id = ecs_make_pair(ecs_id(EcsDependsOn), entity), .oper = EcsQueryOperEqual },
            { .id = ecs_id(EcsSystem), .oper = EcsQueryOperEqual },
            { .id = ecs_id(EcsQueryId), .oper = EcsQueryOperEqual }
        }
    });

    EcsQueryId dependsQueryId = ecs_query_register(world, &dependsQuery);

    ecs_entity_t sys = ecs_new(world);

    ecs_add_pair(world, sys, ecs_id(EcsDependsOn), entity);
    ecs_add(world, sys, ecs_id(EcsSystem));
    ecs_add(world, sys, ecs_id(EcsQueryId));

    int count = 0;
    ecs_iter_t it = ecs_query_iter(world, dependsQueryId);
    while (ecs_iter_next(&it)) {
        count++;
    }
    cr_assert(count == 1);
}
