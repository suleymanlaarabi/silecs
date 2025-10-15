#include "stdio_devtool_args.h"
#include "parsing/ecs_scanner.h"
#include "datastructure/ecs_string.h"
#include "ecs_world.h"
#include <stdlib.h>
#include <string.h>

static bool is_quote(char c)
{
    return c == '"' || c == '\'';
}

static bool is_not_space(char c)
{
    return !ecs_isspace(c);
}

void command_args_init(command_args_t *args, const char *cmd_line)
{
    ecs_vec_init(&args->args, sizeof(char *));
    args->count = 0;

    if (!cmd_line || !*cmd_line) {
        return;
    }

    ecs_scanner_t scanner;
    ecs_scanner_init(&scanner, cmd_line);

    while (!ecs_scanner_is_done(&scanner)) {
        ecs_scanner_skip_whitespace(&scanner);

        if (ecs_scanner_is_done(&scanner)) {
            break;
        }

        ecs_string_t arg;

        if (is_quote(ecs_scanner_peek(&scanner))) {
            ecs_scanner_pop(&scanner);
            arg = ecs_scanner_take_until(&scanner, is_quote);
            if (!ecs_scanner_is_done(&scanner)) {
                ecs_scanner_pop(&scanner);
            }
        } else {
            arg = ecs_scanner_take_while(&scanner, is_not_space);
        }

        if (arg.count > 0) {
            char *arg_str = (char *)malloc(arg.count + 1);
            memcpy(arg_str, arg.data, arg.count);
            arg_str[arg.count] = '\0';

            ecs_vec_push(&args->args, &arg_str);
            args->count++;

            ecs_vec_free(&arg);
        }
    }
}

void command_args_free(command_args_t *args)
{
    for (size_t i = 0; i < args->count; i++) {
        char **arg = (char **)ECS_VEC_GET(char *, &args->args, i);
        free(*arg);
    }
    ecs_vec_free(&args->args);
    args->count = 0;
}

const char *command_args_get(const command_args_t *args, size_t index)
{
    if (index >= args->count) {
        return NULL;
    }

    char **arg = (char **)ECS_VEC_GET(char *, &args->args, index);
    return *arg;
}

size_t command_args_count(const command_args_t *args)
{
    return args->count;
}

bool command_args_parse_entity(ecs_world_t *world, const char *entity_str, ecs_entity_t *out_entity)
{
    if (!entity_str || !out_entity) {
        return false;
    }

    ecs_entity_t entity = ecs_lookup(world, entity_str);

    if (entity.value == 0) {
        char *endptr;
        uint32_t index = (uint32_t)strtoul(entity_str, &endptr, 10);
        if (*endptr == '\0' && index != 0) {
            entity = ecs_entity_manager_get_entity(&world->entity_manager, index);
        }
    }

    if (entity.value != 0 && ecs_is_alive(world, entity)) {
        *out_entity = entity;
        return true;
    }

    return false;
}
