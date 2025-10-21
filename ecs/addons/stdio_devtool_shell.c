#include "ecs_world.h"
#include "rayflect/rayflect_format.h"
#include "stdio_devtool.h"
#include <rayflect/ecs_rayflect.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ecs_system.h"
#include "ecs_types.h"
#include "ecs_bootstrap.h"
#include "ecs_debug.h"
#include "stdio_devtool_commands.h"
#include "stdio_devtool_query.h"
#include "stdio_devtool_utils.h"
#include "stdio_devtool_args.h"

typedef bool (*command_handler_t)(ecs_world_t *world, const char *args);

typedef struct {
    const char *name;
    size_t name_len;
    command_handler_t handler;
    const char *usage;
} command_entry_t;

static bool cmd_progress(ecs_world_t *world, const char *args)
{
    (void)world;
    (void)args;
    return false;
}

static bool cmd_clear(ecs_world_t *world, const char *args)
{
    (void)world;
    (void)args;
    stdio_devtool_clear_screen();
    return true;
}

static bool cmd_ls(ecs_world_t *world, const char *args)
{
    (void)args;
    ecs_iter_t it = ecs_query(world, &query({
        .terms = {
            { ecs_id(EcsComponent) }, { ecs_id(EcsName) }
        }
    }));

    while (ecs_iter_next(&it)) {
        EcsName *name = ecs_field(&it, EcsName);
        for (int i = 0; i < it.count; i++) {
            printf("%s\n", name[i]);
        }
    }
    return true;
}

static bool cmd_ps(ecs_world_t *world, const char *args)
{
    (void)args;
    ecs_iter_t it = ecs_query(world, &query({
        .terms = {
            { ecs_id(EcsSystem) },
            { ecs_id(EcsQueryId) },
            { ecs_id(EcsName) },
        }
    }));

    while (ecs_iter_next(&it)) {
        EcsName *names = ecs_field(&it, EcsName);
        EcsQueryId *queries = ecs_field(&it, EcsQueryId);
        for (int i = 0; i < it.count; i++) {
            printf("%s: ", names[i]);
            ecs_print_queryid(world, queries[i]);
        }
    }
    return true;
}

static bool cmd_query(ecs_world_t *world, const char *args)
{
    stdio_devtool_execute_query(world, args);
    return true;
}

static bool cmd_exit(ecs_world_t *world, const char *args)
{
    (void)world;
    (void)args;
    exit(0);
}

static bool cmd_set(ecs_world_t *world, const char *args)
{
    stdio_devtool_command_set(world, args);
    return true;
}

static bool cmd_new(ecs_world_t *world, const char *args)
{
    stdio_devtool_command_new(world, args);
    return true;
}

static bool cmd_set_name(ecs_world_t *world, const char *args)
{
    stdio_devtool_command_set_name(world, args);
    return true;
}

static bool cmd_print_component(ecs_world_t *world, const char *args)
{
    ecs_entity_t component = ecs_lookup(world, args);
    if (component.value != 0 && ecs_is_alive(world, component)) {
        if (!ecs_has(world, component, ecs_id(EcsStruct))) {
            printf("Component is not reflected\n");
            return true;
        }
        EcsStruct *component_struct = ecs_get(world, component, ecs_id(EcsStruct));
        rayflect_print(component_struct, NULL);
    } else {
        printf("Component not found\n");
    }
    return true;
}

