#include "ecs_archetype.h"
#include "ecs_bootstrap.h"
#include "ecs_component_storage.h"
#include "ecs_config.h"
#include "ecs_entity.h"
#include "ecs_map.h"
#include "ecs_query.h"
#include "ecs_sparseset.h"
#include "ecs_strmap.h"
#include "ecs_system.h"
#include "ecs_types.h"
#include "ecs_vec.h"
#include "ecs_module.h"
#include <ecs_world.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

ECS_COMPONENT_DEFINE(EcsName);

ecs_world_t *ecs_init(void) {
    ecs_world_t *world = malloc(sizeof(ecs_world_t));
    ecs_type_t default_type = ECS_VEC_RAW(ecs_entity_t);

    ecs_vec_init(&world->archetypes, sizeof(ecs_archetype_t));
    ecs_vec_init(&world->queries, sizeof(ecs_query_cache_t));
    ecs_strmap_init(&world->entity_map, 1000);
    ecs_entity_manager_init(&world->entity_manager);
    ecs_hashmap_init(&world->archetype_map);
    ecs_component_storage_init(&world->component_storage);
    ecs_archetype_create(world, &default_type);
    ecs_sparseset_init(&world->component_archetypes, sizeof(ecs_vec_t));

    ecs_new(world);

    ECS_REGISTER_COMPONENT(world, EcsName);
    ecs_init_module(world);
    ECS_IMPORT(world, EcsBootstrapModule);
    ECS_IMPORT(world, EcsQueryModule);
    ECS_IMPORT(world, EcsSystemModule);
    return world;
}

void ecs_fini(ecs_world_t *world) {
    ecs_archetype_t *archetypes = world->archetypes.data;
    uint32_t archetype_count = world->archetypes.count;

    for (uint32_t i = 0; i < archetype_count; i++) {
        ecs_archetype_fini(&archetypes[i]);
    }
    ecs_vec_free(&world->archetypes);
    ecs_query_cache_t *queries = world->queries.data;
    uint32_t query_count = world->queries.count;

    for (uint32_t i = 0; i < query_count; i++) {
        ecs_vec_free(&queries[i].archetypes);
    }
    ecs_vec_free(&world->queries);

    ecs_strmap_destroy(&world->entity_map);

    ecs_entity_manager_fini(&world->entity_manager);

    ecs_hashmap_fini(&world->archetype_map);

    ecs_component_storage_fini(&world->component_storage);

    ecs_vec_t *component_archetypes = world->component_archetypes.dense.data;
    uint32_t component_archetype_count = world->component_archetypes.dense.count;

    for (uint32_t i = 0; i < component_archetype_count; i++) {
        ecs_vec_free(&component_archetypes[i]);
    }
    ecs_sparseset_fini(&world->component_archetypes);

    free(world);
}

ecs_archetype_id_t ecs_archetype_create(ecs_world_t *world, ecs_type_t *type) {
    ecs_archetype_id_t id = world->archetypes.count;
    ecs_hashmap_put(&world->archetype_map, type, id);

    ecs_archetype_t *archetype = ecs_vec_add(&world->archetypes);
    ecs_archetype_init(archetype);

    for (uint32_t i = 0; i < type->count; i++) {
        ecs_entity_t component = *ECS_VEC_GET(ecs_entity_t, type, i);
        ecs_archetype_add_row(
            archetype,
            component,
            ecs_component_storage_get_component_size(&world->component_storage, component)
        );

        ecs_vec_t *component_archetypes = ecs_sparseset_get(&world->component_archetypes, component.value);
        if (component_archetypes) {
            ecs_vec_push(component_archetypes, &id);
        }
    }

    ecs_query_cache_t *queries = world->queries.data;
    uint32_t len = world->queries.count;

    for (uint32_t i = 0; i < len; i++) {
        if (ecs_query_match_type(&queries[i].query, type)) {
            ecs_vec_push(&queries[i].archetypes, &id);
        }
    }

    return id;
}

void ecs_remove_entity_from_archetype(
    ecs_world_t *world,
    ecs_archetype_t *archetype,
    ecs_entity_record_t *record,
    ecs_archetype_id_t new_archetype_id,
    size_t new_row
) {
    ecs_archetype_remove_result_t remove_result = ecs_archetype_remove_entity(archetype, record->row);
    ecs_world_handle_archetype_remove(world, remove_result);

    record->archetype_id = new_archetype_id;
    record->row = new_row;
}

ECS_INLINE
void ecs_world_migrate_remove_entity(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_record_t *record,
    ecs_archetype_id_t new_archetype_id
) {
    ecs_archetype_t *new_archetype = ecs_world_get_archetype(world, new_archetype_id);
    size_t new_row = ecs_archetype_add_entity(new_archetype, entity);
    ecs_archetype_t *archetype = ecs_world_get_archetype(world, record->archetype_id);

    ecs_archetype_migrate_right_entity(archetype, new_archetype, record->row, new_row);

    ecs_remove_entity_from_archetype(world, archetype, record, new_archetype_id, new_row);
}


