#ifndef ecs_entity_t_H
    #define ecs_entity_t_H
    #include "ecs_archetype.h"
    #include "datastructure/ecs_vec.h"
    #include "ecs_types.h"
    #include <stdint.h>
    #include <stdbool.h>
    #define ECS_GET_RECORD(world, e) ECS_VEC_GET(ecs_entity_record_t, &(world)->entity_manager.entity_record, e.index)

typedef struct {
    ecs_archetype_id_t archetype_id;
    uint32_t row;
} ecs_entity_record_t;

typedef struct {
    ecs_vec_t generations;
    ecs_vec_t entity_record;
    ecs_vec_t available_entity;
} ecs_entity_manager_t;

void ecs_entity_manager_init(ecs_entity_manager_t *manager);
ecs_entity_t ecs_entity_manager_new(ecs_entity_manager_t *manager);
bool ecs_entity_manager_is_alive(ecs_entity_manager_t *manager, ecs_entity_t e);
void ecs_entity_manager_kill(ecs_entity_manager_t *manager, uint32_t index);
void ecs_entity_manager_fini(ecs_entity_manager_t *manager);
ecs_entity_t ecs_entity_manager_get_entity(ecs_entity_manager_t *manager, uint32_t index);

#endif
