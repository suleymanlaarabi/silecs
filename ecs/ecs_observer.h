#ifndef ECS_OBSERVER_H
    #define ECS_OBSERVER_H
    #include "ecs_types.h"
    #include "ecs_vec.h"

typedef void (*ecs_observer_call_t)(ecs_world_t *world, ecs_entity_t entity);

typedef struct {
    ecs_vec_t hook; // ecs_observer_call_t
} ecs_observer_t;


void ecs_observer_init(ecs_observer_t *observer);
ecs_observer_t ecs_observer_new(void);

void ecs_observer_fini(ecs_observer_t *observer);
void ecs_observer_trigger(ecs_world_t *world, ecs_entity_t event, ecs_entity_t target);
void ecs_observer_observer(ecs_world_t *world, ecs_entity_t event, ecs_observer_call_t call);

#endif
