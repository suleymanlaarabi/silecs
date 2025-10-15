#ifndef ECS_RAYFLECT_H
#define ECS_RAYFLECT_H

#include "../ecs_types.h"
#include "rayflect/rayflect_types.h"

#define ecs_rayflect_id(name) ecs_struct_##name
#define ECS_STRUCT(name, ...) \
    typedef struct __VA_ARGS__ name; const char *ecs_rayflect_id(name) = #__VA_ARGS__;

typedef ecs_struct_t EcsStruct;
typedef size_t EcsPrimitive;

#define ecs_rayflect_parse_struct(s, d) rayflect_parse(s, d)
#define ecs_rayflect_print_struct(s) rayflect_print(s)
#define ecs_rayflect_free_struct(s) rayflect_free(s)
#define ecs_rayflect_format_struct(s, n) rayflect_format(s, n)

ECS_COMPONENT_DECLARE(EcsStruct);
ECS_COMPONENT_DECLARE(EcsPrimitive);

void EcsRayflectModule(ecs_world_t *world);

#endif
