#include "rayflect_parser.h"
#include "rayflect_types.h"
#include "../parsing/ecs_tokenizer.h"
#include "../datastructure/ecs_string.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void rayflect_parse(ecs_struct_t *ecs_struct, const char *def)
{
    if (!ecs_struct || !def) return;

    ecs_tokenizer_t tokenizer;
    ecs_tokenizer_init(&tokenizer, def);
    
    if (!ecs_tokenizer_tokenize(&tokenizer)) {
        fprintf(stderr, "rayflect: tokenize failed\n");
        ecs_tokenizer_free(&tokenizer);
        return;
    }

    ecs_token_t *token = ecs_tokenizer_peek(&tokenizer);
    if (!token) {
        fprintf(stderr, "rayflect: empty definition\n");
        ecs_tokenizer_free(&tokenizer);
        return;
    }

    if (token->type == ECS_TOKEN_IDENTIFIER) {
        const char *keyword = (const char *)token->value.string.data;
        if (strcmp(keyword, "struct") == 0) {
            ecs_tokenizer_advance(&tokenizer);
        }
    }

    if (!ecs_tokenizer_expect(&tokenizer, ECS_TOKEN_LBRACE)) {
        fprintf(stderr, "rayflect: expected '{'\n");
        ecs_tokenizer_free(&tokenizer);
        return;
    }

    while (!ecs_tokenizer_is_done(&tokenizer)) {
        token = ecs_tokenizer_peek(&tokenizer);
        if (!token) break;
        if (token->type == ECS_TOKEN_RBRACE) {
            ecs_tokenizer_advance(&tokenizer);
            break;
        }

        ecs_string_t type_name = ecs_string_new();
        
        while (!ecs_tokenizer_is_done(&tokenizer)) {
            token = ecs_tokenizer_peek(&tokenizer);
            if (!token) break;
            
            if (token->type == ECS_TOKEN_IDENTIFIER) {
                ecs_token_t *next = ecs_tokenizer_peek_ahead(&tokenizer, 1);
                if (next && (next->type == ECS_TOKEN_SEMICOLON ||
                            next->type == ECS_TOKEN_COMMA ||
                            next->type == ECS_TOKEN_LBRACKET)) {
                    break;
                }
                if (type_name.count > 0) ecs_string_push(&type_name, ' ');
                ecs_string_push_cstr(&type_name, (const char *)token->value.string.data);
                ecs_tokenizer_advance(&tokenizer);
            }
            else if (token->type == ECS_TOKEN_WILDCARD) {
                ecs_string_push(&type_name, '*');
                ecs_tokenizer_advance(&tokenizer);
            }
            else break;
        }

        token = ecs_tokenizer_peek(&tokenizer);
        if (!token || token->type != ECS_TOKEN_IDENTIFIER) {
            ecs_vec_free(&type_name);
            break;
        }

        ecs_string_t field_name = ecs_string_new();
        ecs_string_push_cstr(&field_name, (const char *)token->value.string.data);
        ecs_tokenizer_advance(&tokenizer);

        token = ecs_tokenizer_peek(&tokenizer);
        if (token && token->type == ECS_TOKEN_LBRACKET) {
            ecs_tokenizer_advance(&tokenizer);
            token = ecs_tokenizer_peek(&tokenizer);
            if (token && token->type == ECS_TOKEN_NUMBER) {
                ecs_string_push(&type_name, '[');
                char size_buf[32];
                snprintf(size_buf, sizeof(size_buf), "%d", (int)token->value.number);
                ecs_string_push_cstr(&type_name, size_buf);
                ecs_string_push(&type_name, ']');
                ecs_tokenizer_advance(&tokenizer);
            }
            if (!ecs_tokenizer_expect(&tokenizer, ECS_TOKEN_RBRACKET)) {
                ecs_vec_free(&type_name);
                ecs_vec_free(&field_name);
                break;
            }
        }

        if (!ecs_tokenizer_expect(&tokenizer, ECS_TOKEN_SEMICOLON)) {
            ecs_vec_free(&type_name);
            ecs_vec_free(&field_name);
            break;
        }

        ecs_string_push(&type_name, '\0');
        ecs_string_push(&field_name, '\0');

        ecs_field_t field = {
            .name = (const char *)field_name.data,
            .type = (const char *)type_name.data,
            .simple_type = rayflect_type_simplify((const char *)type_name.data),
            .size = rayflect_type_size((const char *)type_name.data),
            .align = rayflect_type_align((const char *)type_name.data)
        };

        ecs_vec_push(&ecs_struct->fields, &field);
    }

    ecs_tokenizer_free(&tokenizer);
}

void rayflect_free(ecs_struct_t *ecs_struct)
{
    if (!ecs_struct) return;
    
    for (size_t i = 0; i < ecs_struct->fields.count; i++) {
        ecs_field_t *field = ECS_VEC_GET(ecs_field_t, &ecs_struct->fields, i);
        if (field->name) free((void *)field->name);
        if (field->type) free((void *)field->type);
        if (field->simple_type) free((void *)field->simple_type);
    }
    
    ecs_vec_free(&ecs_struct->fields);
}
