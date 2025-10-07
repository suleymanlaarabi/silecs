#include "ecs_observer.h"
#include <criterion/criterion.h>
#include "ecs_query.h"
#include "ecs_types.h"
#include "ecs_world.h"

typedef struct {} SysEvent;

ECS_COMPONENT_DEFINE(SysEvent);

static int sys_event_call_count = 0;
void OnSysEvent(ecs_world_t *world, ecs_entity_t entity, ecs_iter_t *it) {
    (void) world;
    (void) entity;
    sys_event_call_count += it->count;
}

Test(obserer, simple) {
    ecs_world_t *world = ecs_init();
    ECS_REGISTER_COMPONENT(world, SysEvent);

    ecs_entity_t position = ecs_new(world);

    ecs_entity_t player = ecs_new(world);
    ecs_add(world, player, position);

    ecs_query_t query = query({
        .terms = {
            { position, .oper = EcsQueryOperEqual }
        }
    });

    ecs_observe(world, ecs_id(SysEvent), OnSysEvent, &query);

    cr_assert(sys_event_call_count == 0);

    ecs_observer_trigger(world, ecs_id(SysEvent), player);

    cr_assert(sys_event_call_count == 1);

    ecs_add(world, ecs_new(world), position);

    ecs_observer_trigger(world, ecs_id(SysEvent), player);

    cr_assert(sys_event_call_count == 3);
}
