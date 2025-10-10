#include "ecs_query.h"
#include "ecs_system.h"
#include "ecs_types.h"
#include "ecs_world.h"
#include "test.h"
#include <criterion/criterion.h>
#include <stdint.h>
#include <stdio.h>

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

    ecs_progress(world);

    cr_assert(medium_count == 2);
}

void PosVelSys(ecs_iter_t *it) {
    Position *p = ecs_field(it, Position);
    Velocity *v = ecs_field(it, Velocity);

    for (int i = 0; i < it->count; i++) {
        p[i].x += v[i].x;
        p[i].y += v[i].y;
    }
}

Test(system, pos_vel) {
    ecs_world_t *world = ecs_init();

    ECS_REGISTER_COMPONENT(world, Position);
    ECS_REGISTER_COMPONENT(world, Velocity);

    ecs_entity_t player = ecs_new(world);
    ecs_add(world, player, ecs_id(Position));
    ecs_add(world, player, ecs_id(Velocity));

    ecs_set(world, player, ecs_id(Position), &(Position) {0, 0});
    ecs_set(world, player, ecs_id(Velocity), &(Velocity) {1, 1});

    ECS_SYSTEM(world, PosVelSys, EcsOnUpdate, Position, Velocity);

    Position *pos = ecs_get(world, player, ecs_id(Position));

    cr_assert_eq(pos->x, 0);
    cr_assert_eq(pos->y, 0);
    ecs_progress(world);
    cr_assert_eq(pos->x, 1);
    cr_assert_eq(pos->y, 1);
}


Test(system, killed) {
    ecs_world_t *world = ecs_init();

    ECS_REGISTER_COMPONENT(world, Position);
    ECS_REGISTER_COMPONENT(world, Velocity);

    ecs_entity_t player = ecs_new(world);
    ecs_add(world, player, ecs_id(Position));
    ecs_add(world, player, ecs_id(Velocity));

    ecs_set(world, player, ecs_id(Position), &(Position) {0, 0});
    ecs_set(world, player, ecs_id(Velocity), &(Velocity) {1, 1});

    ECS_SYSTEM(world, PosVelSys, EcsOnUpdate, Position, Velocity);

    Position *pos = ecs_get(world, player, ecs_id(Position));

    cr_assert_eq(pos->x, 0);
    cr_assert_eq(pos->y, 0);
    ecs_kill(world, player);
    ecs_progress(world);
    cr_assert_eq(pos->x, 0);
    cr_assert_eq(pos->y, 0);
}

void RelatedPosVelSys(ecs_iter_t *it) {
    Position *p = ecs_field(it, Position);
    Velocity *v = ecs_field(it, Velocity);

    for (int i = 0; i < it->count; i++) {
        p[i].x += v[i].x;
        p[i].y += v[i].y;
    }
}

Test(system, relation) {
    ecs_world_t *world = ecs_init();

    ECS_REGISTER_COMPONENT(world, Position);
    ECS_REGISTER_COMPONENT(world, Velocity);
    ECS_REGISTER_COMPONENT(world, MainScene);

    ecs_entity_t player = ecs_new(world);
    ecs_add(world, player, ecs_id(Position));
    ecs_add(world, player, ecs_id(Velocity));

    ecs_set(world, player, ecs_id(Position), &(Position) {0, 0});
    ecs_set(world, player, ecs_id(Velocity), &(Velocity) {1, 1});

    ECS_SYSTEM(world, RelatedPosVelSys, EcsOnUpdate, Position, Velocity, (ChildOf, MainScene));

    Position *pos = ecs_get(world, player, ecs_id(Position));
    cr_assert_eq(pos->x, 0);
    cr_assert_eq(pos->y, 0);

    ecs_progress(world);
    pos = ecs_get(world, player, ecs_id(Position));

    cr_assert_eq(pos->x, 0);
    cr_assert_eq(pos->y, 0);
    ecs_add_pair(world, player, ecs_id(EcsChildOf), ecs_id(MainScene));

    ecs_progress(world);
    pos = ecs_get(world, player, ecs_id(Position));

    cr_assert_eq(pos->x, 1);
    cr_assert_eq(pos->y, 1);
}
