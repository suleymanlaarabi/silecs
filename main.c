#include "ecs_bootstrap.h"
#include "ecs_query.h"
#include "ecs_system.h"
#include "ecs_types.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <ecs_world.h>
#include <string.h>

typedef struct {
    float x, y;
} Position, Velocity;

typedef struct { int hp; } Health;

ECS_COMPONENT_DEFINE(Position);
ECS_COMPONENT_DEFINE(Velocity);
ECS_COMPONENT_DEFINE(Health);
ECS_TAG_DEFINE(MainScene);
ECS_TAG_DEFINE(Window);

void PositionVelocitySystem(ecs_iter_t *it) {
    Position *positions = ecs_field(it, Position);
    Velocity *velocities = ecs_field(it, Velocity);

    for (uint32_t i = 0; i < it->count; i++) {
        positions[i].x += velocities[i].x;
        positions[i].y += velocities[i].y;
    }
}

int main() {
    ecs_world_t *world = ecs_init();

    ECS_REGISTER_COMPONENT(world, Position);
    ECS_REGISTER_COMPONENT(world, Velocity);
    ECS_REGISTER_COMPONENT(world, Health);
    ECS_TAG_REGISTER(world, MainScene);

    ECS_SYSTEM(world,
        PositionVelocitySystem,
        EcsOnUpdate,
        Position, Velocity, (EcsChildOf, MainScene)
    );

    ECS_SYSTEM(world, PositionVelocitySystem, EcsOnUpdate,
        Position,
        Velocity,
        (EcsChildOf, MainScene)
    );

    ecs_entity_t player = ecs_new(world);
    ecs_add(world, player, ecs_id(Position));
    ecs_add(world, player, ecs_id(Velocity));
    ecs_add_pair(world, player, ecs_id(EcsChildOf), ecs_id(MainScene));
    ecs_run_phase(world);
    ecs_fini(world);
}
