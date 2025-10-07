#ifndef ECS_STRMAP_H
    #define ECS_STRMAP_H
    #include "ecs_config.h"
    #include "ecs_types.h"
    #include <stdint.h>
    #include <stdlib.h>
    #include <string.h>


typedef struct {
    const char *key;
    ecs_entity_t value;
} ecs_strmap_entry_t;

typedef struct {
    ecs_strmap_entry_t *entries;
    size_t capacity;
    size_t count;
} ecs_strmap_t;

ECS_INLINE
uint64_t hash_str(const char *s) {
    uint64_t h = 1469598103934665603ULL;
    while (*s) h = (h ^ (unsigned char)*s++) * 1099511628211ULL;
    return h;
}

ECS_INLINE
ecs_strmap_t *ecs_strmap_create(size_t initial_capacity) {
    ecs_strmap_t *map = malloc(sizeof(ecs_strmap_t));
    map->capacity = initial_capacity;
    map->count = 0;
    map->entries = calloc(initial_capacity, sizeof(ecs_strmap_entry_t));
    return map;
}

ECS_INLINE
void ecs_strmap_init(ecs_strmap_t *map, size_t initial_capacity) {
    map->capacity = initial_capacity;
    map->count = 0;
    map->entries = calloc(initial_capacity, sizeof(ecs_strmap_entry_t));
}

ECS_INLINE
void ecs_strmap_set(ecs_strmap_t *map, const char *key, ecs_entity_t value) {
    size_t cap = map->capacity;
    uint64_t h = hash_str(key);
    size_t i = h & (cap - 1);

    for (;;) {
        if (!map->entries[i].key || strcmp(map->entries[i].key, key) == 0) {
            map->entries[i].key = key;
            map->entries[i].value = value;
            return;
        }
        i = (i + 1) & (cap - 1);
    }
}

ECS_INLINE
ecs_entity_t ecs_strmap_get(ecs_strmap_t *map, const char *key) {
    size_t cap = map->capacity;
    uint64_t h = hash_str(key);
    size_t i = h & (cap - 1);

    for (;;) {
        const char *k = map->entries[i].key;
        if (!k) return (ecs_entity_t) {0};
        if (strcmp(k, key) == 0) return map->entries[i].value;
        i = (i + 1) & (cap - 1);
    }
}

ECS_INLINE
void ecs_strmap_destroy(ecs_strmap_t *map) {
    free(map->entries);
}

#endif
