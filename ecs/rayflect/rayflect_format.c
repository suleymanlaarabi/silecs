#include "rayflect_format.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

static void print_field_value(const ecs_field_t *field, const void *data, size_t offset)
{
    if (!field->simple_type || !data) {
        return;
    }

    const char *field_data = (const char *)data + offset;
    const char *type = field->simple_type;

    if (strcmp(type, "i8") == 0) {
        printf("%d", *(int8_t *)field_data);
    } else if (strcmp(type, "i16") == 0) {
        printf("%d", *(int16_t *)field_data);
    } else if (strcmp(type, "i32") == 0) {
        printf("%d", *(int32_t *)field_data);
    } else if (strcmp(type, "i64") == 0) {
        printf("%lld", *(long long *)field_data);
    } else if (strcmp(type, "u8") == 0) {
        printf("%u", *(uint8_t *)field_data);
    } else if (strcmp(type, "u16") == 0) {
        printf("%u", *(uint16_t *)field_data);
    } else if (strcmp(type, "u32") == 0) {
        printf("%u", *(uint32_t *)field_data);
    } else if (strcmp(type, "u64") == 0) {
        printf("%llu", *(unsigned long long *)field_data);
    } else if (strcmp(type, "f32") == 0) {
        printf("%f", *(float *)field_data);
    } else if (strcmp(type, "f64") == 0) {
        printf("%f", *(double *)field_data);
    } else if (strcmp(type, "ptr") == 0) {
        void *ptr = *(void **)field_data;
        if (ptr) {
            printf("%p", ptr);
        } else {
            printf("NULL");
        }
    } else if (strncmp(type, "array[", 6) == 0) {
        printf("[array]");
    } else {
        printf("(unsupported type)");
    }
}

void rayflect_print(const ecs_struct_t *ecs_struct, const void *data)
{
    if (!ecs_struct) {
        printf("NULL struct\n");
        return;
    }

    printf("struct {\n");
    size_t offset = 0;

    for (size_t i = 0; i < ecs_struct->fields.count; i++) {
        const ecs_field_t *field = ECS_VEC_GET(ecs_field_t, &ecs_struct->fields, i);

        // Align offset
        if (field->align > 0 && offset % field->align != 0) {
            offset += field->align - (offset % field->align);
        }

        printf("  [%zu] %s: %s",
               i,
               field->name ? field->name : "(null)",
               field->simple_type ? field->simple_type : "unknown");

        if (data) {
            printf(" = ");
            print_field_value(field, data, offset);
        }

        printf(" (size: %zu, align: %zu)\n",
               field->size,
               field->align);

        offset += field->size;
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
