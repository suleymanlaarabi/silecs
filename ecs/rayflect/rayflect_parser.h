#ifndef RAYFLECT_PARSER_H
#define RAYFLECT_PARSER_H

#include "rayflect_types.h"

void rayflect_parse(ecs_struct_t *ecs_struct, const char *def);
void rayflect_free(ecs_struct_t *ecs_struct);

#endif
