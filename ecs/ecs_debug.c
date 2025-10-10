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
    if (!query->terms[0].id.value) {
        puts("none)");
        return;
    }
    for (uint32_t i = 0; query->terms[i].id.value; i++) {
        ecs_query_term_t *term = &query->terms[i];
        printf("%s", term->oper == EcsQueryOperNot ? "!" : "");
        ecs_print_id(world, term->id);
        printf("%s", query->terms[i + 1].id.value ? ", " : "");
    }
    printf(")\n");
}

void ecs_print_queryid(ecs_world_t *world, EcsQueryId id) {
    ecs_query_cache_t *cache = ECS_VEC_GET(ecs_query_cache_t, &world->queries, id);
    ecs_print_query(world, &cache->query);
}

ecs_type_t *ecs_entity_type(ecs_world_t *world, ecs_entity_t entity) {
    return &ecs_world_get_entity_archetype(world, entity)->type;
}

void ecs_print_it(ecs_iter_t *it) {
    printf("Count: %d\n", it->count);
    printf("ArchCount: %ld\n", it->archetypes->count);
    printf("Current: %d\n", it->current_archetype);
    fflush(stdout);
}

void ecs_print_entity(ecs_world_t *world, ecs_entity_t entity) {
    if (ecs_has(world, entity, ecs_id(EcsName))) {
        EcsName *name = ecs_get(world, entity, ecs_id(EcsName));
        printf("%s: ", *name);
    } else {
        printf("Unnamed: ");
    }
    print_entity(entity);
    printf("    ");
    ecs_print_type(world, ecs_entity_type(world, entity));
}


void ecs_archetype_print(ecs_archetype_t *archetype)
{
    printf("Archetype: ");
    ecs_vec_print_type(&archetype->type);
    printf("\n");
}
