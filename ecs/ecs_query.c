#include "ecs_query.h"
#include "ecs_archetype.h"
#include "ecs_config.h"
#include "ecs_sparseset.h"
#include "ecs_types.h"
#include "ecs_vec.h"
#include "ecs_world.h"
#include "dsl_types.h"
#include "parser.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {} EcsQueryIdMap;

ECS_COMPONENT_DEFINE(EcsQueryId);
ECS_COMPONENT_DEFINE(EcsQueryIdMap);

bool ecs_query_match_type(ecs_query_t *query, ecs_type_t *type) {
    ecs_entity_t *entities = type->data;
    uint32_t tlen = type->count;

    for (uint32_t i = 0; query->terms[i].id.value; i++) {
        ecs_query_term_t term = query->terms[i];
        bool matched = false;

        if (term.flags & EcsQueryFlagSingleton) {
            continue;
        }

        for (uint32_t j = 0; j < tlen; j++) {
            if (term.id.value == entities[j].value) {
                matched = true;
                break;
            }
        }

        if (term.oper == EcsQueryOperEqual && !matched) {
            return false;
        }
        if (term.oper == EcsQueryOperNot && matched) {
            return false;
        }
    }

    return true;
}

ECS_INLINE
ecs_entity_t get_select_query_select(ecs_query_t *query) {
    for (int i = 0; query->terms[i].id.value; i++) {
        if (query->terms[i].oper == EcsQueryOperEqual) {
            return query->terms[i].id;
        }
    }
    return ECS_NULL;
}

static void ecs_query_update_matches(ecs_world_t *world, ecs_query_cache_t *cache) {
    ecs_archetype_t *archetypes = world->archetypes.data;
    uint32_t len = world->archetypes.count;

    ecs_entity_t select = get_select_query_select(&cache->query);
    if (select.value) {
        ecs_vec_t *select_archetypes = ecs_sparseset_get(&world->component_archetypes, select.value);
        if (select_archetypes) {
            ecs_archetype_id_t *select_ids = select_archetypes->data;
            for (uint32_t i = 0; i < select_archetypes->count; i++) {
                if (ecs_query_match_type(&cache->query, &archetypes[select_ids[i]].type)) {
                    ecs_vec_push(&cache->archetypes, &select_ids[i]);
                }
            }
            return;
        }
    }
    for (uint32_t i = 0; i < len; i++) {
        if (ecs_query_match_type(&cache->query, &archetypes[i].type)) {
            ecs_vec_push(&cache->archetypes, &i);
        }
    }
}

static ecs_query_term_t ecs_query_term_from_dsl(ecs_world_t *world, ecs_dsl_term_t term) {
    ecs_query_term_t result = {0};

    if (term.id.is_pair) {
        result.id = ecs_make_pair(
            ecs_strmap_get(&world->entity_map, term.id.first),
            term.id.second_wildcard ? ecs_id(EcsWildcard) : ecs_strmap_get(&world->entity_map, term.id.second)
        );
        result.oper = EcsQueryOperEqual;
        return result;
    }

    result.id = ecs_strmap_get(&world->entity_map, term.id.first);

    if (!result.id.value) {
        return (ecs_query_term_t) {0};
    }

    if (term.modifier == ECS_DSL_MOD_NONE || term.modifier == ECS_DSL_MOD_OPTIONAL) {
        result.oper = EcsQueryOperEqual;
    } else {
        result.oper = EcsQueryOperNot;
    }

    return result;
}

ecs_query_t *ecs_query_from_str(ecs_world_t *world, const char *str) {
    ecs_dsl_parser_t parser;
    ecs_dsl_parser_init(&parser, str);
    ecs_dsl_query_t *dsl_query = ecs_dsl_parser_parse(&parser);

    ecs_query_t *query = malloc(sizeof(ecs_query_t));

    for (uint32_t i = 0; i < dsl_query->count && i < 8; i++) {
        query->terms[i] = ecs_query_term_from_dsl(world, dsl_query->terms[i]);
        if (!query->terms[i].id.value) {
            return NULL;
        }
    }

    ecs_dsl_query_free(dsl_query);
    ecs_dsl_parser_free(&parser);
    return query;
}

EcsQueryId ecs_query_register(ecs_world_t *world, ecs_query_t *query) {
    ecs_query_cache_t cache = {
        .archetypes = ecs_vec_create(sizeof(ecs_archetype_id_t)),
        .query = *query
    };

    ecs_query_update_matches(world, &cache);
    ecs_vec_push(&world->queries, &cache);
    return world->queries.count - 1;
}

ecs_iter_t ecs_query(ecs_world_t *world, ecs_query_t *query) {
    static ecs_query_cache_t cache;

    cache.archetypes = ecs_vec_create(sizeof(ecs_archetype_id_t));
    cache.query = *query;

    ecs_query_update_matches(world, &cache);
    return (ecs_iter_t) {
        .world = world,
        .archetypes = &cache.archetypes,
        .count = 0,
        .current_archetype = -1
    };
}

ecs_iter_t ecs_query_iter(ecs_world_t *world, EcsQueryId query) {
    ecs_vec_t *archetypes = &ECS_VEC_GET(ecs_query_cache_t, &world->queries, query)->archetypes;

    return (ecs_iter_t) {
        .world = world,
        .archetypes = archetypes,
        .count = 0,
        .current_archetype = -1
    };
}

bool ecs_iter_next(ecs_iter_t *it) {
    it->current_archetype += 1;

    if (it->current_archetype >= (int) it->archetypes->count) {
        return false;
    }

    ecs_archetype_id_t archetype_id = *ECS_VEC_GET(ecs_archetype_id_t, it->archetypes, it->current_archetype);
    it->archetype_p = ecs_world_get_archetype(it->world, archetype_id);
    it->count = it->archetype_p->entities.count;

    return true;
}

void EcsQueryModule(ecs_world_t *world) {
    ECS_REGISTER_COMPONENT(world, EcsQueryId);
    ECS_REGISTER_COMPONENT(world, EcsQueryIdMap);
}
