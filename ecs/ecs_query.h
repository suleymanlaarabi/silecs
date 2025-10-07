#ifndef ECS_QUERY_H
#define ECS_QUERY_H

#include "ecs_archetype.h"
#include "ecs_types.h"
#include "ecs_vec.h"
#include <stdint.h>

#define query(...) ((ecs_query_t) __VA_ARGS__)
#define ecs_field(it, component) ((component *) ((ecs_vec_t *) ecs_sparseset_get(&(it)->archetype_p->rows, ecs_id(component).value))->data)

typedef uint32_t EcsQueryId;

typedef enum {
    EcsQueryOperEqual,
    EcsQueryOperNot,
    EcsQueryOperAnd,
    EcsQueryOperOr
} ecs_query_term_oper_t;

#define EcsQueryFlagSingleton 0b00000001

typedef struct {
    ecs_entity_t id;
    ecs_query_term_oper_t oper;
    uint16_t flags;
} ecs_query_term_t;

typedef struct {
    ecs_query_term_t terms[8];
    uint32_t count;
} ecs_query_t;

typedef struct {
    ecs_vec_t archetypes;
    ecs_query_t query;
} ecs_query_cache_t;

typedef struct {
    ecs_world_t *world;
    ecs_vec_t *archetypes;
    ecs_archetype_t *archetype_p;
    uint32_t current_archetype;
    uint32_t count;
} ecs_iter_t;

ECS_COMPONENT_DECLARE(EcsQueryId);

bool ecs_query_match_type(ecs_query_t *query, ecs_type_t *type);
ecs_query_t *ecs_query_from_str(ecs_world_t *world, const char *str);
EcsQueryId ecs_query_register(ecs_world_t *world, ecs_query_t *query);
ecs_iter_t ecs_query_iter(ecs_world_t *world, EcsQueryId query);
bool ecs_iter_next(ecs_iter_t *it);
void EcsQueryModule(ecs_world_t *world);

#endif
