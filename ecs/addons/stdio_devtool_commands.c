#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ecs_debug.h"
#include "ecs_world.h"
#include "stdio_devtool_args.h"
#include "rayflect/ecs_rayflect.h"
#include "rayflect/rayflect_types.h"
#include "parsing/ecs_scanner.h"

#define MAX_COMPONENTS 32
#define MAX_TOKEN_LEN 256

typedef struct {
    bool is_pair;
    char first[MAX_TOKEN_LEN];
    char second[MAX_TOKEN_LEN];
} parsed_token_t;

static const char *skip_whitespace(const char *p)
{
    while (*p && (*p == ' ' || *p == '\t'))
        p++;
    return p;
}

static const char *parse_identifier(const char *p, char *out, size_t max_len)
{
    size_t len = 0;
    while (*p && !isspace(*p) && *p != ',' && *p != ')' && len < max_len - 1) {
        out[len++] = *p++;
    }
    out[len] = '\0';
    return p;
}

static const char *parse_pair(const char *p, parsed_token_t *token)
{
    token->is_pair = true;
    p++;
    p = skip_whitespace(p);
    p = parse_identifier(p, token->first, MAX_TOKEN_LEN);
    p = skip_whitespace(p);
    if (*p != ',') {
        token->is_pair = false;
        return p;
    }
    p++;
    p = skip_whitespace(p);
    p = parse_identifier(p, token->second, MAX_TOKEN_LEN);
    p = skip_whitespace(p);
    if (*p != ')') {
        token->is_pair = false;
        return p;
    }
    p++;
    return p;
}

static int parse_tokens(const char *args, parsed_token_t tokens[])
{
    int count = 0;
    const char *p = args;
    while (*p && count < MAX_COMPONENTS) {
        p = skip_whitespace(p);
        if (!*p)
            break;
        if (*p == '(') {
            p = parse_pair(p, &tokens[count]);
            if (tokens[count].is_pair)
                count++;
        } else {
            tokens[count].is_pair = false;
            p = parse_identifier(p, tokens[count].first, MAX_TOKEN_LEN);
            if (tokens[count].first[0])
                count++;
        }
    }
    return count;
}

static void add_component_or_pair(
    ecs_world_t *world,
    ecs_entity_t entity,
    const parsed_token_t *token)
{
    if (!token->is_pair) {
        ecs_entity_t component = ecs_lookup(world, token->first);
        if (component.index == 0) {
            printf("Warning: Component '%s' not found, skipping\n", token->first);
            return;
        }
        ecs_add(world, entity, component);
    } else {
        ecs_entity_t relation = ecs_lookup(world, token->first);
        ecs_entity_t target = ecs_lookup(world, token->second);
        if (relation.index == 0) {
            printf("Warning: Relation '%s' not found, skipping\n", token->first);
            return;
        }
        if (target.index == 0) {
            printf("Warning: Target '%s' not found, skipping\n", token->second);
            return;
        }
        ecs_add_pair(world, entity, relation, target);
    }
}

void stdio_devtool_command_new(ecs_world_t *world, const char *args)
{
    parsed_token_t tokens[MAX_COMPONENTS];
    int token_count = parse_tokens(args, tokens);

    if (token_count == 0) {
        puts("Usage: new <EntityName> [Component1] [(Relation, Target)] ...");
        return;
    }

    const char *entity_name = tokens[0].first;
    ecs_entity_t entity = ecs_new(world);
    ecs_add(world, entity, ecs_id(EcsName));
    char *name = strdup(entity_name);
    ecs_set(world, entity, ecs_id(EcsName), &name);

    for (int i = 1; i < token_count; i++) {
        add_component_or_pair(world, entity, &tokens[i]);
    }

    printf("Created entity: ");
    ecs_print_entity(world, entity);
}

