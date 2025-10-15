#include <criterion/criterion.h>
#include "../ecs/parsing/ecs_scanner.h"
#include "../ecs/parsing/ecs_tokenizer.h"
#include <string.h>
#include <stdlib.h>

Test(ecs_scanner, init_and_basics) {
    ecs_scanner_t scanner;
    ecs_scanner_init(&scanner, "hello world");

    cr_assert_eq(scanner.pos, 0);
    cr_assert_eq(scanner.len, 11);
    cr_assert_eq(ecs_scanner_is_done(&scanner), false);
    cr_assert_eq(ecs_scanner_remaining(&scanner), 11);
}

Test(ecs_scanner, peek_and_pop) {
    ecs_scanner_t scanner;
    ecs_scanner_init(&scanner, "abc");

    cr_assert_eq(ecs_scanner_peek(&scanner), 'a');
    cr_assert_eq(scanner.pos, 0);

    cr_assert_eq(ecs_scanner_pop(&scanner), 'a');
    cr_assert_eq(scanner.pos, 1);

    cr_assert_eq(ecs_scanner_peek(&scanner), 'b');
}

Test(ecs_scanner, advance_and_position) {
    ecs_scanner_t scanner;
    ecs_scanner_init(&scanner, "12345");

    ecs_scanner_advance(&scanner);
    cr_assert_eq(ecs_scanner_position(&scanner), 1);

    ecs_scanner_advance_by(&scanner, 2);
    cr_assert_eq(ecs_scanner_position(&scanner), 3);

    cr_assert_eq(ecs_scanner_remaining(&scanner), 2);
}

Test(ecs_scanner, expect_char) {
    ecs_scanner_t scanner;
    ecs_scanner_init(&scanner, "hello");

    cr_assert_eq(ecs_scanner_expect_char(&scanner, 'h'), true);
    cr_assert_eq(scanner.pos, 1);

    cr_assert_eq(ecs_scanner_expect_char(&scanner, 'x'), false);
    cr_assert_eq(scanner.pos, 1);

    cr_assert_eq(ecs_scanner_expect_char(&scanner, 'e'), true);
    cr_assert_eq(scanner.pos, 2);
}

Test(ecs_scanner, expect_string) {
    ecs_scanner_t scanner;
    ecs_scanner_init(&scanner, "function main()");

    cr_assert_eq(ecs_scanner_expect_string(&scanner, "function"), true);
    cr_assert_eq(scanner.pos, 8);

    cr_assert_eq(ecs_scanner_expect_char(&scanner, ' '), true);

    cr_assert_eq(ecs_scanner_expect_string(&scanner, "main"), true);
    cr_assert_eq(scanner.pos, 13);
}

Test(ecs_scanner, take_identifier) {
    ecs_scanner_t scanner;
    ecs_scanner_init(&scanner, "my_var123 ");

    ecs_string_t id = ecs_scanner_take_identifier(&scanner);
    ecs_string_push(&id, '\0');

    cr_assert_str_eq((const char *)id.data, "my_var123");
    cr_assert_eq(scanner.pos, 9);

    ecs_vec_free(&id);
}

Test(ecs_scanner, take_number) {
    ecs_scanner_t scanner;
    ecs_scanner_init(&scanner, "123.456 ");

    ecs_string_t num = ecs_scanner_take_number(&scanner);
    ecs_string_push(&num, '\0');

    cr_assert_str_eq((const char *)num.data, "123.456");
    cr_assert_eq(scanner.pos, 7);

    ecs_vec_free(&num);
}

Test(ecs_scanner, take_while) {
    ecs_scanner_t scanner;
    ecs_scanner_init(&scanner, "aaaabbbccc");

    ecs_string_t result = ecs_scanner_take_while(&scanner, ecs_isalpha);
    ecs_string_push(&result, '\0');

    cr_assert_str_eq((const char *)result.data, "aaaabbbccc");
    cr_assert_eq(ecs_scanner_is_done(&scanner), true);

    ecs_vec_free(&result);
}

Test(ecs_scanner, take_until) {
    ecs_scanner_t scanner;
    ecs_scanner_init(&scanner, "hello world");

    ecs_string_t result = ecs_scanner_take_until(&scanner, ecs_isspace);
    ecs_string_push(&result, '\0');

    cr_assert_str_eq((const char *)result.data, "hello");
    cr_assert_eq(ecs_scanner_peek(&scanner), ' ');

    ecs_vec_free(&result);
}

Test(ecs_scanner, skip_whitespace) {
    ecs_scanner_t scanner;
    ecs_scanner_init(&scanner, "   \t\n  hello");

    ecs_scanner_skip_whitespace(&scanner);
    cr_assert_eq(ecs_scanner_peek(&scanner), 'h');
}

