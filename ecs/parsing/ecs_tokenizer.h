#ifndef ECS_TOKENIZER_H
#define ECS_TOKENIZER_H

#include "ecs_string.h"
#include "ecs_scanner.h"
#include "../datastructure/ecs_vec.h"
#include <stdbool.h>

typedef enum {
    ECS_TOKEN_IDENTIFIER,
    ECS_TOKEN_NUMBER,
    ECS_TOKEN_STRING,
    ECS_TOKEN_AND,
    ECS_TOKEN_OR,
    ECS_TOKEN_NOT,
    ECS_TOKEN_EQ,
    ECS_TOKEN_NEQ,
    ECS_TOKEN_LT,
    ECS_TOKEN_GT,
    ECS_TOKEN_LTE,
    ECS_TOKEN_GTE,
    ECS_TOKEN_LPAREN,
    ECS_TOKEN_RPAREN,
    ECS_TOKEN_LBRACKET,
    ECS_TOKEN_RBRACKET,
    ECS_TOKEN_LBRACE,
    ECS_TOKEN_RBRACE,
    ECS_TOKEN_COMMA,
    ECS_TOKEN_DOT,
    ECS_TOKEN_COLON,
    ECS_TOKEN_SEMICOLON,
    ECS_TOKEN_PIPE,
    ECS_TOKEN_OPTIONAL,
    ECS_TOKEN_WILDCARD,
    ECS_TOKEN_WILDCARD_ONE,
    ECS_TOKEN_IN,
    ECS_TOKEN_OUT,
    ECS_TOKEN_INOUT,
    ECS_TOKEN_WITH,
    ECS_TOKEN_WITHOUT,
    ECS_TOKEN_EOF,
    ECS_TOKEN_ERROR,
    ECS_TOKEN_WHITESPACE,
    ECS_TOKEN_COMMENT,
} ecs_token_type_t;

typedef struct {
    ecs_token_type_t type;
    union {
        ecs_string_t string;
        double number;
        char op;
    } value;
    uint64_t line;
    uint64_t column;
    uint64_t position;
    uint64_t length;
} ecs_token_t;

typedef struct {
    ecs_scanner_t scanner;
    ecs_vec_t tokens;
    uint64_t current;
    uint64_t line;
    uint64_t column;
    bool skip_whitespace;
    bool skip_comments;
} ecs_tokenizer_t;

void ecs_tokenizer_init(ecs_tokenizer_t *tokenizer, const char *source);
void ecs_tokenizer_free(ecs_tokenizer_t *tokenizer);

void ecs_tokenizer_set_skip_whitespace(ecs_tokenizer_t *tokenizer, bool skip);
void ecs_tokenizer_set_skip_comments(ecs_tokenizer_t *tokenizer, bool skip);

bool ecs_tokenizer_tokenize(ecs_tokenizer_t *tokenizer);
ecs_token_t *ecs_tokenizer_next_token(ecs_tokenizer_t *tokenizer);

ecs_token_t *ecs_tokenizer_peek(const ecs_tokenizer_t *tokenizer);
ecs_token_t *ecs_tokenizer_peek_ahead(const ecs_tokenizer_t *tokenizer, uint64_t offset);
ecs_token_t *ecs_tokenizer_advance(ecs_tokenizer_t *tokenizer);
bool ecs_tokenizer_is_done(const ecs_tokenizer_t *tokenizer);
bool ecs_tokenizer_match(ecs_tokenizer_t *tokenizer, ecs_token_type_t type);
bool ecs_tokenizer_expect(ecs_tokenizer_t *tokenizer, ecs_token_type_t type);

ecs_token_t ecs_token_create(ecs_token_type_t type, uint64_t line, uint64_t column, uint64_t position);
ecs_token_t ecs_token_create_identifier(ecs_string_t str, uint64_t line, uint64_t column, uint64_t position);
ecs_token_t ecs_token_create_number(double number, uint64_t line, uint64_t column, uint64_t position);
ecs_token_t ecs_token_create_string(ecs_string_t str, uint64_t line, uint64_t column, uint64_t position);
ecs_token_t ecs_token_create_error(ecs_string_t msg, uint64_t line, uint64_t column, uint64_t position);

const char *ecs_token_type_to_string(ecs_token_type_t type);
void ecs_token_print(const ecs_token_t *token);
void ecs_token_free(ecs_token_t *token);

#endif
