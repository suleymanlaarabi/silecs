#ifndef ECS_COMPONENT_STORAGE_H
    #define ECS_COMPONENT_STORAGE_H
    #include "ecs_archetype.h"
    #include "ecs_config.h"
    #include "ecs_observer.h"
    #include "ecs_sparseset.h"
    #include "ecs_types.h"
    #include "ecs_vec.h"
    #include <stddef.h>
#include <stdint.h>

typedef void (*ecs_component_hook_call)(ecs_world_t *, ecs_entity_t);

typedef struct {
    size_t size;
    ecs_vec_t archetypes;
    ecs_component_hook_call add_hook; // ecs_component_hook_call
    ecs_component_hook_call remove_hook; // ecs_component_hook_call
    ecs_component_hook_call set_hook; // ecs_component_hook_call
    ecs_observer_t observer;
} ecs_component_record_t;

typedef struct {
    ecs_sparseset_t component_meta; // <ecs_entity_t, ecs_component_record_t>
} ecs_component_storage_t;

ECS_INLINE
void ecs_component_storage_init(ecs_component_storage_t *storage) {
    ecs_sparseset_init(&storage->component_meta, sizeof(ecs_component_record_t));
}

ECS_INLINE
void ecs_component_storage_fini(ecs_component_storage_t *storage) {
    ecs_component_record_t *records = storage->component_meta.dense.data;
    uint32_t count = storage->component_meta.dense.count;
    for (uint32_t i = 0; i < count; i ++) {
        ecs_component_record_t *record = &records[i];
        ecs_vec_free(&record->archetypes);
        ecs_observer_fini(&record->observer);
    }
    ecs_sparseset_fini(&storage->component_meta);
}

ECS_INLINE
void ecs_component_storage_set(ecs_component_storage_t *storage, ecs_entity_t entity, size_t size) {
    ecs_sparseset_insert(&storage->component_meta, entity.value, &(ecs_component_record_t) {
        .size = size,
        .archetypes = ecs_vec_create(sizeof(ecs_archetype_id_t)),
        .add_hook = NULL,
        .remove_hook = NULL,
        .set_hook = NULL,
        .observer = ecs_observer_new()
    });
}

ECS_INLINE
size_t ecs_component_storage_get_component_size(ecs_component_storage_t *storage, ecs_entity_t entity) {
    ecs_component_record_t *meta = ecs_sparseset_get(&storage->component_meta, entity.value);
    return meta ? meta->size : 0;
}

ECS_INLINE
ecs_component_record_t *ecs_component_storage_get_component_record(ecs_component_storage_t *storage, ecs_entity_t entity) {
    return ecs_sparseset_get(&storage->component_meta, entity.value);
}

#endif
