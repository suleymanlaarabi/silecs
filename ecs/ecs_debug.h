#ifndef ECS_DEBUG_H
    #define ECS_DEBUG_H
    #include <ecs_world.h>

void ecs_print_id(ecs_world_t *world, ecs_entity_t entity);
void ecs_print_type(ecs_world_t *world, ecs_type_t *type);
void ecs_print_entity(ecs_world_t *world, ecs_entity_t entity);

void ecs_print_query(ecs_world_t *world, ecs_query_t *query);
void ecs_print_queryid(ecs_world_t *world, EcsQueryId id);

#endif
