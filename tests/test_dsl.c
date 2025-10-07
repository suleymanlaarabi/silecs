#include <criterion/criterion.h>
#include "../dsl/parser.h"
#include <string.h>

Test(dsl_lexer, basic_tokens) {
    ecs_dsl_lexer_t lexer;
    ecs_dsl_lexer_init(&lexer, "Position, Velocity");

    ecs_dsl_token_t tok1 = ecs_dsl_lexer_next_token(&lexer);
    cr_assert_eq(tok1.type, ECS_DSL_TOKEN_IDENTIFIER);
    cr_assert_str_eq(tok1.value, "Position");
    ecs_dsl_token_free(&tok1);

    ecs_dsl_token_t tok2 = ecs_dsl_lexer_next_token(&lexer);
    cr_assert_eq(tok2.type, ECS_DSL_TOKEN_COMMA);
    ecs_dsl_token_free(&tok2);

    ecs_dsl_token_t tok3 = ecs_dsl_lexer_next_token(&lexer);
    cr_assert_eq(tok3.type, ECS_DSL_TOKEN_IDENTIFIER);
    cr_assert_str_eq(tok3.value, "Velocity");
    ecs_dsl_token_free(&tok3);

    ecs_dsl_token_t tok4 = ecs_dsl_lexer_next_token(&lexer);
    cr_assert_eq(tok4.type, ECS_DSL_TOKEN_EOF);
    ecs_dsl_token_free(&tok4);
}

Test(dsl_lexer, operators) {
    ecs_dsl_lexer_t lexer;
    ecs_dsl_lexer_init(&lexer, "!Position, ?Velocity, A || B");

    ecs_dsl_token_t tok1 = ecs_dsl_lexer_next_token(&lexer);
    cr_assert_eq(tok1.type, ECS_DSL_TOKEN_NOT);
    ecs_dsl_token_free(&tok1);

    ecs_dsl_token_t tok2 = ecs_dsl_lexer_next_token(&lexer);
    cr_assert_eq(tok2.type, ECS_DSL_TOKEN_IDENTIFIER);
    ecs_dsl_token_free(&tok2);

    ecs_dsl_lexer_next_token(&lexer);

    ecs_dsl_token_t tok3 = ecs_dsl_lexer_next_token(&lexer);
    cr_assert_eq(tok3.type, ECS_DSL_TOKEN_OPTIONAL);
    ecs_dsl_token_free(&tok3);

    ecs_dsl_lexer_next_token(&lexer);
    ecs_dsl_lexer_next_token(&lexer);
    ecs_dsl_lexer_next_token(&lexer);

    ecs_dsl_token_t tok4 = ecs_dsl_lexer_next_token(&lexer);
    cr_assert_eq(tok4.type, ECS_DSL_TOKEN_OR);
    ecs_dsl_token_free(&tok4);
}

Test(dsl_lexer, wildcards) {
    ecs_dsl_lexer_t lexer;
    ecs_dsl_lexer_init(&lexer, "*, _");

    ecs_dsl_token_t tok1 = ecs_dsl_lexer_next_token(&lexer);
    cr_assert_eq(tok1.type, ECS_DSL_TOKEN_WILDCARD);
    ecs_dsl_token_free(&tok1);

    ecs_dsl_lexer_next_token(&lexer);

    ecs_dsl_token_t tok2 = ecs_dsl_lexer_next_token(&lexer);
    cr_assert_eq(tok2.type, ECS_DSL_TOKEN_WILDCARD_ONE);
    ecs_dsl_token_free(&tok2);
}

Test(dsl_lexer, access_modes) {
    ecs_dsl_lexer_t lexer;
    ecs_dsl_lexer_init(&lexer, "[in] [out] [inout]");

    ecs_dsl_token_t tok1 = ecs_dsl_lexer_next_token(&lexer);
    cr_assert_eq(tok1.type, ECS_DSL_TOKEN_IN);
    ecs_dsl_token_free(&tok1);

    ecs_dsl_token_t tok2 = ecs_dsl_lexer_next_token(&lexer);
    cr_assert_eq(tok2.type, ECS_DSL_TOKEN_OUT);
    ecs_dsl_token_free(&tok2);

    ecs_dsl_token_t tok3 = ecs_dsl_lexer_next_token(&lexer);
    cr_assert_eq(tok3.type, ECS_DSL_TOKEN_INOUT);
    ecs_dsl_token_free(&tok3);
}

