#include "ecs_component_storage.h"
#include "ecs_vec.h"
#include "ecs_world.h"
#include <ecs_observer.h>
#include <stdint.h>

void ecs_observer_init(ecs_observer_t *observer) {
    ecs_vec_init(&observer->hook, sizeof(ecs_observer_call_t));
}

ecs_observer_t ecs_observer_new(void) {
    ecs_observer_t observer;
    ecs_observer_init(&observer);
    return observer;
}

void ecs_observer_fini(ecs_observer_t *observer) {
    ecs_vec_free(&observer->hook);
}

void ecs_observer_trigger(ecs_world_t *world, ecs_entity_t event, ecs_entity_t target) {
    ecs_component_record_t *record = ecs_component_storage_get_component_record(&world->component_storage, event);

    ecs_observer_call_t *calls = record->observer.hook.data;
    uint32_t count = record->observer.hook.count;

    for (uint32_t i = 0; i < count; i++) {
        calls[i](world, target);
    }
}

void ecs_observer_observer(ecs_world_t *world, ecs_entity_t event, ecs_observer_call_t call) {
    ecs_component_record_t *record = ecs_component_storage_get_component_record(&world->component_storage, event);

    ecs_vec_push(&record->observer.hook, call);
}
