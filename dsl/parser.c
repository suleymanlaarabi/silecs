#include "parser.h"
#include <stdlib.h>
#include <string.h>

#define INITIAL_QUERY_CAPACITY 8

static void parser_advance(ecs_dsl_parser_t *parser) {
    ecs_dsl_token_free(&parser->current_token);
    parser->current_token = ecs_dsl_lexer_next_token(&parser->lexer);
}

static void identifier_init(ecs_dsl_identifier_t *id) {
    id->first = NULL;
    id->second = NULL;
    id->is_pair = false;
    id->first_wildcard = false;
    id->second_wildcard = false;
}

static void identifier_free(ecs_dsl_identifier_t *id) {
    if (id->first) {
        free(id->first);
        id->first = NULL;
    }
    if (id->second) {
        free(id->second);
        id->second = NULL;
    }
}

static ecs_dsl_query_t *query_create(void) {
    ecs_dsl_query_t *query = malloc(sizeof(ecs_dsl_query_t));
    query->capacity = INITIAL_QUERY_CAPACITY;
    query->count = 0;
    query->terms = malloc(sizeof(ecs_dsl_term_t) * query->capacity);
    return query;
}

static void query_add_term(ecs_dsl_query_t *query, ecs_dsl_term_t term) {
    if (query->count >= query->capacity) {
        query->capacity *= 2;
        query->terms = realloc(query->terms, sizeof(ecs_dsl_term_t) * query->capacity);
    }
    query->terms[query->count++] = term;
}

static bool parse_identifier(ecs_dsl_parser_t *parser, ecs_dsl_identifier_t *id) {
    identifier_init(id);

    if (parser->current_token.type == ECS_DSL_TOKEN_WILDCARD) {
        id->first = strdup("*");
        id->first_wildcard = true;
        parser_advance(parser);
        return true;
    }

    if (parser->current_token.type == ECS_DSL_TOKEN_WILDCARD_ONE) {
        id->first = strdup("_");
        id->first_wildcard = true;
        parser_advance(parser);
        return true;
    }

    if (parser->current_token.type != ECS_DSL_TOKEN_IDENTIFIER) {
        return false;
    }

    id->first = strdup(parser->current_token.value);
    parser_advance(parser);
    return true;
}

static bool parse_pair(ecs_dsl_parser_t *parser, ecs_dsl_identifier_t *id) {
    identifier_init(id);

    if (parser->current_token.type != ECS_DSL_TOKEN_LPAREN) {
        return false;
    }
    parser_advance(parser);

    if (parser->current_token.type == ECS_DSL_TOKEN_WILDCARD) {
        id->first = strdup("*");
        id->first_wildcard = true;
        parser_advance(parser);
    } else if (parser->current_token.type == ECS_DSL_TOKEN_WILDCARD_ONE) {
        id->first = strdup("_");
        id->first_wildcard = true;
        parser_advance(parser);
    } else if (parser->current_token.type == ECS_DSL_TOKEN_IDENTIFIER) {
        id->first = strdup(parser->current_token.value);
        parser_advance(parser);
    } else {
        return false;
    }

    if (parser->current_token.type != ECS_DSL_TOKEN_COMMA) {
        identifier_free(id);
        return false;
    }
    parser_advance(parser);

    if (parser->current_token.type == ECS_DSL_TOKEN_WILDCARD) {
        id->second = strdup("*");
        id->second_wildcard = true;
        parser_advance(parser);
    } else if (parser->current_token.type == ECS_DSL_TOKEN_WILDCARD_ONE) {
        id->second = strdup("_");
        id->second_wildcard = true;
        parser_advance(parser);
    } else if (parser->current_token.type == ECS_DSL_TOKEN_IDENTIFIER) {
        id->second = strdup(parser->current_token.value);
        parser_advance(parser);
    } else {
        identifier_free(id);
        return false;
    }

    if (parser->current_token.type != ECS_DSL_TOKEN_RPAREN) {
        identifier_free(id);
        return false;
    }
    parser_advance(parser);

    id->is_pair = true;
    return true;
}

static bool parse_term(ecs_dsl_parser_t *parser, ecs_dsl_term_t *term) {
    term->op = ECS_DSL_OP_AND;
    term->modifier = ECS_DSL_MOD_NONE;
    term->access = ECS_DSL_ACCESS_DEFAULT;

    if (parser->current_token.type == ECS_DSL_TOKEN_IN) {
        term->access = ECS_DSL_ACCESS_IN;
        parser_advance(parser);
    } else if (parser->current_token.type == ECS_DSL_TOKEN_OUT) {
        term->access = ECS_DSL_ACCESS_OUT;
        parser_advance(parser);
    } else if (parser->current_token.type == ECS_DSL_TOKEN_INOUT) {
        term->access = ECS_DSL_ACCESS_INOUT;
        parser_advance(parser);
    }

    if (parser->current_token.type == ECS_DSL_TOKEN_NOT) {
        term->modifier = ECS_DSL_MOD_NOT;
        parser_advance(parser);
    } else if (parser->current_token.type == ECS_DSL_TOKEN_OPTIONAL) {
        term->modifier = ECS_DSL_MOD_OPTIONAL;
        parser_advance(parser);
    }

    if (parser->current_token.type == ECS_DSL_TOKEN_LPAREN) {
        if (!parse_pair(parser, &term->id)) {
            return false;
        }
    } else {
        if (!parse_identifier(parser, &term->id)) {
            return false;
        }
    }

    return true;
}

void ecs_dsl_parser_init(ecs_dsl_parser_t *parser, const char *input) {
    ecs_dsl_lexer_init(&parser->lexer, input);
    parser->current_token = ecs_dsl_lexer_next_token(&parser->lexer);
}

ecs_dsl_query_t *ecs_dsl_parser_parse(ecs_dsl_parser_t *parser) {
    ecs_dsl_query_t *query = query_create();

    while (parser->current_token.type != ECS_DSL_TOKEN_EOF) {
        ecs_dsl_term_t term;
        if (!parse_term(parser, &term)) {
            ecs_dsl_query_free(query);
            return NULL;
        }

        query_add_term(query, term);

        if (parser->current_token.type == ECS_DSL_TOKEN_OR) {
            parser_advance(parser);
            if (query->count > 0) {
                query->terms[query->count - 1].op = ECS_DSL_OP_OR;
            }
        } else if (parser->current_token.type == ECS_DSL_TOKEN_COMMA) {
            parser_advance(parser);
        } else if (parser->current_token.type == ECS_DSL_TOKEN_EOF) {
            break;
        } else {
            break;
        }
    }

    return query;
}

void ecs_dsl_query_free(ecs_dsl_query_t *query) {
    if (query) {
        for (size_t i = 0; i < query->count; i++) {
            identifier_free(&query->terms[i].id);
        }
        free(query->terms);
        free(query);
    }
}

void ecs_dsl_parser_free(ecs_dsl_parser_t *parser) {
    ecs_dsl_token_free(&parser->current_token);
}
