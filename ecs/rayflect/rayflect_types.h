#ifndef RAYFLECT_TYPES_H
#define RAYFLECT_TYPES_H

#include "../datastructure/ecs_vec.h"
#include <stddef.h>

typedef enum {
    ECS_TYPE_UNKNOWN = 0,
    ECS_TYPE_I8,
    ECS_TYPE_I16,
    ECS_TYPE_I32,
    ECS_TYPE_I64,
    ECS_TYPE_U8,
    ECS_TYPE_U16,
    ECS_TYPE_U32,
    ECS_TYPE_U64,
    ECS_TYPE_F32,
    ECS_TYPE_F64,
    ECS_TYPE_BOOL,
    ECS_TYPE_PTR,
    ECS_TYPE_ARRAY,
    ECS_TYPE_CUSTOM
} ecs_simple_type_t;

typedef struct {
    const char *name;
    ecs_simple_type_t type;
    int array_size;
    size_t size;
    size_t align;
} ecs_field_t;

typedef struct {
    ecs_vec_t fields;
} ecs_struct_t;

size_t rayflect_type_size(const char *type_str);
size_t rayflect_type_align(const char *type_str);
ecs_simple_type_t rayflect_type_simplify(const char *c_type, int *out_array_size);
const char* rayflect_type_to_string(ecs_simple_type_t type, int array_size);
int rayflect_set_field(const ecs_struct_t *ecs_struct, void *instance, const char *field_name, const void *value);

#endif
