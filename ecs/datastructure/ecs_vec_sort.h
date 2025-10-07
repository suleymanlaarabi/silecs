#ifndef ECS_VEC_SORT_H
    #define ECS_VEC_SORT_H

    #include <stdint.h>
    #include <stdlib.h>
    #include <string.h>
    #include "ecs_config.h"
    #include "ecs_vec.h"

ECS_INLINE
void insertion_sort_uint64(uint64_t *arr, size_t n) {
    for (size_t i = 1; i < n; i++) {
        uint64_t key = arr[i];
        size_t j = i;
        while (j > 0 && arr[j - 1] > key) {
            arr[j] = arr[j - 1];
            j--;
        }
        arr[j] = key;
    }
}

ECS_INLINE
int cmp_uint64(const void *a, const void *b) {
    uint64_t ua = *(const uint64_t*)a;
    uint64_t ub = *(const uint64_t*)b;
    return (ua > ub) - (ua < ub);
}

ECS_INLINE
void ecs_vec_sort_u64(ecs_vec_t *v) {
    uint64_t *arr = (uint64_t*)v->data;
    size_t n = v->count;

    if (n <= 50) {
        insertion_sort_uint64(arr, n);
    } else {
        qsort(arr, n, sizeof(uint64_t), cmp_uint64);
    }
}
ECS_INLINE
void ecs_vec_sort_last_u64(ecs_vec_t *v) {
    uint64_t *arr = (uint64_t*)v->data;
    size_t n = v->count;

    if (n <= 1) return;

    uint64_t key = arr[n - 1];
    size_t i = n - 1;

    while (i > 0 && arr[i - 1] > key) {
        arr[i] = arr[i - 1];
        i--;
    }
    arr[i] = key;
}

#endif
