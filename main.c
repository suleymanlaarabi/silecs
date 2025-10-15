#include "addons/stdio_devtool.h"
#include "ecs_module.h"
#include "ecs_query.h"
#include "ecs_system.h"
#include "ecs_types.h"
#include "rayflect/ecs_rayflect.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <ecs_world.h>
#include <string.h>

ECS_STRUCT(Position, {
    float x;
    float y;
});
ECS_STRUCT(Velocity, {
    float x;
    float y;
});

typedef struct {
    int value;
} Health;

ECS_TAGS(
    Jump,
    MainScene
);

ECS_COMPONENT_DEFINE(Jump);
ECS_COMPONENT_DEFINE(Position);
ECS_COMPONENT_DEFINE(Velocity);
ECS_COMPONENT_DEFINE(Health);
ECS_COMPONENT_DEFINE(MainScene);

void PosVelSys(ecs_iter_t *it) {
    Position *p = ecs_field(it, Position);
    Velocity *v = ecs_field(it, Velocity);

    for (int i = 0; i < it->count; i++) {
        p[i].x += v[i].x;
        p[i].y += v[i].y;
    }
}

void onPreUpdate(ecs_iter_t *_) {
    puts("on pre update");
}

void onUpdate(ecs_iter_t *_) {
    puts("on update");
}

void onPostUpdate(ecs_iter_t *_) {
    puts("on post update");
}

int main() {
    ecs_world_t *world = ecs_init();

    ECS_IMPORT(world, EcsStdioInspectorModule);

    ECS_REGISTER_COMPONENT(world, Position);
    ECS_REGISTER_COMPONENT(world, Velocity);
    ECS_REGISTER_COMPONENT(world, MainScene);

    ECS_REGISTER_REFLECTION(world, Position);
    ECS_REGISTER_REFLECTION(world, Velocity);

    ECS_SYSTEM(world, onPostUpdate, EcsOnPostUpdate, Position, Velocity);
    ECS_SYSTEM(world, onPreUpdate, EcsOnPreUpdate, Position, Velocity);
    ECS_SYSTEM(world, onUpdate, EcsOnUpdate, Position, Velocity);

    ecs_entity_t player = ecs_new(world);
    ecs_add(world, player, ecs_id(Position));
    ecs_add(world, player, ecs_id(Velocity));
    ecs_add_pair(world, player, ecs_id(EcsChildOf), ecs_id(MainScene));

    ecs_set(world, player, ecs_id(Position), &(Position) {0, 0});
    ecs_set(world, player, ecs_id(Velocity), &(Velocity) {1, 1});

    ecs_progress(world);
}
