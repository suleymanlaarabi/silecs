#ifndef ECS_WORLD_H
    #define ECS_WORLD_H
    #include "ecs_archetype.h"
    #include "ecs_component_storage.h"
    #include "ecs_config.h"
    #include "ecs_entity.h"
    #include "ecs_query.h"
    #include "ecs_sparseset.h"
    #include "ecs_types.h"
    #include "ecs_vec.h"
    #include "ecs_map.h"
    #include "ecs_bootstrap.h"
    #include "ecs_strmap.h"
    #include <stdio.h>
    #include <stddef.h>
    #define ecs_world_get_archetype_by_id(world, archetype_id) ECS_VEC_GET(ecs_archetype_t, &world->archetypes, archetype_id)
    #define ecs_world_get_entity_archetype(world, entity) ecs_world_get_archetype(world, ecs_world_get_record(world, entity)->archetype_id)
    #define ecs_world_get_record_by_index(world, index) ECS_VEC_GET(ecs_entity_record_t, &world->entity_manager.entity_record, index)
    #define ecs_world_get_record(world, entity) ecs_world_get_record_by_index(world, entity.index)
    #define ecs_world_get_default_archetype(world) ECS_VEC_GET(ecs_archetype_t, &world->archetypes, 0)
    #define ecs_singleton(world, entity) ecs_add(world, entity, entity)
    #define ecs_singleton_set(world, entity, value) ecs_set(world, entity, entity, value)
    #define ecs_singleton_get(world, entity) ecs_get(world, entity, entity)
    #define ecs_component_get_record(world, entity) ecs_component_storage_get_component_record(&world->component_storage, entity);

typedef struct ecs_world_t {
    ecs_entity_manager_t entity_manager;
    ecs_vec_t archetypes;
    ecs_hashmap_t archetype_map;
    ecs_component_storage_t component_storage;
    ecs_vec_t queries;
    EcsQueryId phases_query;
    ecs_strmap_t entity_map;
    ecs_sparseset_t component_archetypes; // ecs_vec<ecs_archetype_id>
} ecs_world_t;

typedef char* EcsName;

ECS_COMPONENT_DECLARE(EcsName);

ecs_world_t *ecs_init(void);
void ecs_add(ecs_world_t *world, ecs_entity_t entity, ecs_entity_t component);
void ecs_remove(ecs_world_t *world, ecs_entity_t entity, ecs_entity_t component);
ecs_archetype_id_t ecs_archetype_create(ecs_world_t *world, ecs_type_t *type);
void ecs_add_pair(ecs_world_t *world, ecs_entity_t source, ecs_entity_t relation, ecs_entity_t target) ;
void ecs_remove_pair(ecs_world_t *world, ecs_entity_t source, ecs_entity_t relation, ecs_entity_t target);
void ecs_remove_hook(ecs_world_t *world, ecs_entity_t component, ecs_component_hook_call call);
void ecs_add_hook(ecs_world_t *world, ecs_entity_t component, ecs_component_hook_call call);
void ecs_set_hook(ecs_world_t *world, ecs_entity_t component, ecs_component_hook_call call);
void ecs_kill(ecs_world_t *world, ecs_entity_t entity);
void ecs_fini(ecs_world_t *world);

ECS_INLINE
ecs_archetype_t *ecs_world_get_archetype(ecs_world_t *world, ecs_archetype_id_t id) {
    return ECS_VEC_GET(ecs_archetype_t, &world->archetypes, id);
}

ECS_INLINE
ecs_archetype_id_t ecs_archetype_get_or_create(ecs_world_t *world, ecs_type_t *type) {
    uint64_t archetype_id;
    if (ecs_hashmap_get(&world->archetype_map, type, &archetype_id)) {
        return archetype_id;
    }
    return ecs_archetype_create(world, type);
}

ECS_INLINE
ecs_entity_t ecs_new(ecs_world_t *world) {
    ecs_entity_t entity = ecs_entity_manager_new(&world->entity_manager);

    ecs_archetype_t *archetype = ecs_world_get_default_archetype(world);
    ecs_archetype_add_entity(archetype, entity);
    return entity;
}

ECS_INLINE
void ecs_world_handle_archetype_remove(ecs_world_t *world, ecs_archetype_remove_result_t remove_result) {

    if (remove_result.swapped_entity_new_row == UINT32_MAX) {
        return;
    }

    ecs_entity_record_t *record = ecs_world_get_record_by_index(world, remove_result.removed_entity_index);
    record->row = remove_result.swapped_entity_new_row;
}

ECS_INLINE
void ecs_set_component_meta(ecs_world_t *world, ecs_entity_t component, size_t size) {
    ecs_component_storage_set(&world->component_storage, component, size);
}

ECS_INLINE
void *ecs_get(ecs_world_t *world, ecs_entity_t entity, ecs_entity_t component) {
    ecs_entity_record_t *record = ecs_world_get_record(world, entity);
    ecs_archetype_t *archetype = ecs_world_get_archetype(world, record->archetype_id);

    return ecs_archetype_get_component(archetype, record->row, component);
}

ECS_INLINE
void ecs_set(ecs_world_t *world, ecs_entity_t entity, ecs_entity_t component, void *value) {
    void *component_p = ecs_get(world, entity, component);
    ecs_component_record_t *component_record = ecs_component_get_record(world, component);

    memcpy(component_p, value, component_record->size);
    if (component_record != NULL && component_record->set_hook != NULL) {
        component_record->set_hook(world, entity);
    }
}

ECS_INLINE
void ecs_insert(ecs_world_t *world, ecs_entity_t entity, ecs_entity_t component, void *value) {
    ecs_add(world, entity, component);
    ecs_set(world, entity, component, value);
}

ECS_INLINE
bool ecs_has(ecs_world_t *world, ecs_entity_t entity, ecs_entity_t component) {
    ecs_entity_record_t *record = ecs_world_get_record(world, entity);
    ecs_archetype_t *archetype = ecs_world_get_archetype(world, record->archetype_id);

    return ecs_archetype_has_component(archetype, component);
}

#define ECS_PAIR 0x0000000000000001
#define ecs_is_pair(entity) ((entity.flags & ECS_PAIR) == ECS_PAIR)
#define ecs_pair_target(entity) (ecs_entity_t) { .index = entity.relation.target, .gen = 0 }
#define ecs_pair_relation(entity) (ecs_entity_t) { .index = entity.relation.relation, .gen = 0 }

ECS_INLINE
ecs_entity_t ecs_make_pair(ecs_entity_t relation, ecs_entity_t target) {
    ecs_entity_t pair = {
        .relation = {
            .relation = relation.index,
            .target = target.index
        },
    };
    pair.flags |= ECS_PAIR;
    return pair;
}

ECS_INLINE
bool ecs_has_pair(ecs_world_t *world, ecs_entity_t entity, ecs_entity_t relation, ecs_entity_t target) {
    return ecs_has(world, entity, ecs_make_pair(relation, target));
}

ECS_INLINE
ecs_entity_t ecs_lookup(ecs_world_t *world, const char *name) {
    ecs_entity_t value = ecs_strmap_get(&world->entity_map, name);

    return value;
}

#endif
