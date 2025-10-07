#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void lexer_advance(ecs_dsl_lexer_t *lexer) {
    if (lexer->pos < lexer->length) {
        lexer->pos++;
        lexer->current = (lexer->pos < lexer->length) ? lexer->input[lexer->pos] : '\0';
    } else {
        lexer->current = '\0';
    }
}

static void lexer_skip_whitespace(ecs_dsl_lexer_t *lexer) {
    while (lexer->current && isspace(lexer->current)) {
        lexer_advance(lexer);
    }
}

static char lexer_peek(ecs_dsl_lexer_t *lexer) {
    if (lexer->pos + 1 < lexer->length) {
        return lexer->input[lexer->pos + 1];
    }
    return '\0';
}

static ecs_dsl_token_t make_token(ecs_dsl_token_type_t type, const char *value, size_t length) {
    ecs_dsl_token_t token;
    token.type = type;
    token.length = length;
    if (value && length > 0) {
        token.value = malloc(length + 1);
        memcpy(token.value, value, length);
        token.value[length] = '\0';
    } else {
        token.value = NULL;
    }
    return token;
}

static ecs_dsl_token_t lexer_read_identifier(ecs_dsl_lexer_t *lexer) {
    size_t start = lexer->pos;
    while (lexer->current && (isalnum(lexer->current) || lexer->current == '_')) {
        lexer_advance(lexer);
    }
    size_t length = lexer->pos - start;
    return make_token(ECS_DSL_TOKEN_IDENTIFIER, &lexer->input[start], length);
}

static ecs_dsl_token_t lexer_read_access_mode(ecs_dsl_lexer_t *lexer) {
    lexer_advance(lexer);

    size_t start = lexer->pos;
    while (lexer->current && lexer->current != ']') {
        lexer_advance(lexer);
    }

    size_t length = lexer->pos - start;

    if (lexer->current == ']') {
        lexer_advance(lexer);
    }

    if (length == 2 && strncmp(&lexer->input[start], "in", 2) == 0) {
        return make_token(ECS_DSL_TOKEN_IN, "in", 2);
    } else if (length == 3 && strncmp(&lexer->input[start], "out", 3) == 0) {
        return make_token(ECS_DSL_TOKEN_OUT, "out", 3);
    } else if (length == 5 && strncmp(&lexer->input[start], "inout", 5) == 0) {
        return make_token(ECS_DSL_TOKEN_INOUT, "inout", 5);
    }

    return make_token(ECS_DSL_TOKEN_ERROR, NULL, 0);
}

void ecs_dsl_lexer_init(ecs_dsl_lexer_t *lexer, const char *input) {
    lexer->input = input;
    lexer->pos = 0;
    lexer->length = strlen(input);
    lexer->current = (lexer->length > 0) ? input[0] : '\0';
}

ecs_dsl_token_t ecs_dsl_lexer_next_token(ecs_dsl_lexer_t *lexer) {
    lexer_skip_whitespace(lexer);

    if (lexer->current == '\0') {
        return make_token(ECS_DSL_TOKEN_EOF, NULL, 0);
    }

    switch (lexer->current) {
        case ',':
            lexer_advance(lexer);
            return make_token(ECS_DSL_TOKEN_COMMA, ",", 1);

        case '!':
            lexer_advance(lexer);
            return make_token(ECS_DSL_TOKEN_NOT, "!", 1);

        case '?':
            lexer_advance(lexer);
            return make_token(ECS_DSL_TOKEN_OPTIONAL, "?", 1);

        case '(':
            lexer_advance(lexer);
            return make_token(ECS_DSL_TOKEN_LPAREN, "(", 1);

        case ')':
            lexer_advance(lexer);
            return make_token(ECS_DSL_TOKEN_RPAREN, ")", 1);

        case '*':
            lexer_advance(lexer);
            return make_token(ECS_DSL_TOKEN_WILDCARD, "*", 1);

        case '_':
            lexer_advance(lexer);
            return make_token(ECS_DSL_TOKEN_WILDCARD_ONE, "_", 1);

        case '|':
            if (lexer_peek(lexer) == '|') {
                lexer_advance(lexer);
                lexer_advance(lexer);
                return make_token(ECS_DSL_TOKEN_OR, "||", 2);
            }
            lexer_advance(lexer);
            return make_token(ECS_DSL_TOKEN_ERROR, NULL, 0);

        case '[':
            return lexer_read_access_mode(lexer);
    }

    if (isalpha(lexer->current) || lexer->current == '_') {
        return lexer_read_identifier(lexer);
    }

    lexer_advance(lexer);
    return make_token(ECS_DSL_TOKEN_ERROR, NULL, 0);
}

void ecs_dsl_token_free(ecs_dsl_token_t *token) {
    if (token->value) {
        free(token->value);
        token->value = NULL;
    }
}