ECS_INLINE
void ecs_world_migrate_add_entity(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_record_t *record,
    ecs_archetype_id_t new_archetype_id
) {
    ecs_archetype_t *new_archetype = ecs_world_get_archetype(world, new_archetype_id);
    size_t new_row = ecs_archetype_add_entity(new_archetype, entity);
    ecs_archetype_t *archetype = ecs_world_get_archetype(world, record->archetype_id);

    ecs_archetype_migrate_same_entity(archetype, new_archetype, record->row, new_row);

    ecs_remove_entity_from_archetype(world, archetype, record, new_archetype_id, new_row);
}

void ecs_add(ecs_world_t *world, ecs_entity_t entity, ecs_entity_t component) {
    ecs_entity_record_t *record = ecs_world_get_record(world, entity);
    ecs_archetype_t *archetype = ecs_world_get_archetype(world, record->archetype_id);

    if (ECS_UNLIKELY(ecs_archetype_has_component(archetype, component))) {
        return;
    }

    ecs_archetype_id_t *cached_archetype = ((ecs_archetype_id_t *) ecs_sparseset_get(&archetype->add_edge, component.value));
    ecs_archetype_id_t new_archetype_id = 0;

    if (cached_archetype == NULL) {
        ecs_type_t *type = ecs_type_from_other_add_temp(&archetype->type, component);

        new_archetype_id = ecs_archetype_get_or_create(world, type);
        // refresh because the archetype may have reallocated
        archetype = ecs_world_get_archetype(world, record->archetype_id);
        ecs_sparseset_insert(&archetype->add_edge, component.value, &new_archetype_id);
        ecs_sparseset_insert(&ecs_world_get_archetype(world, new_archetype_id)->remove_edge, component.value, &record->archetype_id);
    } else {
        new_archetype_id = *cached_archetype;
    }


    ecs_world_migrate_add_entity(world, entity, record, new_archetype_id);

    ecs_component_record_t *component_record = ecs_component_get_record(world, component);
    if (component_record && component_record->add_hook) {
        component_record->add_hook(world, entity);
    }
}

void ecs_add_hook(ecs_world_t *world, ecs_entity_t component, ecs_component_hook_call call) {
    ecs_component_record_t *component_record = ecs_component_get_record(world, component);
    component_record->add_hook = call;
}

void ecs_remove_hook(ecs_world_t *world, ecs_entity_t component, ecs_component_hook_call call) {
    ecs_component_record_t *component_record = ecs_component_get_record(world, component);
    component_record->remove_hook = call;
}

void ecs_set_hook(ecs_world_t *world, ecs_entity_t component, ecs_component_hook_call call) {
    ecs_component_record_t *component_record = ecs_component_get_record(world, component);
    component_record->set_hook = call;
}

void ecs_remove(ecs_world_t *world, ecs_entity_t entity, ecs_entity_t component) {
    ecs_entity_record_t *record = ecs_world_get_record(world, entity);
    ecs_archetype_t *archetype = ecs_world_get_archetype(world, record->archetype_id);

    ecs_archetype_id_t *cached_archetype = ((ecs_archetype_id_t *) ecs_sparseset_get(&archetype->remove_edge, component.value));
    ecs_archetype_id_t new_archetype_id = 0;

    if (cached_archetype == NULL) {
        ecs_type_t *type = ecs_type_from_other_remove_temp(&archetype->type, component);
        new_archetype_id = ecs_archetype_get_or_create(world, type);
        // refresh because the archetype may have reallocated
        archetype = ecs_world_get_archetype(world, record->archetype_id);
        ecs_sparseset_insert(&archetype->remove_edge, component.value, &new_archetype_id);
        ecs_sparseset_insert(&ecs_world_get_archetype(world, new_archetype_id)->add_edge, component.value, &record->archetype_id);
    } else {
        new_archetype_id = *cached_archetype;
    }

    ecs_world_migrate_remove_entity(world, entity, record, new_archetype_id);
    ecs_component_record_t *component_record = ecs_component_get_record(world, component);
    if (component_record && component_record->remove_hook) {
        component_record->remove_hook(world, entity);
    }
}

void ecs_add_pair(ecs_world_t *world, ecs_entity_t source, ecs_entity_t relation, ecs_entity_t target) {
    ecs_add(world, source,
        ecs_make_pair(relation, target)
    );
    ecs_add(world, source,
        ecs_make_pair(relation, ecs_id(EcsWildcard))
    );
}

void ecs_remove_pair(ecs_world_t *world, ecs_entity_t source, ecs_entity_t relation, ecs_entity_t target) {
    ecs_remove(world, source, ecs_make_pair(relation, target));

    ecs_type_t *type = &ecs_world_get_entity_archetype(world, source)->type;

    iter_vec(ecs_entity_t, type) {
        if (iter_value.index == relation.index && iter_value.relation.target != ecs_id(EcsWildcard).index) {
            return;
        }
    }
    ecs_remove(world, source, ecs_make_pair(relation, ecs_id(EcsWildcard)));
}

bool ecs_is_alive(ecs_world_t *world, ecs_entity_t entity) {
    return ecs_entity_manager_is_alive(&world->entity_manager, entity);
}

void ecs_kill(ecs_world_t *world, ecs_entity_t entity) {
    ecs_entity_record_t *record = ECS_GET_RECORD(world, entity);
    ecs_archetype_remove_entity(ecs_world_get_archetype(world, record->archetype_id), record->row);
    ecs_entity_manager_kill(&world->entity_manager, entity.index);
}
