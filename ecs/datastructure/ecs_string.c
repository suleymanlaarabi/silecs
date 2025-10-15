#include "ecs_string.h"
#include "ecs_vec.h"

ecs_string_t ecs_string_from_cstring(char *str) {
    size_t len = strlen(str);

    return (ecs_string_t) {
        .data = strdup(str),
        .capacity = len,
        .count = len,
        .size = sizeof(char),
    };
}

ecs_string_t ecs_string_new() {
    return ecs_vec_create(sizeof(char));
}

void ecs_string_push_cstr(ecs_string_t *string, const char *str) {
    ecs_vec_push_batch(string, str, strlen(str));
}

void ecs_string_push_str(ecs_string_t *string, const char *str, uint32_t len) {
    ecs_vec_push_batch(string, str, len);
}

void ecs_string_push(ecs_string_t *string, const char c) {
    ecs_vec_push(string, &c);
}
