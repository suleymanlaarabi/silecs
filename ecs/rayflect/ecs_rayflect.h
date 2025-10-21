#ifndef ECS_RAYFLECT_H
    #define ECS_RAYFLECT_H

    #include "../ecs_types.h"
    #include "rayflect/rayflect_types.h"
    #include <rayflect/rayflect_parser.h>

    #define ecs_rayflect_id(name) ecs_struct_##name
    #define ECS_STRUCT(name, ...) \
        typedef struct __VA_ARGS__ name; const char *ecs_rayflect_id(name) = #__VA_ARGS__;

    #define ecs_reflection_struct_id(component) component##__struct
    #define ECS_REGISTER_REFLECTION(world, name){ \
        ecs_struct_t ecs_reflection_struct_id(name); \
        ecs_vec_init(&ecs_reflection_struct_id(name).fields, sizeof(ecs_field_t)); \
        rayflect_parse(&ecs_reflection_struct_id(name), ecs_rayflect_id(name)); \
        ecs_add(world, ecs_id(name), ecs_id(EcsStruct)); \
        ecs_set(world, ecs_id(name), ecs_id(EcsStruct), &ecs_reflection_struct_id(name)); }


typedef ecs_struct_t EcsStruct;
typedef size_t EcsPrimitive;

#define ecs_rayflect_parse_struct(s, d) rayflect_parse(s, d)
#define ecs_rayflect_print_struct(s) rayflect_print(s)
#define ecs_rayflect_free_struct(s) rayflect_free(s)
#define ecs_rayflect_format_struct(s, n) rayflect_format(s, n)
#define ecs_rayflect_set_field(s, i, f, v) rayflect_set_field(s, i, f, v)

ECS_COMPONENT_DECLARE(EcsStruct);
ECS_COMPONENT_DECLARE(EcsPrimitive);

void EcsRayflectModule(ecs_world_t *world);

#endif
