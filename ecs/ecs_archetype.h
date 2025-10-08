#ifndef ECS_ARCHETYPE_H
    #define ECS_ARCHETYPE_H
    #include "datastructure/ecs_sparseset.h"
    #include "datastructure/ecs_vec.h"
    #include "ecs_config.h"
    #include "ecs_types.h"
    #include <stdio.h>
    #include <stddef.h>
    #include <stdint.h>

typedef uint32_t ecs_archetype_id_t;

typedef struct {
    uint32_t removed_entity_index;
    uint32_t swapped_entity_new_row;
} ecs_archetype_remove_result_t;

typedef struct {
    ecs_sparseset_t rows;
    ecs_vec_t entities;
    ecs_type_t type;

    ecs_sparseset_t add_edge;
    ecs_sparseset_t remove_edge;
} ecs_archetype_t;

void ecs_archetype_init(ecs_archetype_t *archetype);
void ecs_archetype_add_row(ecs_archetype_t *archetype, ecs_entity_t component, size_t size);
void ecs_archetype_add_singleton(ecs_archetype_t *archetype, ecs_entity_t component);
uint32_t ecs_archetype_add_entity(ecs_archetype_t *archetype, ecs_entity_t entity);
ecs_archetype_remove_result_t ecs_archetype_remove_entity(ecs_archetype_t *archetype, size_t row);
void ecs_archetype_fini(ecs_archetype_t *archetype);
void ecs_archetype_migrate_same_entity(ecs_archetype_t *src, ecs_archetype_t *dest, size_t row, size_t dest_row);
void ecs_archetype_migrate_right_entity(ecs_archetype_t *src, ecs_archetype_t *dest, size_t row, size_t dest_row);

ECS_INLINE
void *ecs_archetype_get_component(ecs_archetype_t *archetype, size_t row, ecs_entity_t component) {
    return ECS_VEC_GET(void, (ecs_vec_t *) ecs_sparseset_get(&archetype->rows, component.value), row);
}

ECS_INLINE
bool ecs_archetype_has_component(ecs_archetype_t *archetype, ecs_entity_t component) {
    return ecs_sparseset_exists(&archetype->rows, component.value);
}

#endif
