#ifndef ECS_SYSTEM_H
    #define ECS_SYSTEM_H
    #include "ecs_query.h"
    #include "ecs_types.h"
    #define ECS_SYSTEM(world, func, phase, ...) ecs_register_system_phase(world, func, ecs_id(phase), ecs_query_from_str(world, #__VA_ARGS__));

typedef struct ecs_world_t ecs_world_t;


typedef void (*ecs_iter_func)(ecs_iter_t *it);

typedef struct {
    ecs_iter_func func;
    EcsQueryId dependsQuery;
} EcsSystem;

typedef struct {

} EcsPhase;

typedef struct {

} EcsDependsOn;

typedef struct {

} EcsEnables;

ECS_COMPONENT_DECLARE(EcsDependsOn);
ECS_COMPONENT_DECLARE(EcsEnables);
ECS_COMPONENT_DECLARE(EcsSystem);
ECS_COMPONENT_DECLARE(EcsPhase);
ECS_TAG_DECLARE(EcsOnPreUpdate);
ECS_TAG_DECLARE(EcsOnUpdate);
ECS_TAG_DECLARE(EcsOnPostUpdate);

void EcsSystemModule(ecs_world_t *world);
void ecs_run_phase(ecs_world_t *world);
ecs_entity_t ecs_register_system(ecs_world_t *world, ecs_iter_func func, ecs_query_t *query);
ecs_entity_t ecs_register_system_root(ecs_world_t *world, ecs_iter_func func, ecs_query_t *query);
ecs_entity_t ecs_register_system_phase(
    ecs_world_t *world,
    ecs_iter_func func,
    ecs_entity_t phase,
    ecs_query_t *query
);

#endif