void stdio_devtool_command_set_name(ecs_world_t *world, const char *args)
{
    const char *p = skip_whitespace(args);

    char entity_index_str[MAX_TOKEN_LEN];
    p = parse_identifier(p, entity_index_str, MAX_TOKEN_LEN);

    if (entity_index_str[0] == '\0') {
        puts("Usage: set_name <entity_index> <name>");
        return;
    }

    uint32_t index = (uint32_t) strtoul(entity_index_str, NULL, 10);
    if (index == 0) {
        printf("Error: Invalid entity index '%s'\n", entity_index_str);
        return;
    }

    p = skip_whitespace(p);
    char name_buffer[MAX_TOKEN_LEN];
    p = parse_identifier(p, name_buffer, MAX_TOKEN_LEN);

    if (name_buffer[0] == '\0') {
        puts("Usage: set_name <entity_index> <name>");
        return;
    }

    ecs_entity_t entity = ecs_entity_manager_get_entity(&world->entity_manager, index);

    if (!ecs_is_alive(world, entity)) {
        printf("Error: Entity with index %u does not exist or is not alive\n", index);
        return;
    }

    if (!ecs_has(world, entity, ecs_id(EcsName))) {
        ecs_add(world, entity, ecs_id(EcsName));
    }

    char *name = strdup(name_buffer);
    ecs_set(world, entity, ecs_id(EcsName), &name);

    printf("Set name of entity %u to '%s'\n", index, name_buffer);
}

static bool parse_value_from_string(const char *value_str, ecs_simple_type_t type, void *out_value)
{
    if (!value_str || !out_value) return false;

    ecs_scanner_t scanner;
    ecs_scanner_init(&scanner, value_str);
    ecs_scanner_skip_whitespace(&scanner);

    switch (type) {
        case ECS_TYPE_I8:
        case ECS_TYPE_I16:
        case ECS_TYPE_I32:
        case ECS_TYPE_I64: {
            ecs_string_t num_str = ecs_scanner_take_number(&scanner);
            if (num_str.count == 0) {
                ecs_vec_free(&num_str);
                return false;
            }
            ecs_string_push(&num_str, '\0');
            int64_t val = atoll((const char *)num_str.data);
            ecs_vec_free(&num_str);

            if (type == ECS_TYPE_I8) *(int8_t *)out_value = (int8_t)val;
            else if (type == ECS_TYPE_I16) *(int16_t *)out_value = (int16_t)val;
            else if (type == ECS_TYPE_I32) *(int32_t *)out_value = (int32_t)val;
            else *(int64_t *)out_value = val;
            return true;
        }
        case ECS_TYPE_U8:
        case ECS_TYPE_U16:
        case ECS_TYPE_U32:
        case ECS_TYPE_U64: {
            ecs_string_t num_str = ecs_scanner_take_number(&scanner);
            if (num_str.count == 0) {
                ecs_vec_free(&num_str);
                return false;
            }
            ecs_string_push(&num_str, '\0');
            uint64_t val = strtoull((const char *)num_str.data, NULL, 10);
            ecs_vec_free(&num_str);

            if (type == ECS_TYPE_U8) *(uint8_t *)out_value = (uint8_t)val;
            else if (type == ECS_TYPE_U16) *(uint16_t *)out_value = (uint16_t)val;
            else if (type == ECS_TYPE_U32) *(uint32_t *)out_value = (uint32_t)val;
            else *(uint64_t *)out_value = val;
            return true;
        }
        case ECS_TYPE_F32:
        case ECS_TYPE_F64: {
            ecs_string_t num_str = ecs_scanner_take_number(&scanner);
            if (num_str.count == 0) {
                ecs_vec_free(&num_str);
                return false;
            }
            ecs_string_push(&num_str, '\0');
            double val = strtod((const char *)num_str.data, NULL);
            ecs_vec_free(&num_str);

            if (type == ECS_TYPE_F32) *(float *)out_value = (float)val;
            else *(double *)out_value = val;
            return true;
        }
        case ECS_TYPE_BOOL: {
            ecs_string_t bool_str = ecs_scanner_take_identifier(&scanner);
            if (bool_str.count == 0) {
                bool_str = ecs_scanner_take_number(&scanner);
            }
            if (bool_str.count == 0) {
                ecs_vec_free(&bool_str);
                return false;
            }
            ecs_string_push(&bool_str, '\0');
            const char *str = (const char *)bool_str.data;
            *(bool *)out_value = (strcmp(str, "true") == 0 || strcmp(str, "1") == 0);
            ecs_vec_free(&bool_str);
            return true;
        }
        default:
            return false;
    }
}