static bool cmd_inspect_entity_component(ecs_world_t *world, const char *args)
{
    command_args_t cmd_args;
    command_args_init(&cmd_args, args);

    if (command_args_count(&cmd_args) != 2) {
        printf("Usage: inspect <entity_name_or_index> <component_name>\n");
        command_args_free(&cmd_args);
        return true;
    }

    const char *entity_str = command_args_get(&cmd_args, 0);
    const char *component_name = command_args_get(&cmd_args, 1);

    ecs_entity_t entity;
    if (!command_args_parse_entity(world, entity_str, &entity)) {
        printf("Entity '%s' not found or not alive\n", entity_str);
        command_args_free(&cmd_args);
        return true;
    }

    ecs_entity_t component = ecs_lookup(world, component_name);
    if (component.value == 0 || !ecs_is_alive(world, component)) {
        printf("Component '%s' not found\n", component_name);
        command_args_free(&cmd_args);
        return true;
    }

    if (!ecs_has(world, entity, component)) {
        printf("Entity '%s' does not have component '%s'\n", entity_str, component_name);
        command_args_free(&cmd_args);
        return true;
    }

    if (!ecs_has(world, component, ecs_id(EcsStruct))) {
        printf("Component '%s' is not reflected\n", component_name);
        command_args_free(&cmd_args);
        return true;
    }

    EcsStruct *component_struct = ecs_get(world, component, ecs_id(EcsStruct));
    void *component_data = ecs_get(world, entity, component);

    printf("Entity '%s' component '%s':\n", entity_str, component_name);
    rayflect_print(component_struct, component_data);

    command_args_free(&cmd_args);
    return true;
}

static bool cmd_help(ecs_world_t *world, const char *args);

static const command_entry_t commands[] = {
    { "progress", 8, cmd_progress, NULL },
    { "clear", 5, cmd_clear, "clear" },
    { "ls", 2, cmd_ls, "ls" },
    { "ps", 2, cmd_ps, "ps" },
    { "query", 5, cmd_query, "query <query>" },
    { "exit", 4, cmd_exit, "exit" },
    { "new", 3, cmd_new, "new <name> [components...]" },
    { "set", 3, cmd_set, "set <entity_name_or_index> <Component.field> <value>" },
    { "set_name", 8, cmd_set_name, "set_name <entity_index> <name>" },
    { "rayflect", 8, cmd_print_component, "print_component <entity_index> <component>" },
    { "inspect", 7, cmd_inspect_entity_component, "inspect <entity_name_or_index> <component_name>" },
    { "help", 4, cmd_help, "help" },
    { NULL, 0, NULL, NULL }
};

static bool cmd_help(ecs_world_t *world, const char *args)
{
    (void)world;
    (void)args;
    printf("Available commands:\n");
    for (int i = 0; commands[i].name != NULL; i++) {
        if (commands[i].usage) {
            printf("  %s\n", commands[i].usage);
        }
    }
    return true;
}

static bool process_command(ecs_world_t *world, const char *cmd)
{
    if (!cmd)
        return true;

    const char *trimmed = stdio_devtool_trim_whitespace(cmd);
    if (!trimmed)
        return true;

    const char *space = strchr(trimmed, ' ');
    size_t cmd_len = space ? (size_t)(space - trimmed) : strlen(trimmed);
    const char *args = space ? space + 1 : "";

    for (int i = 0; commands[i].name != NULL; i++) {
        if (cmd_len == commands[i].name_len &&
            strncmp(trimmed, commands[i].name, cmd_len) == 0) {
            return commands[i].handler(world, args);
        }
    }

    printf("Command not found: '%.*s'\n", (int)cmd_len, trimmed);
    return true;
}

static void run_shell(ecs_world_t *world)
{
    char *input = NULL;
    size_t len = 0;
    ssize_t read;

    while (1) {
        printf("silecs> ");
        fflush(stdout);

        read = getline(&input, &len, stdin);
        if (read == -1)
            break;

        if (read > 0 && input[read - 1] == '\n')
            input[read - 1] = '\0';

        if (!process_command(world, input))
            break;
    }

    free(input);
}

void InspectorShellSystem(ecs_iter_t *it)
{
    run_shell(it->world);
}

void EcsStdioInspectorModule(ecs_world_t *world)
{
    ECS_SYSTEM(world, InspectorShellSystem, EcsOnPreUpdate);
}
