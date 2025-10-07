#include "ecs_archetype.h"
#include "ecs_sparseset.h"
#include "ecs_strmap.h"
#include "ecs_types.h"
#include "ecs_vec.h"
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

void OnAddComponent(ecs_world_t *world, ecs_entity_t entity) {
    ecs_vec_t vec;
    ecs_vec_init(&vec, sizeof(ecs_archetype_id_t));

    ecs_sparseset_insert(&world->component_archetypes, entity.value, &vec);
}

void EcsBootstrapModule(ecs_world_t *world) {
    ECS_TAG_REGISTER(world, EcsWildcard);
    ECS_TAG_REGISTER(world, EcsChildOf);
    ECS_REGISTER_COMPONENT(world, EcsComponent);

    ecs_set_hook(world, ecs_id(EcsName), OnAddName);
    ecs_add_hook(world, ecs_id(EcsComponent), OnAddComponent);
}
