#ifndef STDIO_DEVTOOL_ARGS_H
#define STDIO_DEVTOOL_ARGS_H

#include "../datastructure/ecs_vec.h"
#include "../ecs_types.h"
#include <stdbool.h>

typedef struct ecs_world_t ecs_world_t;

typedef struct {
    ecs_vec_t args;
    size_t count;
} command_args_t;

void command_args_init(command_args_t *args, const char *cmd_line);

void command_args_free(command_args_t *args);
const char *command_args_get(const command_args_t *args, size_t index);
size_t command_args_count(const command_args_t *args);
bool command_args_parse_entity(ecs_world_t *world, const char *entity_str, ecs_entity_t *out_entity);

#endif
