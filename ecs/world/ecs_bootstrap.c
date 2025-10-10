#include "ecs_strmap.h"
#include "ecs_types.h"
#include <ecs_bootstrap.h>
#include <ecs_world.h>
#include <stdio.h>

ECS_TAG_DEFINE(EcsWildcard);
ECS_TAG_DEFINE(EcsChildOf);
ECS_COMPONENT_DEFINE(EcsComponent);

void OnAddName(ecs_world_t *world, ecs_entity_t entity) {
    EcsName *name = ecs_get(world, entity, ecs_id(EcsName));

    ecs_strmap_set(&world->entity_map, *name, entity);
}

void EcsBootstrapModule(ecs_world_t *world) {
    ecs_set_hook(world, ecs_id(EcsName), OnAddName);

    ECS_REGISTER_COMPONENT(world, EcsComponent);
    ecs_add(world, ecs_id(EcsName), ecs_id(EcsComponent));

    ECS_TAG_REGISTER(world, EcsWildcard);
    ECS_TAG_REGISTER(world, EcsChildOf);

    const char *ChildOfName = "ChildOf";
    const char *EcsName = "EcsName";

    ecs_set(world, ecs_id(EcsChildOf), ecs_id(EcsName), &ChildOfName);
    ecs_add(world, ecs_id(EcsName), ecs_id(EcsName));
    ecs_set(world, ecs_id(EcsName), ecs_id(EcsName), &EcsName);
}