Test(dsl_lexer, parentheses) {
    ecs_dsl_lexer_t lexer;
    ecs_dsl_lexer_init(&lexer, "(Likes, Bob)");

    ecs_dsl_token_t tok1 = ecs_dsl_lexer_next_token(&lexer);
    cr_assert_eq(tok1.type, ECS_DSL_TOKEN_LPAREN);
    ecs_dsl_token_free(&tok1);

    ecs_dsl_lexer_next_token(&lexer);
    ecs_dsl_lexer_next_token(&lexer);
    ecs_dsl_lexer_next_token(&lexer);

    ecs_dsl_token_t tok2 = ecs_dsl_lexer_next_token(&lexer);
    cr_assert_eq(tok2.type, ECS_DSL_TOKEN_RPAREN);
    ecs_dsl_token_free(&tok2);
}

Test(dsl_parser, simple_query) {
    ecs_dsl_parser_t parser;
    ecs_dsl_parser_init(&parser, "Position, Velocity");

    ecs_dsl_query_t *query = ecs_dsl_parser_parse(&parser);
    cr_assert_not_null(query);
    cr_assert_eq(query->count, 2);

    cr_assert_str_eq(query->terms[0].id.first, "Position");
    cr_assert_eq(query->terms[0].id.is_pair, false);
    cr_assert_eq(query->terms[0].op, ECS_DSL_OP_AND);

    cr_assert_str_eq(query->terms[1].id.first, "Velocity");
    cr_assert_eq(query->terms[1].id.is_pair, false);
    cr_assert_eq(query->terms[1].op, ECS_DSL_OP_AND);

    ecs_dsl_query_free(query);
    ecs_dsl_parser_free(&parser);
}

Test(dsl_parser, not_operator) {
    ecs_dsl_parser_t parser;
    ecs_dsl_parser_init(&parser, "Position, !Velocity");

    ecs_dsl_query_t *query = ecs_dsl_parser_parse(&parser);
    cr_assert_not_null(query);
    cr_assert_eq(query->count, 2);

    cr_assert_eq(query->terms[1].modifier, ECS_DSL_MOD_NOT);
    cr_assert_str_eq(query->terms[1].id.first, "Velocity");

    ecs_dsl_query_free(query);
    ecs_dsl_parser_free(&parser);
}

Test(dsl_parser, optional_operator) {
    ecs_dsl_parser_t parser;
    ecs_dsl_parser_init(&parser, "Position, ?Velocity");

    ecs_dsl_query_t *query = ecs_dsl_parser_parse(&parser);
    cr_assert_not_null(query);
    cr_assert_eq(query->count, 2);

    cr_assert_eq(query->terms[1].modifier, ECS_DSL_MOD_OPTIONAL);
    cr_assert_str_eq(query->terms[1].id.first, "Velocity");

    ecs_dsl_query_free(query);
    ecs_dsl_parser_free(&parser);
}

Test(dsl_parser, or_operator) {
    ecs_dsl_parser_t parser;
    ecs_dsl_parser_init(&parser, "Position || Velocity");

    ecs_dsl_query_t *query = ecs_dsl_parser_parse(&parser);
    cr_assert_not_null(query);
    cr_assert_eq(query->count, 2);

    cr_assert_eq(query->terms[0].op, ECS_DSL_OP_OR);
    cr_assert_str_eq(query->terms[0].id.first, "Position");

    ecs_dsl_query_free(query);
    ecs_dsl_parser_free(&parser);
}

Test(dsl_parser, pair) {
    ecs_dsl_parser_t parser;
    ecs_dsl_parser_init(&parser, "(Likes, Bob)");

    ecs_dsl_query_t *query = ecs_dsl_parser_parse(&parser);
    cr_assert_not_null(query);
    cr_assert_eq(query->count, 1);

    cr_assert_eq(query->terms[0].id.is_pair, true);
    cr_assert_str_eq(query->terms[0].id.first, "Likes");
    cr_assert_str_eq(query->terms[0].id.second, "Bob");

    ecs_dsl_query_free(query);
    ecs_dsl_parser_free(&parser);
}

Test(dsl_parser, pair_wildcard) {
    ecs_dsl_parser_t parser;
    ecs_dsl_parser_init(&parser, "(Likes, *)");

    ecs_dsl_query_t *query = ecs_dsl_parser_parse(&parser);
    cr_assert_not_null(query);
    cr_assert_eq(query->count, 1);

    cr_assert_eq(query->terms[0].id.is_pair, true);
    cr_assert_str_eq(query->terms[0].id.first, "Likes");
    cr_assert_str_eq(query->terms[0].id.second, "*");
    cr_assert_eq(query->terms[0].id.second_wildcard, true);

    ecs_dsl_query_free(query);
    ecs_dsl_parser_free(&parser);
}

