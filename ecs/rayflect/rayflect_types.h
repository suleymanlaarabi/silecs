#ifndef RAYFLECT_TYPES_H
#define RAYFLECT_TYPES_H

#include "../datastructure/ecs_vec.h"
#include <stddef.h>

typedef struct {
    const char *name;
    const char *type;
    const char *simple_type;
    size_t size;
    size_t align;
} ecs_field_t;

typedef struct {
    ecs_vec_t fields;
} ecs_struct_t;

size_t rayflect_type_size(const char *type_str);
size_t rayflect_type_align(const char *type_str);
char* rayflect_type_simplify(const char *c_type);
const char* rayflect_type_simplify_static(const char *c_type);

#endif
