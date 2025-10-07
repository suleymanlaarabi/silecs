#include "ecs_component_storage.h"
#include "ecs_query.h"
#include "ecs_vec.h"
#include "ecs_world.h"
#include <ecs_observer.h>
#include <stdint.h>
#include <stdio.h>

void ecs_observer_init(ecs_observer_t *observer) {
    ecs_vec_init(&observer->hooks, sizeof(ecs_observer_record_t));
}

ecs_observer_t ecs_observer_new(void) {
    ecs_observer_t observer;
    ecs_observer_init(&observer);
    return observer;
}

void ecs_observer_fini(ecs_observer_t *observer) {
    ecs_vec_free(&observer->hooks);
}

void ecs_observer_trigger(ecs_world_t *world, ecs_entity_t event, ecs_entity_t target) {
    ecs_component_record_t *record = ecs_component_storage_get_component_record(&world->component_storage, event);

    ecs_observer_record_t *records = record->observer.hooks.data;
    uint32_t count = record->observer.hooks.count;


    for (uint32_t i = 0; i < count; i++) {
        EcsQueryId query = records[i].query;
        ecs_iter_t it = ecs_query_iter(world, query);
        while (ecs_iter_next(&it)) {
            records[i].call(world, target, &it);
        }
    }
}

void ecs_observe(ecs_world_t *world, ecs_entity_t event, ecs_observer_call_t call, ecs_query_t *query) {
    ecs_component_record_t *record = ecs_component_storage_get_component_record(&world->component_storage, event);
    ecs_observer_record_t observer_record = {
        .call = call,
        .query = ecs_query_register(world, query)
    };
    ecs_vec_push(&record->observer.hooks, &observer_record);
}
