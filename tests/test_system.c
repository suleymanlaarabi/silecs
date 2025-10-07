#include "ecs_query.h"
#include "ecs_system.h"
#include "ecs_types.h"
#include "ecs_world.h"
#include <criterion/criterion.h>

static int call_count = 0;
void simple_system(ecs_iter_t *it) {
    (void)it;
    call_count++;
}

Test(system, simple) {
    ecs_world_t *world = ecs_init();

    ecs_entity_t player = ecs_new(world);

    ecs_query_t q = query({
        .terms = { { player } },
    });
    ecs_entity_t my_sys = ecs_register_system(world, simple_system, &q);

    ecs_entity_t online_player = ecs_new(world);
    ecs_add(world, online_player, player);

    ecs_run_phase(world);

    // the system doesn't have EcsPhase (is not running)
    cr_assert(call_count == 0);

    ecs_entity_t FirstPhase = ecs_new(world);
    ecs_add_pair(world, my_sys, ecs_id(EcsPhase), FirstPhase);

    ecs_run_phase(world);

    cr_assert(call_count == 1);

    ecs_run_phase(world);

    cr_assert(call_count == 2);
}

static int medium_count = 0;
void medium_system(ecs_iter_t *it) {
    medium_count += it->count;
}

ECS_TAG_DEFINE(SysEnemy);
ECS_TAG_DEFINE(SysPlayer);

Test(system, medium) {
    ecs_world_t *world = ecs_init();

    ECS_TAG_REGISTER(world, SysEnemy);
    ECS_TAG_REGISTER(world, SysPlayer);

    ECS_SYSTEM(world, medium_system, EcsOnUpdate, SysEnemy, !SysPlayer);

    ecs_entity_t enemy = ecs_new(world);
    ecs_add(world, enemy, ecs_id(SysEnemy));

    ecs_entity_t enemy2 = ecs_new(world);
    ecs_add(world, enemy2, ecs_id(SysEnemy));

    ecs_entity_t enemy3 = ecs_new(world);
    ecs_add(world, enemy3, ecs_id(SysEnemy));
    ecs_add(world, enemy3, ecs_id(SysPlayer));

    ecs_run_phase(world);

    cr_assert(medium_count == 2);
}
