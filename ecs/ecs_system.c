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

    ecs_add(world, entity, ecs_id(EcsSystem));
    ecs_add(world, entity, ecs_id(EcsQueryId));
    ecs_set(world, entity, ecs_id(EcsSystem), &(EcsSystem) {
        .func = func,
    });
    ecs_set(world, entity, ecs_id(EcsQueryId), &queryId);
    return entity;
}

ecs_entity_t ecs_system(
    ecs_world_t *world,
    ecs_iter_func func,
    ecs_entity_t phase,
    ecs_query_t *query
) {
    ecs_entity_t system = ecs_register_system(world, func, query);

    ecs_add_pair(world, system, ecs_id(EcsPhase), phase);
    return system;
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

        for (int i = 0; i < it.count; i++) {
            ecs_invoke_system(world,
                &systems[i],
                queryIds[i]
            );
        }
    }
}

bool ecs_progress(ecs_world_t *world) {
    ecs_invoke_systems(world, world->OnPreUpdateQuery);
    ecs_invoke_systems(world, world->OnUpdateQuery);
    ecs_invoke_systems(world, world->OnPostUpdateQuery);
    return true;
}

void EcsSystemModule(ecs_world_t *world) {
    ECS_REGISTER_COMPONENT(world, EcsSystem);
    ECS_REGISTER_COMPONENT(world, EcsDependsOn);
    ECS_REGISTER_COMPONENT(world, EcsEnables);
    ECS_REGISTER_COMPONENT(world, EcsPhase);

    ECS_TAG_REGISTER(world, EcsOnPreUpdate);
    ECS_TAG_REGISTER(world, EcsOnUpdate);
    ECS_TAG_REGISTER(world, EcsOnPostUpdate);

    ecs_query_t onPreUpdateQuery = query({
        .terms = {
            { .id = ecs_make_pair(ecs_id(EcsPhase), ecs_id(EcsOnPreUpdate)), .oper = EcsQueryOperEqual },
            { .id = ecs_id(EcsSystem), .oper = EcsQueryOperEqual },
            { .id = ecs_id(EcsQueryId), .oper = EcsQueryOperEqual }
        },
    });

    ecs_query_t onUpdateQuery = query({
        .terms = {
            { .id = ecs_make_pair(ecs_id(EcsPhase), ecs_id(EcsOnUpdate)), .oper = EcsQueryOperEqual },
            { .id = ecs_id(EcsSystem), .oper = EcsQueryOperEqual },
            { .id = ecs_id(EcsQueryId), .oper = EcsQueryOperEqual }
        },
    });

    ecs_query_t onPostUpdateQuery = query({
        .terms = {
            { .id = ecs_make_pair(ecs_id(EcsPhase), ecs_id(EcsOnPostUpdate)), .oper = EcsQueryOperEqual },
            { .id = ecs_id(EcsSystem), .oper = EcsQueryOperEqual },
            { .id = ecs_id(EcsQueryId), .oper = EcsQueryOperEqual }
        },
    });

    world->OnPreUpdateQuery = ecs_query_register(world, &onPreUpdateQuery);
    world->OnUpdateQuery = ecs_query_register(world, &onUpdateQuery);
    world->OnPostUpdateQuery = ecs_query_register(world, &onPostUpdateQuery);
}
