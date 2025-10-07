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
void ecs_archetype_print(ecs_archetype_t *archetype);
void ecs_archetype_fini(ecs_archetype_t *archetype);

ECS_INLINE
bool ecs_archetype_has_component(ecs_archetype_t *archetype, ecs_entity_t component) {
    return ecs_sparseset_exists(&archetype->rows, component.value);
}

ECS_INLINE
void ecs_archetype_migrate_entity(ecs_archetype_t *src, ecs_archetype_t *dest, size_t row, size_t dest_row) {
    ecs_entity_t *components = src->type.data;
    int len = src->type.count;

    for (int i = 0; i < len; i++) {
        ecs_entity_t component = components[i];
        if (ecs_archetype_has_component(dest, component)) {
            ecs_vec_copy_element(
                ecs_sparseset_get(&src->rows, component.value),
                ecs_sparseset_get(&dest->rows, component.value),
                row, dest_row
            );
        }
    }
}

ECS_INLINE
void ecs_archetype_migrate_same_entity(ecs_archetype_t *src, ecs_archetype_t *dest, size_t row, size_t dest_row) {
    ecs_vec_t *src_rows = src->rows.dense.data;
    ecs_vec_t *dest_rows = dest->rows.dense.data;
    int len = src->rows.dense.count;

    for (int i = 0; i < len; i++) {
        ecs_vec_copy_element(
            &src_rows[i],
            &dest_rows[i],
            row, dest_row
        );
    }
}

ECS_INLINE
void ecs_archetype_migrate_right_entity(ecs_archetype_t *src, ecs_archetype_t *dest, size_t row, size_t dest_row) {
    int len = dest->rows.dense.count;
    ecs_vec_t *dest_rows = dest->rows.dense.data;
    ecs_vec_t *src_rows = src->rows.dense.data;

    ecs_entity_t *src_type = src->type.data;
    ecs_entity_t *dest_type = dest->type.data;

    for (int src_i = 0, dest_i = 0; dest_i < len;) {
        if (src_type[src_i].value == dest_type[dest_i].value) {
            ecs_vec_copy_element(
                &src_rows[src_i],
                &dest_rows[dest_i],
                row, dest_row
            );
            src_i++;
            dest_i++;
        } else {
            src_i++;
        }
    }
}

ECS_INLINE
void *ecs_archetype_get_component(ecs_archetype_t *archetype, size_t row, ecs_entity_t component) {
    return ECS_VEC_GET(void, (ecs_vec_t *) ecs_sparseset_get(&archetype->rows, component.value), row);
}

#endif
