#include "ecs_archetype.h"
#include "datastructure/ecs_sparseset.h"
#include "datastructure/ecs_vec.h"
#include "datastructure/ecs_vec_sort.h"
#include "ecs_types.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>


void ecs_archetype_init(ecs_archetype_t *archetype)
{
    ecs_sparseset_init(&archetype->rows, sizeof(ecs_vec_t));
    ecs_sparseset_init(&archetype->add_edge, sizeof(ecs_archetype_id_t));
    ecs_sparseset_init(&archetype->remove_edge, sizeof(ecs_archetype_id_t));
    ecs_vec_init(&archetype->type, sizeof(ecs_entity_t));
    ecs_vec_init(&archetype->entities, sizeof(uint32_t));
}

void ecs_archetype_fini(ecs_archetype_t *archetype)
{
    ecs_vec_t *rows = archetype->rows.dense.data;
    uint32_t count = archetype->rows.dense.count;
    for (uint32_t i = 0; i < count; i++) {
        ecs_vec_free(&rows[i]);
    }
    ecs_sparseset_fini(&archetype->rows);
    ecs_sparseset_fini(&archetype->add_edge);
    ecs_sparseset_fini(&archetype->remove_edge);
    ecs_vec_free(&archetype->type);
    ecs_vec_free(&archetype->entities);
}

void ecs_archetype_add_row(ecs_archetype_t *archetype, ecs_entity_t component, size_t size)
{
    ecs_vec_t col;
    ecs_vec_init(&col, size);
    ecs_sparseset_insert(&archetype->rows, component.value, &col);
    ecs_vec_push(&archetype->type, &component.value);
    ecs_vec_sort_u64(&archetype->type);
}

uint32_t ecs_archetype_add_entity(ecs_archetype_t *archetype, ecs_entity_t entity)
{
    ecs_vec_t *cols = archetype->rows.dense.data;
    size_t cols_len = archetype->rows.dense.count;

    for (size_t i = 0; i < cols_len; i++) {
        ecs_vec_push_zero(&cols[i]);
    }
    ecs_vec_push(&archetype->entities, &entity.index);
    return archetype->entities.count - 1;
}

ecs_archetype_remove_result_t ecs_archetype_remove_entity(ecs_archetype_t *archetype, size_t row)
{
    ecs_archetype_remove_result_t result = {0};
    uint32_t *entities_data = (uint32_t *)archetype->entities.data;

    if (row == archetype->entities.count - 1) {
        result.swapped_entity_new_row = UINT32_MAX;
    } else {
        result.removed_entity_index = entities_data[archetype->entities.count - 1];
        result.swapped_entity_new_row = row;
    }
    ecs_vec_t *cols = (ecs_vec_t *)archetype->rows.dense.data;
    size_t cols_count = archetype->rows.dense.count;

    for (size_t i = 0; i < cols_count; i++) {
        ecs_vec_remove_fast(&cols[i], row);
    }
    ecs_vec_remove_fast(&archetype->entities, row);
    return result;
}

void ecs_archetype_print(ecs_archetype_t *archetype)
{
    printf("Archetype: ");
    ecs_vec_print_type(&archetype->type);
    printf("\n");
}
