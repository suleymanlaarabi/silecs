#include "ecs_query.h"
#include "ecs_system.h"
#include "ecs_types.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <ecs_world.h>
#include <string.h>

typedef struct {
    int x, y;
} Position, Velocity;

typedef struct {
    int value;
} Health;

typedef struct {} Jump;

ECS_COMPONENT_DEFINE(Jump);
ECS_COMPONENT_DEFINE(Position);
ECS_COMPONENT_DEFINE(Velocity);
ECS_COMPONENT_DEFINE(Health);
ECS_TAG_DEFINE(MainScene);

void PosVelSys(ecs_iter_t *it) {
    Position *p = ecs_field(it, Position);
    Velocity *v = ecs_field(it, Velocity);

    for (uint32_t i = 0; i < it->count; i++) {
        p[i].x += v[i].x;
        p[i].y += v[i].y;
    }
}

void onPreUpdate() {
    puts("on pre update");
}

void onUpdate() {
    puts("on update");
}

void onPostUpdate() {
    puts("on post update");
}

int main() {
    ecs_world_t *world = ecs_init();

    ECS_REGISTER_COMPONENT(world, Position);
    ECS_REGISTER_COMPONENT(world, Velocity);

    ECS_SYSTEM(world, onPostUpdate, EcsOnPostUpdate, Position, Velocity);
    ECS_SYSTEM(world, onPreUpdate, EcsOnPreUpdate, Position, Velocity);
    ECS_SYSTEM(world, onUpdate, EcsOnUpdate, Position, Velocity);

    ecs_entity_t player = ecs_new(world);
    ecs_add(world, player, ecs_id(Position));
    ecs_add(world, player, ecs_id(Velocity));

    ecs_set(world, player, ecs_id(Position), &(Position) {0, 0});
    ecs_set(world, player, ecs_id(Velocity), &(Velocity) {1, 1});
    ecs_progress(world);
}
