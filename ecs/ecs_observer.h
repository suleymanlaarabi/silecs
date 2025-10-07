#ifndef ECS_OBSERVER_H
    #define ECS_OBSERVER_H
    #include "ecs_query.h"
#include "ecs_types.h"
    #include "ecs_vec.h"

typedef void (*ecs_observer_call_t)(ecs_world_t *, ecs_entity_t, ecs_iter_t *);

typedef struct {
    ecs_observer_call_t call;
    EcsQueryId query;
} ecs_observer_record_t;

typedef struct {
    ecs_vec_t hooks; // ecs_observer_record_t
} ecs_observer_t;

void ecs_observer_init(ecs_observer_t *observer);
ecs_observer_t ecs_observer_new(void);

void ecs_observer_fini(ecs_observer_t *observer);
void ecs_observer_trigger(ecs_world_t *world, ecs_entity_t event, ecs_entity_t target);
void ecs_observe(ecs_world_t *world, ecs_entity_t event, ecs_observer_call_t call, ecs_query_t *query);

#endif
