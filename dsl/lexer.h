#ifndef DSL_LEXER_H
    #define DSL_LEXER_H
    #include "dsl_types.h"

typedef struct {
    const char *input;
    size_t pos;
    size_t length;
    char current;
} ecs_dsl_lexer_t;

void ecs_dsl_lexer_init(ecs_dsl_lexer_t *lexer, const char *input);

ecs_dsl_token_t ecs_dsl_lexer_next_token(ecs_dsl_lexer_t *lexer);

void ecs_dsl_token_free(ecs_dsl_token_t *token);

#endif
