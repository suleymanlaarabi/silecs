#include "ecs_types.h"
#include <ecs_module.h>
#include <ecs_world.h>


ECS_TAG_DEFINE(EcsModule);

void ecs_init_module(ecs_world_t *world) {
    ECS_TAG_REGISTER(world, EcsModule);
}

void ecs_module_register(
    ecs_world_t *world,
    ecs_module_func func,
    char *name
) {
    func(world);

    ecs_entity_t module_entity = ecs_new(world);
    ecs_add(world, module_entity, ecs_id(EcsModule));
    ecs_add(world, module_entity, ecs_id(EcsName));
    ecs_set(world, module_entity, ecs_id(EcsName), &name);
}
