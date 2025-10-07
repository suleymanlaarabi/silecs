#include "datastructure/ecs_vec.h"
#include "ecs_types.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "ecs_entity.h"


void ecs_entity_manager_init(ecs_entity_manager_t *manager) {
    ecs_vec_init(&manager->entity_record, sizeof(ecs_entity_record_t));
    ecs_vec_init(&manager->generations, sizeof(uint16_t));
    ecs_vec_init(&manager->available_entity, sizeof(uint32_t));
}

void ecs_entity_manager_fini(ecs_entity_manager_t *manager) {
    ecs_vec_free(&manager->entity_record);
    ecs_vec_free(&manager->generations);
    ecs_vec_free(&manager->available_entity);
}

ecs_entity_t ecs_entity_manager_new(ecs_entity_manager_t *manager) {
    ecs_entity_t entity;

    if (manager->available_entity.count > 0) {
        entity.index = *ECS_VEC_GET_LAST(uint32_t, &manager->available_entity);
        entity.gen = *ECS_VEC_GET(uint16_t, &manager->generations, entity.index);
        ecs_vec_set(&manager->entity_record, entity.index, &(ecs_entity_record_t) {
            .archetype_id = 0
        });
        ecs_vec_remove_last(&manager->available_entity);
        return entity;
    }
    entity.index = manager->generations.count;
    entity.gen = 0;
    ecs_vec_push(&manager->entity_record, &(ecs_entity_record_t) {
        .row = 0,
        .archetype_id = 0
    });
    ecs_vec_push(&manager->generations, &(uint16_t) {0});
    return entity;
}

bool ecs_entity_manager_is_alive(ecs_entity_manager_t *manager, ecs_entity_t e) {
    return e.gen == *ECS_VEC_GET(uint16_t, &manager->generations, e.index);
}

void ecs_entity_manager_kill(ecs_entity_manager_t *manager, uint32_t index) {
    ecs_vec_push(&manager->available_entity, &index);
    uint16_t gen = (*ECS_VEC_GET(uint16_t, &manager->generations, index)) + 1;
    ecs_vec_set(&manager->generations, index, &gen);
}