Test(dsl_parser, pair_double_wildcard) {
    ecs_dsl_parser_t parser;
    ecs_dsl_parser_init(&parser, "(*, *)");

    ecs_dsl_query_t *query = ecs_dsl_parser_parse(&parser);
    cr_assert_not_null(query);
    cr_assert_eq(query->count, 1);

    cr_assert_eq(query->terms[0].id.is_pair, true);
    cr_assert_eq(query->terms[0].id.first_wildcard, true);
    cr_assert_eq(query->terms[0].id.second_wildcard, true);

    ecs_dsl_query_free(query);
    ecs_dsl_parser_free(&parser);
}

Test(dsl_parser, access_mode) {
    ecs_dsl_parser_t parser;
    ecs_dsl_parser_init(&parser, "[in] Position, [out] Velocity");

    ecs_dsl_query_t *query = ecs_dsl_parser_parse(&parser);
    cr_assert_not_null(query);
    cr_assert_eq(query->count, 2);

    cr_assert_eq(query->terms[0].access, ECS_DSL_ACCESS_IN);
    cr_assert_eq(query->terms[1].access, ECS_DSL_ACCESS_OUT);

    ecs_dsl_query_free(query);
    ecs_dsl_parser_free(&parser);
}

Test(dsl_parser, complex_query) {
    ecs_dsl_parser_t parser;
    ecs_dsl_parser_init(&parser, "Position, !Dead, (Likes, *), ?Health || Armor");

    ecs_dsl_query_t *query = ecs_dsl_parser_parse(&parser);
    cr_assert_not_null(query);
    cr_assert_eq(query->count, 5);

    cr_assert_str_eq(query->terms[0].id.first, "Position");
    cr_assert_eq(query->terms[0].op, ECS_DSL_OP_AND);

    cr_assert_str_eq(query->terms[1].id.first, "Dead");
    cr_assert_eq(query->terms[1].modifier, ECS_DSL_MOD_NOT);

    cr_assert_eq(query->terms[2].id.is_pair, true);
    cr_assert_str_eq(query->terms[2].id.first, "Likes");
    cr_assert_eq(query->terms[2].id.second_wildcard, true);

    cr_assert_str_eq(query->terms[3].id.first, "Health");
    cr_assert_eq(query->terms[3].modifier, ECS_DSL_MOD_OPTIONAL);
    cr_assert_eq(query->terms[3].op, ECS_DSL_OP_OR);

    cr_assert_str_eq(query->terms[4].id.first, "Armor");
    cr_assert_eq(query->terms[4].modifier, ECS_DSL_MOD_NONE);
    cr_assert_eq(query->terms[4].op, ECS_DSL_OP_AND);

    ecs_dsl_query_free(query);
    ecs_dsl_parser_free(&parser);
}

Test(dsl_parser, wildcard_identifier) {
    ecs_dsl_parser_t parser;
    ecs_dsl_parser_init(&parser, "*, _");

    ecs_dsl_query_t *query = ecs_dsl_parser_parse(&parser);
    cr_assert_not_null(query);
    cr_assert_eq(query->count, 2);

    cr_assert_str_eq(query->terms[0].id.first, "*");
    cr_assert_eq(query->terms[0].id.first_wildcard, true);

    cr_assert_str_eq(query->terms[1].id.first, "_");
    cr_assert_eq(query->terms[1].id.first_wildcard, true);

    ecs_dsl_query_free(query);
    ecs_dsl_parser_free(&parser);
}

Test(dsl_parser, access_and_operators) {
    ecs_dsl_parser_t parser;
    ecs_dsl_parser_init(&parser, "[in] !Position, [out] ?Velocity");

    ecs_dsl_query_t *query = ecs_dsl_parser_parse(&parser);
    cr_assert_not_null(query);
    cr_assert_eq(query->count, 2);

    cr_assert_eq(query->terms[0].access, ECS_DSL_ACCESS_IN);
    cr_assert_eq(query->terms[0].modifier, ECS_DSL_MOD_NOT);

    cr_assert_eq(query->terms[1].access, ECS_DSL_ACCESS_OUT);
    cr_assert_eq(query->terms[1].modifier, ECS_DSL_MOD_OPTIONAL);

    ecs_dsl_query_free(query);
    ecs_dsl_parser_free(&parser);
}

Test(dsl_parser, empty_query) {
    ecs_dsl_parser_t parser;
    ecs_dsl_parser_init(&parser, "");

    ecs_dsl_query_t *query = ecs_dsl_parser_parse(&parser);
    cr_assert_not_null(query);
    cr_assert_eq(query->count, 0);

    ecs_dsl_query_free(query);
    ecs_dsl_parser_free(&parser);
}
