#ifndef ECS_VEC_HASHMAP_H
#define ECS_VEC_HASHMAP_H
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ecs_config.h>
#include "ecs_vec.h"

#define HASHMAP_CAPACITY 8192
#define EMPTY_KEY 0xFFFFFFFFFFFFFFFFULL

typedef struct {
    ecs_vec_t key;
    uint64_t value;
    bool used;
} hash_entry_t;

typedef struct {
    hash_entry_t entries[HASHMAP_CAPACITY];
} ecs_hashmap_t;

ECS_INLINE
uint64_t hash64(uint64_t x) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

ECS_INLINE
uint64_t hash_key(const ecs_vec_t *k) {
    uint64_t h = 0xCBF29CE484222325ULL;
    for (size_t i = 0; i < k->count; i++) {
        uint64_t val = *(uint64_t *)((char*)k->data + i * k->size);
        h ^= hash64(val);
    }
    return h;
}

ECS_INLINE
bool key_equal(const ecs_vec_t *a, const ecs_vec_t *b) {
    if (a->count != b->count) return false;
    return memcmp(a->data, b->data, a->count * sizeof(uint64_t)) == 0;
}

ECS_INLINE
void ecs_hashmap_init(ecs_hashmap_t *map) {
    for (int i = 0; i < HASHMAP_CAPACITY; i++) {
        map->entries[i].used = false;
        ecs_vec_init(&map->entries[i].key, sizeof(uint64_t));
    }
}

ECS_INLINE
void ecs_hashmap_fini(ecs_hashmap_t *map) {
    for (int i = 0; i < HASHMAP_CAPACITY; i++) {
        ecs_vec_free(&map->entries[i].key);
    }
}

ECS_INLINE
bool ecs_hashmap_put(ecs_hashmap_t *map, const ecs_vec_t *key, uint64_t value) {
    uint64_t h = hash_key(key);
    uint64_t idx = h & (HASHMAP_CAPACITY - 1);

    for (int i = 0; i < HASHMAP_CAPACITY; i++) {
        uint64_t probe = (idx + i) & (HASHMAP_CAPACITY - 1);
        hash_entry_t *e = &map->entries[probe];

        if (!e->used || key_equal(&e->key, key)) {
            ecs_vec_free(&e->key);
            ecs_vec_copy((ecs_vec_t *)key, &e->key);
            e->value = value;
            e->used = true;
            return true;
        }
    }
    return false;
}

ECS_INLINE
bool ecs_hashmap_get(const ecs_hashmap_t *map, const ecs_vec_t *key, uint64_t *out_value) {
    uint64_t h = hash_key(key);
    uint64_t idx = h & (HASHMAP_CAPACITY - 1);

    for (int i = 0; i < HASHMAP_CAPACITY; i++) {
        uint64_t probe = (idx + i) & (HASHMAP_CAPACITY - 1);
        const hash_entry_t *e = &map->entries[probe];

        if (!e->used) return false;
        if (key_equal(&e->key, key)) {
            *out_value = e->value;
            return true;
        }
    }
    return false;
}

ECS_INLINE
bool ecs_hashmap_remove(ecs_hashmap_t *map, const ecs_vec_t *key) {
    uint64_t h = hash_key(key);
    uint64_t idx = h & (HASHMAP_CAPACITY - 1);

    for (int i = 0; i < HASHMAP_CAPACITY; i++) {
        uint64_t probe = (idx + i) & (HASHMAP_CAPACITY - 1);
        hash_entry_t *e = &map->entries[probe];
        if (!e->used) return false;
        if (key_equal(&e->key, key)) {
            ecs_vec_free(&e->key);
            e->used = false;
            return true;
        }
    }
    return false;
}

#endif
