#include "rayflect_types.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

size_t rayflect_type_size(const char *type_str)
{
    if (!type_str || type_str[0] == '\0') return 0;
    
    if (strcmp(type_str, "char") == 0 || strcmp(type_str, "unsigned char") == 0 || strcmp(type_str, "signed char") == 0) return 1;
    if (strcmp(type_str, "short") == 0 || strcmp(type_str, "unsigned short") == 0) return 2;
    if (strcmp(type_str, "int") == 0 || strcmp(type_str, "unsigned int") == 0) return 4;
    if (strcmp(type_str, "long") == 0 || strcmp(type_str, "unsigned long") == 0) return sizeof(long);
    if (strcmp(type_str, "long long") == 0 || strcmp(type_str, "unsigned long long") == 0) return 8;
    if (strcmp(type_str, "float") == 0) return 4;
    if (strcmp(type_str, "double") == 0) return 8;
    if (strcmp(type_str, "bool") == 0 || strcmp(type_str, "_Bool") == 0) return 1;
    
    if (strstr(type_str, "*")) return sizeof(void*);
    
    if (strstr(type_str, "[")) {
        char *bracket = strchr(type_str, '[');
        if (bracket) {
            int array_size = atoi(bracket + 1);
            char base_type[256] = {0};
            size_t base_len = bracket - type_str;
            if (base_len < sizeof(base_type)) {
                strncpy(base_type, type_str, base_len);
                while (base_len > 0 && base_type[base_len - 1] == ' ') {
                    base_type[--base_len] = '\0';
                }
                size_t element_size = rayflect_type_size(base_type);
                return element_size * array_size;
            }
        }
    }
    
    return 0;
}

size_t rayflect_type_align(const char *type_str)
{
    if (!type_str || type_str[0] == '\0') return 0;
    
    if (strcmp(type_str, "char") == 0 || strcmp(type_str, "unsigned char") == 0 || strcmp(type_str, "signed char") == 0) return 1;
    if (strcmp(type_str, "short") == 0 || strcmp(type_str, "unsigned short") == 0) return 2;
    if (strcmp(type_str, "int") == 0 || strcmp(type_str, "unsigned int") == 0) return 4;
    if (strcmp(type_str, "long") == 0 || strcmp(type_str, "unsigned long") == 0) return sizeof(long);
    if (strcmp(type_str, "long long") == 0 || strcmp(type_str, "unsigned long long") == 0) return 8;
    if (strcmp(type_str, "float") == 0) return 4;
    if (strcmp(type_str, "double") == 0) return 8;
    if (strcmp(type_str, "bool") == 0 || strcmp(type_str, "_Bool") == 0) return 1;
    
    if (strstr(type_str, "*")) return sizeof(void*);
    
    if (strstr(type_str, "[")) {
        char *bracket = strchr(type_str, '[');
        if (bracket) {
            char base_type[256] = {0};
            size_t base_len = bracket - type_str;
            if (base_len < sizeof(base_type)) {
                strncpy(base_type, type_str, base_len);
                while (base_len > 0 && base_type[base_len - 1] == ' ') {
                    base_type[--base_len] = '\0';
                }
                return rayflect_type_align(base_type);
            }
        }
    }
    
    return 0;
}

ecs_simple_type_t rayflect_type_simplify(const char *c_type, int *out_array_size)
{
    if (out_array_size) *out_array_size = 0;
    
    if (!c_type) return ECS_TYPE_UNKNOWN;
    
    if (strcmp(c_type, "int") == 0) return ECS_TYPE_I32;
    if (strcmp(c_type, "unsigned int") == 0) return ECS_TYPE_U32;
    if (strcmp(c_type, "short") == 0) return ECS_TYPE_I16;
    if (strcmp(c_type, "unsigned short") == 0) return ECS_TYPE_U16;
    if (strcmp(c_type, "long") == 0) return ECS_TYPE_I64;
    if (strcmp(c_type, "unsigned long") == 0) return ECS_TYPE_U64;
    if (strcmp(c_type, "char") == 0) return ECS_TYPE_I8;
    if (strcmp(c_type, "unsigned char") == 0) return ECS_TYPE_U8;
    if (strcmp(c_type, "float") == 0) return ECS_TYPE_F32;
    if (strcmp(c_type, "double") == 0) return ECS_TYPE_F64;
    if (strcmp(c_type, "bool") == 0) return ECS_TYPE_BOOL;
    if (strcmp(c_type, "_Bool") == 0) return ECS_TYPE_BOOL;
    
    if (strstr(c_type, "*")) return ECS_TYPE_PTR;
    
    if (strstr(c_type, "[")) {
        char *bracket = strchr(c_type, '[');
        if (bracket && out_array_size) {
            *out_array_size = atoi(bracket + 1);
        }
        return ECS_TYPE_ARRAY;
    }
    
    return ECS_TYPE_CUSTOM;
}

const char* rayflect_type_to_string(ecs_simple_type_t type, int array_size)
{
    static char array_buf[64];
    
    switch (type) {
        case ECS_TYPE_I8:   return "i8";
        case ECS_TYPE_I16:  return "i16";
        case ECS_TYPE_I32:  return "i32";
        case ECS_TYPE_I64:  return "i64";
        case ECS_TYPE_U8:   return "u8";
        case ECS_TYPE_U16:  return "u16";
        case ECS_TYPE_U32:  return "u32";
        case ECS_TYPE_U64:  return "u64";
        case ECS_TYPE_F32:  return "f32";
        case ECS_TYPE_F64:  return "f64";
        case ECS_TYPE_BOOL: return "bool";
        case ECS_TYPE_PTR:  return "ptr";
        case ECS_TYPE_ARRAY:
            if (array_size > 0) {
                snprintf(array_buf, sizeof(array_buf), "array[%d]", array_size);
                return array_buf;
            }
            return "array";
        case ECS_TYPE_CUSTOM: return "custom";
        case ECS_TYPE_UNKNOWN:
        default:
            return "unknown";
    }
}

int rayflect_set_field(const ecs_struct_t *ecs_struct, void *instance, const char *field_name, const void *value)
{
    if (!ecs_struct || !instance || !field_name || !value) {
        return -1;
    }

    size_t offset = 0;
    for (size_t i = 0; i < ecs_struct->fields.count; i++) {
        const ecs_field_t *field = ECS_VEC_GET(ecs_field_t, &ecs_struct->fields, i);
        
        if (field->align > 0 && offset % field->align != 0) {
            offset += field->align - (offset % field->align);
        }

        if (field->name && strcmp(field->name, field_name) == 0) {
            char *field_ptr = (char *)instance + offset;
            memcpy(field_ptr, value, field->size);
            return 0;
        }

        offset += field->size;
    }

    return -1;
}
