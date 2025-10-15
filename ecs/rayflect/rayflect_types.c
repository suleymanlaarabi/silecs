#include "rayflect_types.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

char* rayflect_type_simplify(const char *c_type)
{
    if (!c_type) return strdup("unknown");
    
    if (strcmp(c_type, "int") == 0) return strdup("i32");
    if (strcmp(c_type, "unsigned int") == 0) return strdup("u32");
    if (strcmp(c_type, "short") == 0) return strdup("i16");
    if (strcmp(c_type, "unsigned short") == 0) return strdup("u16");
    if (strcmp(c_type, "long") == 0) return strdup("i64");
    if (strcmp(c_type, "unsigned long") == 0) return strdup("u64");
    if (strcmp(c_type, "char") == 0) return strdup("i8");
    if (strcmp(c_type, "unsigned char") == 0) return strdup("u8");
    if (strcmp(c_type, "float") == 0) return strdup("f32");
    if (strcmp(c_type, "double") == 0) return strdup("f64");
    if (strcmp(c_type, "bool") == 0) return strdup("bool");
    if (strcmp(c_type, "_Bool") == 0) return strdup("bool");
    
    if (strstr(c_type, "*")) return strdup("ptr");
    
    if (strstr(c_type, "[")) {
        char *bracket = strchr(c_type, '[');
        if (bracket) {
            int array_size = atoi(bracket + 1);
            char result[64];
            snprintf(result, sizeof(result), "array[%d]", array_size);
            return strdup(result);
        }
        return strdup("array");
    }
    
    return strdup(c_type);
}

const char* rayflect_type_simplify_static(const char *c_type)
{
    if (!c_type) return "unknown";
    
    if (strcmp(c_type, "int") == 0) return "i32";
    if (strcmp(c_type, "unsigned int") == 0) return "u32";
    if (strcmp(c_type, "short") == 0) return "i16";
    if (strcmp(c_type, "unsigned short") == 0) return "u16";
    if (strcmp(c_type, "long") == 0) return "i64";
    if (strcmp(c_type, "unsigned long") == 0) return "u64";
    if (strcmp(c_type, "char") == 0) return "i8";
    if (strcmp(c_type, "unsigned char") == 0) return "u8";
    if (strcmp(c_type, "float") == 0) return "f32";
    if (strcmp(c_type, "double") == 0) return "f64";
    if (strcmp(c_type, "bool") == 0) return "bool";
    if (strcmp(c_type, "_Bool") == 0) return "bool";
    
    if (strstr(c_type, "*")) return "ptr";
    if (strstr(c_type, "[")) return "array";
    
    return c_type;
}