void stdio_devtool_command_set(ecs_world_t *world, const char *args)
{
    command_args_t cmd_args;
    command_args_init(&cmd_args, args);

    if (command_args_count(&cmd_args) != 3) {
        printf("Usage: set <entity_name_or_index> <Component.field> <value>\n");
        command_args_free(&cmd_args);
        return;
    }

    const char *entity_str = command_args_get(&cmd_args, 0);
    const char *field_path = command_args_get(&cmd_args, 1);
    const char *value_str = command_args_get(&cmd_args, 2);

    ecs_entity_t entity;
    if (!command_args_parse_entity(world, entity_str, &entity)) {
        printf("Entity '%s' not found or not alive\n", entity_str);
        command_args_free(&cmd_args);
        return;
    }

    char component_name[MAX_TOKEN_LEN];
    char field_name[MAX_TOKEN_LEN];
    const char *dot = strchr(field_path, '.');
    if (!dot) {
        printf("Error: Invalid field path '%s'. Expected format: Component.field\n", field_path);
        command_args_free(&cmd_args);
        return;
    }

    size_t component_len = dot - field_path;
    if (component_len >= MAX_TOKEN_LEN) {
        printf("Error: Component name too long\n");
        command_args_free(&cmd_args);
        return;
    }

    strncpy(component_name, field_path, component_len);
    component_name[component_len] = '\0';

    strncpy(field_name, dot + 1, MAX_TOKEN_LEN - 1);
    field_name[MAX_TOKEN_LEN - 1] = '\0';

    ecs_entity_t component = ecs_lookup(world, component_name);
    if (component.value == 0 || !ecs_is_alive(world, component)) {
        printf("Component '%s' not found\n", component_name);
        command_args_free(&cmd_args);
        return;
    }

    if (!ecs_has(world, entity, component)) {
        printf("Entity '%s' does not have component '%s'\n", entity_str, component_name);
        command_args_free(&cmd_args);
        return;
    }

    if (!ecs_has(world, component, ecs_id(EcsStruct))) {
        printf("Component '%s' is not reflected\n", component_name);
        command_args_free(&cmd_args);
        return;
    }

    EcsStruct *component_struct = ecs_get(world, component, ecs_id(EcsStruct));
    if (!component_struct) {
        printf("Error: Could not get EcsStruct for component '%s'\n", component_name);
        command_args_free(&cmd_args);
        return;
    }

    void *component_data = ecs_get(world, entity, component);
    if (!component_data) {
        printf("Error: Could not get component data\n");
        command_args_free(&cmd_args);
        return;
    }

    const ecs_field_t *field = NULL;
    for (size_t i = 0; i < component_struct->fields.count; i++) {
        const ecs_field_t *f = ECS_VEC_GET(ecs_field_t, &component_struct->fields, i);
        if (f->name && strcmp(f->name, field_name) == 0) {
            field = f;
            break;
        }
    }

    if (!field) {
        printf("Field '%s' not found in component '%s'\n", field_name, component_name);
        command_args_free(&cmd_args);
        return;
    }

    uint8_t value_buffer[32];
    memset(value_buffer, 0, sizeof(value_buffer));

    if (!parse_value_from_string(value_str, field->type, value_buffer)) {
        printf("Error: Cannot parse value '%s' for field type %s\n",
               value_str, rayflect_type_to_string(field->type, field->array_size));
        command_args_free(&cmd_args);
        return;
    }

    if (rayflect_set_field(component_struct, component_data, field_name, value_buffer) != 0) {
        printf("Error: Failed to set field '%s'\n", field_name);
        command_args_free(&cmd_args);
        return;
    }

    printf("Set %s.%s = %s\n", component_name, field_name, value_str);
    command_args_free(&cmd_args);
}
