#ifndef ECS_SPARSESET_H
#define ECS_SPARSESET_H

    #include "ecs_vec.h"
    #include <stdbool.h>
    #include <stdint.h>
    #include <stdio.h>
    #include <string.h>

typedef struct {
    uint32_t indices[256];
} ecs_sparseset_page_t;

typedef struct {
    void *pages[1024];
} ecs_sparseset_lvl_t;

typedef struct {
    ecs_vec_t dense;
    ecs_vec_t dense_sparse_key;
    ecs_sparseset_lvl_t root;
} ecs_sparseset_t;


ECS_INLINE
ecs_sparseset_lvl_t *ensure_alloc(void **ptr) {
    if (__builtin_expect(*ptr == NULL, 0)) {
        *ptr = calloc(1, sizeof(ecs_sparseset_lvl_t));
    }
    return *ptr;
}

ECS_INLINE
ecs_sparseset_page_t *get_sparse_page(ecs_sparseset_t *set, uint64_t key) {
    ecs_sparseset_lvl_t *lvl1 = ensure_alloc((void**)&set->root.pages[(key >> 38) & 0x3FF]);
    ecs_sparseset_lvl_t *lvl2 = ensure_alloc((void**)&lvl1->pages[(key >> 28) & 0x3FF]);
    ecs_sparseset_lvl_t *lvl3 = ensure_alloc((void**)&lvl2->pages[(key >> 18) & 0x3FF]);

    ecs_sparseset_page_t **page = (ecs_sparseset_page_t**)&lvl3->pages[(key >> 8) & 0x3FF];

    if (ECS_UNLIKELY(*page == NULL)) {
        *page = malloc(sizeof(ecs_sparseset_page_t));
        memset(*page, 0xFF, sizeof(ecs_sparseset_page_t));
    }

    return *page;
}

ECS_INLINE
void ecs_sparseset_insert(ecs_sparseset_t *set, uint64_t key, void *value) {
    uint8_t offset = key & 0xFF;
    ecs_sparseset_page_t *page = get_sparse_page(set, key);
    uint32_t index = page->indices[offset];

    if (index == UINT32_MAX) {
        ecs_vec_push(&set->dense, value);
        ecs_vec_push(&set->dense_sparse_key, &index);
        page->indices[offset] = set->dense.count - 1;
        return;
    }

    ecs_vec_set_unsafe(&set->dense, index, value);
}

ECS_INLINE
void *ecs_sparseset_get(ecs_sparseset_t *set, uint64_t key) {
    uint32_t index = get_sparse_page(set, key)->indices[key & 0xFF];

    if (index == UINT32_MAX) {
        return NULL;
    }

    return ECS_VEC_GET(void, &set->dense, index);
}

ECS_INLINE
void ecs_sparseset_set_dense_index(ecs_sparseset_t *set, uint64_t key, uint32_t dense_index) {
    get_sparse_page(set, key)->indices[key & 0xFF] = dense_index;
}

ECS_INLINE
bool ecs_sparseset_exists(ecs_sparseset_t *set, uint64_t key) {
    return get_sparse_page(set, key)->indices[key & 0xFF] != UINT32_MAX;
}

ECS_INLINE
void ecs_sparseset_remove(ecs_sparseset_t *set, uint64_t key) {
    uint8_t offset = key & 0xFF;
    ecs_sparseset_page_t *page = get_sparse_page(set, key);
    uint32_t index = page->indices[offset];

    ecs_vec_remove_fast(&set->dense, index);

    uint32_t last_index = set->dense.count;
    if (index != last_index) {
        uint32_t *swapped_key = ECS_VEC_GET(uint32_t, &set->dense_sparse_key, index);
        ecs_sparseset_set_dense_index(set, *swapped_key, index);
    }

    ecs_vec_remove_fast(&set->dense_sparse_key, index);
    page->indices[offset] = UINT32_MAX;
}

ECS_INLINE
void ecs_sparseset_init(ecs_sparseset_t *set, size_t elem_size) {
    set->dense = ecs_vec_create(elem_size);
    set->dense_sparse_key = ecs_vec_create(sizeof(uint_fast64_t));
    set->root = (ecs_sparseset_lvl_t) {0};
}

ECS_INLINE
void ecs_sparseset_fini(ecs_sparseset_t *set) {
    ecs_vec_free(&set->dense);
    ecs_vec_free(&set->dense_sparse_key);

    for (int i = 0; i < 1024; i++) {
        ecs_sparseset_lvl_t *lvl1 = (ecs_sparseset_lvl_t *)set->root.pages[i];
        if (lvl1 == NULL) continue;

        for (int j = 0; j < 1024; j++) {
            ecs_sparseset_lvl_t *lvl2 = (ecs_sparseset_lvl_t *)lvl1->pages[j];
            if (lvl2 == NULL) continue;

            for (int k = 0; k < 1024; k++) {
                ecs_sparseset_lvl_t *lvl3 = (ecs_sparseset_lvl_t *)lvl2->pages[k];
                if (lvl3 == NULL) continue;

                for (int l = 0; l < 1024; l++) {
                    ecs_sparseset_page_t *page = (ecs_sparseset_page_t *)lvl3->pages[l];
                    if (page != NULL) {
                        free(page);
                    }
                }
                free(lvl3);
            }
            free(lvl2);
        }
        free(lvl1);
    }
}

#endif
