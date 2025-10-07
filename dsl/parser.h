#ifndef DSL_PARSER_H
#define DSL_PARSER_H

#include "dsl_types.h"
#include "lexer.h"

typedef struct {
    ecs_dsl_lexer_t lexer;
    ecs_dsl_token_t current_token;
} ecs_dsl_parser_t;

void ecs_dsl_parser_init(ecs_dsl_parser_t *parser, const char *input);

ecs_dsl_query_t *ecs_dsl_parser_parse(ecs_dsl_parser_t *parser);

void ecs_dsl_query_free(ecs_dsl_query_t *query);

void ecs_dsl_parser_free(ecs_dsl_parser_t *parser);

#endif
