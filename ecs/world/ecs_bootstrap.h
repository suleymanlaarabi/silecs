#ifndef ECS_BOOTSTRAP_H
    #define ECS_BOOTSTRAP_H
    #include "ecs_types.h"

typedef struct ecs_world_t ecs_world_t;

ECS_TAGS(
    EcsWildcard,
    EcsChildOf,
    EcsComponent
)

ECS_COMPONENT_DECLARE(EcsWildcard);
ECS_COMPONENT_DECLARE(EcsChildOf);
ECS_COMPONENT_DECLARE(EcsComponent);

void EcsBootstrapModule(ecs_world_t *world);

#endif
