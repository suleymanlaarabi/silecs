#include "rayflect_format.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

static void print_field_value(const ecs_field_t *field, const void *data, size_t offset)
{
    if (!data) {
        return;
    }

    const char *field_data = (const char *)data + offset;

    switch (field->type) {
        case ECS_TYPE_I8:
            printf("%d", *(int8_t *)field_data);
            break;
        case ECS_TYPE_I16:
            printf("%d", *(int16_t *)field_data);
            break;
        case ECS_TYPE_I32:
            printf("%d", *(int32_t *)field_data);
            break;
        case ECS_TYPE_I64:
            printf("%lld", *(long long *)field_data);
            break;
        case ECS_TYPE_U8:
            printf("%u", *(uint8_t *)field_data);
            break;
        case ECS_TYPE_U16:
            printf("%u", *(uint16_t *)field_data);
            break;
        case ECS_TYPE_U32:
            printf("%u", *(uint32_t *)field_data);
            break;
        case ECS_TYPE_U64:
            printf("%llu", *(unsigned long long *)field_data);
            break;
        case ECS_TYPE_F32:
            printf("%f", *(float *)field_data);
            break;
        case ECS_TYPE_F64:
            printf("%f", *(double *)field_data);
            break;
        case ECS_TYPE_BOOL:
            printf("%s", *(bool *)field_data ? "true" : "false");
            break;
        case ECS_TYPE_PTR: {
            void *ptr = *(void **)field_data;
            if (ptr) {
                printf("%p", ptr);
            } else {
                printf("NULL");
            }
            break;
        }
        case ECS_TYPE_ARRAY:
            printf("[array]");
            break;
        case ECS_TYPE_CUSTOM:
        case ECS_TYPE_UNKNOWN:
        default:
            printf("(unsupported type)");
            break;
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
               rayflect_type_to_string(field->type, field->array_size));

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
        ecs_string_push_cstr(&result, rayflect_type_to_string(field->type, field->array_size));
    }

    ecs_string_push_cstr(&result, " }");
    ecs_string_push(&result, '\0');

    return result;
}