Test(ecs_scanner, predicates) {
    cr_assert_eq(ecs_isalpha('a'), true);
    cr_assert_eq(ecs_isalpha('Z'), true);
    cr_assert_eq(ecs_isalpha('_'), true);
    cr_assert_eq(ecs_isalpha('0'), false);

    cr_assert_eq(ecs_isdigit('5'), true);
    cr_assert_eq(ecs_isdigit('a'), false);

    cr_assert_eq(ecs_isalnum('a'), true);
    cr_assert_eq(ecs_isalnum('5'), true);
    cr_assert_eq(ecs_isalnum('_'), true);
    cr_assert_eq(ecs_isalnum(' '), false);

    cr_assert_eq(ecs_isspace(' '), true);
    cr_assert_eq(ecs_isspace('\t'), true);
    cr_assert_eq(ecs_isspace('\n'), true);
    cr_assert_eq(ecs_isspace('a'), false);
}

Test(ecs_tokenizer, init_and_free) {
    ecs_tokenizer_t tokenizer;
    ecs_tokenizer_init(&tokenizer, "hello world");

    cr_assert_eq(tokenizer.current, 0);
    cr_assert_eq(tokenizer.line, 1);
    cr_assert_eq(tokenizer.column, 1);

    ecs_tokenizer_free(&tokenizer);
}

Test(ecs_tokenizer, tokenize_identifier) {
    ecs_tokenizer_t tokenizer;
    ecs_tokenizer_init(&tokenizer, "hello_world");

    ecs_token_t *token = ecs_tokenizer_next_token(&tokenizer);
    cr_assert_not_null(token);
    cr_assert_eq(token->type, ECS_TOKEN_IDENTIFIER);

    ecs_string_push(&token->value.string, '\0');
    cr_assert_str_eq((const char *)token->value.string.data, "hello_world");

    ecs_tokenizer_free(&tokenizer);
}

Test(ecs_tokenizer, tokenize_number) {
    ecs_tokenizer_t tokenizer;
    ecs_tokenizer_init(&tokenizer, "123.456");

    ecs_token_t *token = ecs_tokenizer_next_token(&tokenizer);
    cr_assert_not_null(token);
    cr_assert_eq(token->type, ECS_TOKEN_NUMBER);
    cr_assert_float_eq(token->value.number, 123.456, 0.001);

    ecs_tokenizer_free(&tokenizer);
}

Test(ecs_tokenizer, tokenize_string) {
    ecs_tokenizer_t tokenizer;
    ecs_tokenizer_init(&tokenizer, "\"hello world\"");

    ecs_token_t *token = ecs_tokenizer_next_token(&tokenizer);
    cr_assert_not_null(token);
    cr_assert_eq(token->type, ECS_TOKEN_STRING);

    ecs_string_push(&token->value.string, '\0');
    cr_assert_str_eq((const char *)token->value.string.data, "hello world");

    ecs_tokenizer_free(&tokenizer);
}

Test(ecs_tokenizer, tokenize_operators) {
    ecs_tokenizer_t tokenizer;
    ecs_tokenizer_init(&tokenizer, "&& || ! == != < > <= >=");

    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_AND);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_OR);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_NOT);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_EQ);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_NEQ);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_LT);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_GT);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_LTE);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_GTE);

    ecs_tokenizer_free(&tokenizer);
}

Test(ecs_tokenizer, tokenize_delimiters) {
    ecs_tokenizer_t tokenizer;
    ecs_tokenizer_init(&tokenizer, "( ) [ ] { } , . : ;");

    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_LPAREN);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_RPAREN);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_LBRACKET);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_RBRACKET);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_LBRACE);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_RBRACE);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_COMMA);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_DOT);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_COLON);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_SEMICOLON);

    ecs_tokenizer_free(&tokenizer);
}

Test(ecs_tokenizer, tokenize_keywords) {
    ecs_tokenizer_t tokenizer;
    ecs_tokenizer_init(&tokenizer, "in out inout with without AND OR NOT");

    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_IN);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_OUT);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_INOUT);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_WITH);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_WITHOUT);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_AND);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_OR);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_NOT);

    ecs_tokenizer_free(&tokenizer);
}

Test(ecs_tokenizer, tokenize_wildcards) {
    ecs_tokenizer_t tokenizer;
    ecs_tokenizer_init(&tokenizer, "* _ ?");

    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_WILDCARD);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_WILDCARD_ONE);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_OPTIONAL);

    ecs_tokenizer_free(&tokenizer);
}

Test(ecs_tokenizer, tokenize_comment) {
    ecs_tokenizer_t tokenizer;
    ecs_tokenizer_init(&tokenizer, "hello // this is a comment\nworld");

    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_IDENTIFIER);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_IDENTIFIER);

    ecs_tokenizer_free(&tokenizer);
}

