#ifndef STDIO_DEVTOOL_COMMANDS_H
#define STDIO_DEVTOOL_COMMANDS_H

#include "ecs_types.h"

void stdio_devtool_command_new(ecs_world_t *world, const char *args);
void stdio_devtool_command_set_name(ecs_world_t *world, const char *args);

#endif
