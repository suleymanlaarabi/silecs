#ifndef RAYFLECT_FORMAT_H
#define RAYFLECT_FORMAT_H

#include "rayflect_types.h"
#include "../datastructure/ecs_string.h"

void rayflect_print(const ecs_struct_t *ecs_struct);
ecs_string_t rayflect_format(const ecs_struct_t *ecs_struct, const char *struct_name);

#endif
