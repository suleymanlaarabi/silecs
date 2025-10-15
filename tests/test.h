#ifndef ECS_TEST_H
    #define ECS_TEST_H
    #include "ecs_types.h"
    #include <ecs_world.h>

typedef struct {
    int x, y;
} Position, Velocity;
typedef struct {
    int value;
} Health;
typedef struct {} Jump;
typedef struct {} MainScene;

ECS_COMPONENT_DECLARE(Jump);
ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(Velocity);
ECS_COMPONENT_DECLARE(Health);
ECS_COMPONENT_DECLARE(MainScene);
ECS_COMPONENT_DECLARE(MainScene);

#endif
