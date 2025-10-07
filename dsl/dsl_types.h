#ifndef DSL_TYPES_H
#define DSL_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    ECS_DSL_TOKEN_IDENTIFIER,
    ECS_DSL_TOKEN_COMMA,           // ,
    ECS_DSL_TOKEN_OR,              // ||
    ECS_DSL_TOKEN_NOT,             // !
    ECS_DSL_TOKEN_OPTIONAL,        // ?
    ECS_DSL_TOKEN_LPAREN,          // (
    ECS_DSL_TOKEN_RPAREN,          // )
    ECS_DSL_TOKEN_WILDCARD,        // *
    ECS_DSL_TOKEN_WILDCARD_ONE,    // _
    ECS_DSL_TOKEN_IN,              // [in]
    ECS_DSL_TOKEN_OUT,             // [out]
    ECS_DSL_TOKEN_INOUT,           // [inout]
    ECS_DSL_TOKEN_EOF,
    ECS_DSL_TOKEN_ERROR
} ecs_dsl_token_type_t;

typedef struct {
    ecs_dsl_token_type_t type;
    char *value;
    size_t length;
} ecs_dsl_token_t;

typedef enum {
    ECS_DSL_MOD_NONE,
    ECS_DSL_MOD_NOT,         // !
    ECS_DSL_MOD_OPTIONAL     // ?
} ecs_dsl_term_modifier_t;

typedef enum {
    ECS_DSL_OP_AND,          // default (comma)
    ECS_DSL_OP_OR            // ||
} ecs_dsl_term_operator_t;

typedef enum {
    ECS_DSL_ACCESS_DEFAULT,
    ECS_DSL_ACCESS_IN,
    ECS_DSL_ACCESS_OUT,
    ECS_DSL_ACCESS_INOUT
} ecs_dsl_access_mode_t;

typedef struct {
    char *first;           // First identifier (or only one if not a pair)
    char *second;          // Second identifier (NULL if not a pair)
    bool is_pair;
    bool first_wildcard;   // True if first is *
    bool second_wildcard;  // True if second is *
} ecs_dsl_identifier_t;

typedef struct {
    ecs_dsl_identifier_t id;
    ecs_dsl_term_modifier_t modifier;  // NOT or OPTIONAL
    ecs_dsl_term_operator_t op;        // AND or OR
    ecs_dsl_access_mode_t access;
} ecs_dsl_term_t;

typedef struct {
    ecs_dsl_term_t *terms;
    size_t count;
    size_t capacity;
} ecs_dsl_query_t;

#endif
