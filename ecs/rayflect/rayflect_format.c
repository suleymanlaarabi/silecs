#include "rayflect_format.h"
#include "rayflect_types.h"
#include <stdio.h>

void rayflect_print(const ecs_struct_t *ecs_struct)
{
    if (!ecs_struct) {
        printf("NULL struct\n");
        return;
    }

    printf("struct {\n");

    for (size_t i = 0; i < ecs_struct->fields.count; i++) {
        const ecs_field_t *field = ECS_VEC_GET(ecs_field_t, &ecs_struct->fields, i);
        printf("  [%zu] %s: %s (size: %zu, align: %zu)\n",
               i,
               field->name ? field->name : "(null)",
               field->simple_type ? field->simple_type : "unknown",
               field->size,
               field->align);
    }

    printf("}\n");
    printf("Total fields: %zu\n", ecs_struct->fields.count);
}

ecs_string_t rayflect_format(const ecs_struct_t *ecs_struct, const char *struct_name)
{
    ecs_string_t result = ecs_string_new();
    
    if (!ecs_struct) {
        ecs_string_push_cstr(&result, "NULL");
        return result;
    }

    if (struct_name) {
        ecs_string_push_cstr(&result, struct_name);
        ecs_string_push_cstr(&result, " ");
    }
    
    ecs_string_push_cstr(&result, "{ ");
    
    for (size_t i = 0; i < ecs_struct->fields.count; i++) {
        const ecs_field_t *field = ECS_VEC_GET(ecs_field_t, &ecs_struct->fields, i);
        
        if (!field->name) continue;
        
        if (i > 0) {
            ecs_string_push_cstr(&result, ", ");
        }
        
        ecs_string_push_cstr(&result, field->name);
        ecs_string_push_cstr(&result, ": ");
        
        if (field->simple_type && field->simple_type[0] != '\0') {
            ecs_string_push_cstr(&result, field->simple_type);
        } else {
            ecs_string_push_cstr(&result, "unknown");
        }
    }
    
    ecs_string_push_cstr(&result, " }");
    ecs_string_push(&result, '\0');
    
    return result;
}
