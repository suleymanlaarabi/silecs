#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "ecs_debug.h"
#include "ecs_world.h"

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