Test(ecs_tokenizer, tokenize_complex_query) {
    ecs_tokenizer_t tokenizer;
    ecs_tokenizer_init(&tokenizer, "Position, Velocity, !Static, ?Rotation");

    ecs_token_t *tok1 = ecs_tokenizer_next_token(&tokenizer);
    cr_assert_eq(tok1->type, ECS_TOKEN_IDENTIFIER);
    ecs_string_push(&tok1->value.string, '\0');
    cr_assert_str_eq((const char *)tok1->value.string.data, "Position");

    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_COMMA);

    ecs_token_t *tok2 = ecs_tokenizer_next_token(&tokenizer);
    cr_assert_eq(tok2->type, ECS_TOKEN_IDENTIFIER);
    ecs_string_push(&tok2->value.string, '\0');
    cr_assert_str_eq((const char *)tok2->value.string.data, "Velocity");

    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_COMMA);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_NOT);

    ecs_token_t *tok3 = ecs_tokenizer_next_token(&tokenizer);
    cr_assert_eq(tok3->type, ECS_TOKEN_IDENTIFIER);
    ecs_string_push(&tok3->value.string, '\0');
    cr_assert_str_eq((const char *)tok3->value.string.data, "Static");

    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_COMMA);
    cr_assert_eq(ecs_tokenizer_next_token(&tokenizer)->type, ECS_TOKEN_OPTIONAL);

    ecs_token_t *tok4 = ecs_tokenizer_next_token(&tokenizer);
    cr_assert_eq(tok4->type, ECS_TOKEN_IDENTIFIER);
    ecs_string_push(&tok4->value.string, '\0');
    cr_assert_str_eq((const char *)tok4->value.string.data, "Rotation");

    ecs_tokenizer_free(&tokenizer);
}

Test(ecs_tokenizer, navigation) {
    ecs_tokenizer_t tokenizer;
    ecs_tokenizer_init(&tokenizer, "a b c");

    ecs_tokenizer_tokenize(&tokenizer);

    ecs_token_t *tok = ecs_tokenizer_peek(&tokenizer);
    cr_assert_not_null(tok);
    cr_assert_eq(tok->type, ECS_TOKEN_IDENTIFIER);

    cr_assert_eq(tokenizer.current, 0);

    tok = ecs_tokenizer_advance(&tokenizer);
    cr_assert_eq(tokenizer.current, 1);

    tok = ecs_tokenizer_peek_ahead(&tokenizer, 1);
    cr_assert_not_null(tok);
    cr_assert_eq(tok->type, ECS_TOKEN_IDENTIFIER);

    ecs_tokenizer_free(&tokenizer);
}

Test(ecs_tokenizer, match_and_expect) {
    ecs_tokenizer_t tokenizer;
    ecs_tokenizer_init(&tokenizer, "( hello )");

    ecs_tokenizer_tokenize(&tokenizer);

    cr_assert_eq(ecs_tokenizer_match(&tokenizer, ECS_TOKEN_LPAREN), true);
    cr_assert_eq(tokenizer.current, 1);

    cr_assert_eq(ecs_tokenizer_expect(&tokenizer, ECS_TOKEN_IDENTIFIER), true);
    cr_assert_eq(tokenizer.current, 2);

    cr_assert_eq(ecs_tokenizer_expect(&tokenizer, ECS_TOKEN_RPAREN), true);

    ecs_tokenizer_free(&tokenizer);
}

Test(ecs_tokenizer, token_position_tracking) {
    ecs_tokenizer_t tokenizer;
    ecs_tokenizer_init(&tokenizer, "hello\nworld");

    ecs_token_t *tok1 = ecs_tokenizer_next_token(&tokenizer);
    cr_assert_eq(tok1->line, 1);
    cr_assert_eq(tok1->column, 1);

    ecs_token_t *tok2 = ecs_tokenizer_next_token(&tokenizer);
    cr_assert_eq(tok2->line, 2);
    cr_assert_eq(tok2->column, 1);

    ecs_tokenizer_free(&tokenizer);
}

Test(ecs_tokenizer, escape_sequences) {
    ecs_tokenizer_t tokenizer;
    ecs_tokenizer_init(&tokenizer, "\"hello\\nworld\\t!\"");

    ecs_token_t *token = ecs_tokenizer_next_token(&tokenizer);
    cr_assert_eq(token->type, ECS_TOKEN_STRING);

    const char *str = (const char *)token->value.string.data;
    cr_assert_eq(str[5], '\n');
    cr_assert_eq(str[11], '\t');

    ecs_tokenizer_free(&tokenizer);
}

Test(ecs_tokenizer, utilities) {
    cr_assert_str_eq(ecs_token_type_to_string(ECS_TOKEN_IDENTIFIER), "IDENTIFIER");
    cr_assert_str_eq(ecs_token_type_to_string(ECS_TOKEN_NUMBER), "NUMBER");
    cr_assert_str_eq(ecs_token_type_to_string(ECS_TOKEN_AND), "AND");
    cr_assert_str_eq(ecs_token_type_to_string(ECS_TOKEN_LPAREN), "LPAREN");
}
