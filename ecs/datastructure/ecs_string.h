#ifndef ECS_STRING_H
    #define ECS_STRING_H
    #include "ecs_vec.h"
    #include <stddef.h>
    #include <stdint.h>
    #include <string.h>

typedef ecs_vec_t ecs_string_t;

ecs_string_t ecs_string_from_cstring(char *str);
ecs_string_t ecs_string_new();

void ecs_string_push(ecs_string_t *string, const char c);
void ecs_string_push_cstr(ecs_string_t *string, const char *str);
void ecs_string_push_str(ecs_string_t *string, const char *str, uint32_t len);


#endif
