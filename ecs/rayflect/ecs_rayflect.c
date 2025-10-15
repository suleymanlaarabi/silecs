#include "ecs_rayflect.h"
#include "../ecs_types.h"
#include "../world/ecs_world.h"

ECS_COMPONENT_DEFINE(EcsStruct);
ECS_COMPONENT_DEFINE(EcsPrimitive);

void EcsRayflectModule(ecs_world_t *world) {
    ECS_REGISTER_COMPONENT(world, EcsStruct);
    ECS_REGISTER_COMPONENT(world, EcsPrimitive);
}
