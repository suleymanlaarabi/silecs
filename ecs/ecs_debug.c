#include "ecs_types.h"
#include <ecs_debug.h>
#include <ecs_world.h>
#include <stdio.h>

void ecs_print_id(ecs_world_t *world, ecs_entity_t entity) {
    if (!ecs_has(world, entity, ecs_id(EcsName))) return;

    char **name = ecs_get(world, entity, ecs_id(EcsName));
    if (ecs_is_pair(entity)) {
        ecs_entity_t target = ecs_pair_target(entity);
        if (ecs_has(world, target, ecs_id(EcsName))) {
            char **target_name = ecs_get(world, target, ecs_id(EcsName));
            printf("(%s, %s)", *name, *target_name);
        } else {
            printf("(%s, %s)", *name, "Unnamed");
        }
        return;
    }
    printf("%s", *name);
}

void ecs_print_type(ecs_world_t *world, ecs_type_t *type) {
    printf("(");
    iter_vec(ecs_entity_t, type) {
        ecs_print_id(world, iter_value);
        printf("%s", __index < type->count - 1 ? ", " : "");
    }
    puts(")");
}

void ecs_print_query(ecs_world_t *world, ecs_query_t *query) {
    printf("(");
    for (uint32_t i = 0; i < query->terms[i].id.value; i++) {
        ecs_query_term_t *term = &query->terms[i];
        printf("%s", term->oper == EcsQueryOperNot ? "!" : "");
        ecs_print_id(world, term->id);
        printf("%s", i == query->terms[i].id.value - 1 ? "" : ", ");
    }
    printf(")\n");
}

void ecs_print_queryid(ecs_world_t *world, EcsQueryId id) {
    ecs_query_cache_t *cache = ECS_VEC_GET(ecs_query_cache_t, &world->queries, id);
    printf("Query ID: %d\n", id);
    ecs_print_query(world, &cache->query);
}

void ecs_print_entity(ecs_world_t *world, ecs_entity_t entity) {
    print_entity(entity);
    ecs_print_type(world, &ecs_world_get_entity_archetype(world, entity)->type);
}
