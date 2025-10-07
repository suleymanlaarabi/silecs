#include "ecs_query.h"
#include "ecs_types.h"
#include <ecs_world.h>
#include <ecs_system.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

ECS_COMPONENT_DEFINE(EcsSystem);
ECS_COMPONENT_DEFINE(EcsDependsOn);
ECS_COMPONENT_DEFINE(EcsEnables);
ECS_COMPONENT_DEFINE(EcsPhase);
ECS_TAG_DEFINE(EcsOnPreUpdate);
ECS_TAG_DEFINE(EcsOnUpdate);
ECS_TAG_DEFINE(EcsOnPostUpdate);



ecs_entity_t ecs_register_system(ecs_world_t *world, ecs_iter_func func, ecs_query_t *query) {
    ecs_entity_t entity = ecs_new(world);
    EcsQueryId queryId = ecs_query_register(world, query);

    ecs_query_t dependsQuery = query({
        .terms = {
            { .id = ecs_make_pair(ecs_id(EcsDependsOn), entity), .oper = EcsQueryOperEqual },
            { .id = ecs_id(EcsSystem), .oper = EcsQueryOperEqual },
            { .id = ecs_id(EcsQueryId), .oper = EcsQueryOperEqual }
        },
    });

    EcsQueryId dependsQueryId = ecs_query_register(world, &dependsQuery);

    ecs_add(world, entity, ecs_id(EcsSystem));
    ecs_add(world, entity, ecs_id(EcsQueryId));
    ecs_set(world, entity, ecs_id(EcsSystem), &(EcsSystem) {
        .func = func,
        .dependsQuery = dependsQueryId
    });
    ecs_set(world, entity, ecs_id(EcsQueryId), &queryId);
    return entity;
}

ecs_entity_t ecs_register_system_phase(
    ecs_world_t *world,
    ecs_iter_func func,
    ecs_entity_t phase,
    ecs_query_t *query
) {
    ecs_entity_t system = ecs_register_system(world, func, query);

    ecs_add_pair(world, system, ecs_id(EcsPhase), phase);
    free(query);
    return system;
}

ecs_entity_t ecs_register_system_root(ecs_world_t *world, ecs_iter_func func, ecs_query_t *query) {
    ecs_entity_t entity = ecs_register_system(world, func, query);
    ecs_add_pair(world, entity, ecs_id(EcsPhase), ecs_new(world));
    return entity;
}

void ecs_invoke_system(ecs_world_t *world, EcsSystem *system, EcsQueryId query) {
    ecs_iter_t it = ecs_query_iter(world, query);

    while (ecs_iter_next(&it)) {
        system->func(&it);
    }
}

void ecs_invoke_systems(ecs_world_t *world, EcsQueryId query) {
    ecs_iter_t it = ecs_query_iter(world, query);
    while (ecs_iter_next(&it)) {
        EcsSystem *systems = ecs_field(&it, EcsSystem);
        EcsQueryId *queryIds = ecs_field(&it, EcsQueryId);

        for (uint32_t i = 0; i < it.count; i++) {
            ecs_invoke_system(world,
                &systems[i],
                queryIds[i]
            );
            ecs_invoke_systems(world, systems[i].dependsQuery);
        }
    }
}

void ecs_run_phase(ecs_world_t *world) {
    ecs_invoke_systems(world, world->phases_query);
}

void EcsSystemModule(ecs_world_t *world) {
    ECS_REGISTER_COMPONENT(world, EcsSystem);
    ECS_REGISTER_COMPONENT(world, EcsDependsOn);
    ECS_REGISTER_COMPONENT(world, EcsEnables);
    ECS_REGISTER_COMPONENT(world, EcsPhase);
    ECS_TAG_REGISTER(world, EcsOnPreUpdate);
    ECS_TAG_REGISTER(world, EcsOnUpdate);
    ECS_TAG_REGISTER(world, EcsOnPostUpdate);

    ecs_query_t phase_query = query({
        .terms = {
            { .id = ecs_make_pair(ecs_id(EcsPhase), ecs_id(EcsWildcard)), .oper = EcsQueryOperEqual },
            { .id = ecs_id(EcsSystem), .oper = EcsQueryOperEqual },
            { .id = ecs_id(EcsQueryId), .oper = EcsQueryOperEqual }
        },
    });

    world->phases_query = ecs_query_register(world, &phase_query);
}
