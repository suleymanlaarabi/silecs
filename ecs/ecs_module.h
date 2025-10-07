#ifndef ECS_MODULE_H
    #define ECS_MODULE_H
    #include "ecs_types.h"
#define ECS_IMPORT(world, module) ecs_module_register(world, module, #module)


typedef struct ecs_world_t ecs_world_t;

typedef void (*ecs_module_func)(struct ecs_world_t *world);

ECS_TAG_DECLARE(EcsModule);


void ecs_init_module(ecs_world_t *world);
void ecs_module_register(
    ecs_world_t *world,
    ecs_module_func func,
    char *name
);

#endif
