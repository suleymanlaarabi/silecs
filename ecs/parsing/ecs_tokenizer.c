#include "ecs_tokenizer.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool is_alnum(char c) {
    return is_alpha(c) || is_digit(c) || c == '_';
}

static bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

ecs_token_t ecs_token_create(ecs_token_type_t type, uint64_t line, uint64_t column, uint64_t position) {
    ecs_token_t token = {0};
    token.type = type;
    token.line = line;
    token.column = column;
    token.position = position;
    token.length = 1;
    return token;
}

ecs_token_t ecs_token_create_identifier(ecs_string_t str, uint64_t line, uint64_t column, uint64_t position) {
    ecs_token_t token = ecs_token_create(ECS_TOKEN_IDENTIFIER, line, column, position);
    token.value.string = str;
    token.length = str.count;
    return token;
}

ecs_token_t ecs_token_create_number(double number, uint64_t line, uint64_t column, uint64_t position) {
    ecs_token_t token = ecs_token_create(ECS_TOKEN_NUMBER, line, column, position);
    token.value.number = number;
    return token;
}

ecs_token_t ecs_token_create_string(ecs_string_t str, uint64_t line, uint64_t column, uint64_t position) {
    ecs_token_t token = ecs_token_create(ECS_TOKEN_STRING, line, column, position);
    token.value.string = str;
    token.length = str.count + 2;
    return token;
}

ecs_token_t ecs_token_create_error(ecs_string_t msg, uint64_t line, uint64_t column, uint64_t position) {
    ecs_token_t token = ecs_token_create(ECS_TOKEN_ERROR, line, column, position);
    token.value.string = msg;
    return token;
}

void ecs_token_free(ecs_token_t *token) {
    if (token->type == ECS_TOKEN_IDENTIFIER ||
        token->type == ECS_TOKEN_STRING ||
        token->type == ECS_TOKEN_ERROR) {
        if (token->value.string.data) {
            ecs_vec_free(&token->value.string);
        }
    }
}

void ecs_tokenizer_init(ecs_tokenizer_t *tokenizer, const char *source) {
    ecs_scanner_init(&tokenizer->scanner, source);
    ecs_vec_init(&tokenizer->tokens, sizeof(ecs_token_t));
    tokenizer->current = 0;
    tokenizer->line = 1;
    tokenizer->column = 1;
    tokenizer->skip_whitespace = true;
    tokenizer->skip_comments = true;
}

void ecs_tokenizer_free(ecs_tokenizer_t *tokenizer) {
    for (size_t i = 0; i < tokenizer->tokens.count; i++) {
        ecs_token_t *token = ECS_VEC_GET(ecs_token_t, &tokenizer->tokens, i);
        ecs_token_free(token);
    }
    ecs_vec_free(&tokenizer->tokens);
}

void ecs_tokenizer_set_skip_whitespace(ecs_tokenizer_t *tokenizer, bool skip) {
    tokenizer->skip_whitespace = skip;
}

void ecs_tokenizer_set_skip_comments(ecs_tokenizer_t *tokenizer, bool skip) {
    tokenizer->skip_comments = skip;
}

static void skip_whitespace_internal(ecs_tokenizer_t *tokenizer) {
    while (!ecs_scanner_is_done(&tokenizer->scanner)) {
        char c = ecs_scanner_peek(&tokenizer->scanner);
        if (is_whitespace(c)) {
            if (c == '\n') {
                tokenizer->line++;
                tokenizer->column = 1;
            } else {
                tokenizer->column++;
            }
            ecs_scanner_advance(&tokenizer->scanner);
        } else {
            break;
        }
    }
}

static ecs_token_t tokenize_number(ecs_tokenizer_t *tokenizer) {
    uint64_t start_pos = tokenizer->scanner.pos;
    uint64_t start_col = tokenizer->column;
    ecs_string_t num_str = ecs_string_new();

    while (!ecs_scanner_is_done(&tokenizer->scanner) &&
           is_digit(ecs_scanner_peek(&tokenizer->scanner))) {
        ecs_string_push(&num_str, ecs_scanner_pop(&tokenizer->scanner));
        tokenizer->column++;
    }

    if (!ecs_scanner_is_done(&tokenizer->scanner) &&
        ecs_scanner_peek(&tokenizer->scanner) == '.') {
        ecs_string_push(&num_str, ecs_scanner_pop(&tokenizer->scanner));
        tokenizer->column++;

        while (!ecs_scanner_is_done(&tokenizer->scanner) &&
               is_digit(ecs_scanner_peek(&tokenizer->scanner))) {
            ecs_string_push(&num_str, ecs_scanner_pop(&tokenizer->scanner));
            tokenizer->column++;
        }
    }

    ecs_string_push(&num_str, '\0');
    double value = atof((const char *)num_str.data);

    ecs_token_t token = ecs_token_create_number(value, tokenizer->line, start_col, start_pos);
    token.length = tokenizer->scanner.pos - start_pos;

    ecs_vec_free(&num_str);
    return token;
}

static ecs_token_t tokenize_identifier(ecs_tokenizer_t *tokenizer) {
    uint64_t start_pos = tokenizer->scanner.pos;
    uint64_t start_col = tokenizer->column;
    ecs_string_t id_str = ecs_string_new();

    while (!ecs_scanner_is_done(&tokenizer->scanner) &&
           is_alnum(ecs_scanner_peek(&tokenizer->scanner))) {
        ecs_string_push(&id_str, ecs_scanner_pop(&tokenizer->scanner));
        tokenizer->column++;
    }

    ecs_string_push(&id_str, '\0');

    const char *str = (const char *)id_str.data;
    ecs_token_type_t type = ECS_TOKEN_IDENTIFIER;

    if (strcmp(str, "in") == 0) type = ECS_TOKEN_IN;
    else if (strcmp(str, "out") == 0) type = ECS_TOKEN_OUT;
    else if (strcmp(str, "inout") == 0) type = ECS_TOKEN_INOUT;
    else if (strcmp(str, "with") == 0) type = ECS_TOKEN_WITH;
    else if (strcmp(str, "without") == 0) type = ECS_TOKEN_WITHOUT;
    else if (strcmp(str, "AND") == 0 || strcmp(str, "and") == 0) type = ECS_TOKEN_AND;
    else if (strcmp(str, "OR") == 0 || strcmp(str, "or") == 0) type = ECS_TOKEN_OR;
    else if (strcmp(str, "NOT") == 0 || strcmp(str, "not") == 0) type = ECS_TOKEN_NOT;

    ecs_token_t token;
    if (type == ECS_TOKEN_IDENTIFIER) {
        token = ecs_token_create_identifier(id_str, tokenizer->line, start_col, start_pos);
    } else {
        token = ecs_token_create(type, tokenizer->line, start_col, start_pos);
        token.length = tokenizer->scanner.pos - start_pos;
        ecs_vec_free(&id_str);
    }

    return token;
}

static ecs_token_t tokenize_string(ecs_tokenizer_t *tokenizer, char quote) {
    uint64_t start_pos = tokenizer->scanner.pos;
    uint64_t start_col = tokenizer->column;
    ecs_string_t str = ecs_string_new();

    ecs_scanner_advance(&tokenizer->scanner);
    tokenizer->column++;

    while (!ecs_scanner_is_done(&tokenizer->scanner)) {
        char c = ecs_scanner_peek(&tokenizer->scanner);

        if (c == quote) {
            ecs_scanner_advance(&tokenizer->scanner);
            tokenizer->column++;
            break;
        }

        if (c == '\\' && !ecs_scanner_is_done(&tokenizer->scanner)) {
            ecs_scanner_advance(&tokenizer->scanner);
            tokenizer->column++;
            if (!ecs_scanner_is_done(&tokenizer->scanner)) {
                char escaped = ecs_scanner_pop(&tokenizer->scanner);
                tokenizer->column++;

                switch (escaped) {
                    case 'n': ecs_string_push(&str, '\n'); break;
                    case 't': ecs_string_push(&str, '\t'); break;
                    case 'r': ecs_string_push(&str, '\r'); break;
                    case '\\': ecs_string_push(&str, '\\'); break;
                    case '"': ecs_string_push(&str, '"'); break;
                    case '\'': ecs_string_push(&str, '\''); break;
                    default: ecs_string_push(&str, escaped); break;
                }
            }
        } else {
            ecs_string_push(&str, ecs_scanner_pop(&tokenizer->scanner));
            tokenizer->column++;
        }
    }

    ecs_string_push(&str, '\0');
    return ecs_token_create_string(str, tokenizer->line, start_col, start_pos);
}

static ecs_token_t tokenize_comment(ecs_tokenizer_t *tokenizer) {
    uint64_t start_pos = tokenizer->scanner.pos;
    uint64_t start_col = tokenizer->column;

    ecs_scanner_advance(&tokenizer->scanner);
    ecs_scanner_advance(&tokenizer->scanner);
    tokenizer->column += 2;

    while (!ecs_scanner_is_done(&tokenizer->scanner)) {
        char c = ecs_scanner_peek(&tokenizer->scanner);
        if (c == '\n') {
            break;
        }
        ecs_scanner_advance(&tokenizer->scanner);
        tokenizer->column++;
    }

    ecs_token_t token = ecs_token_create(ECS_TOKEN_COMMENT, tokenizer->line, start_col, start_pos);
    token.length = tokenizer->scanner.pos - start_pos;
    return token;
}

ecs_token_t *ecs_tokenizer_next_token(ecs_tokenizer_t *tokenizer) {
    if (tokenizer->skip_whitespace) {
        skip_whitespace_internal(tokenizer);
    }

    if (ecs_scanner_is_done(&tokenizer->scanner)) {
        ecs_token_t token = ecs_token_create(ECS_TOKEN_EOF, tokenizer->line, tokenizer->column, tokenizer->scanner.pos);
        ecs_vec_push(&tokenizer->tokens, &token);
        return ECS_VEC_GET(ecs_token_t, &tokenizer->tokens, tokenizer->tokens.count - 1);
    }

    char c = ecs_scanner_peek(&tokenizer->scanner);
    uint64_t start_col = tokenizer->column;
    uint64_t start_pos = tokenizer->scanner.pos;
    ecs_token_t token;

    if (is_digit(c)) {
        token = tokenize_number(tokenizer);
    }

    else if (c == '_') {
        if (!ecs_scanner_is_done(&tokenizer->scanner)) {
            char next = ecs_scanner_peek_at(&tokenizer->scanner, 1);
            if (is_alnum(next)) {
                token = tokenize_identifier(tokenizer);
            } else {
                ecs_scanner_advance(&tokenizer->scanner);
                tokenizer->column++;
                token = ecs_token_create(ECS_TOKEN_WILDCARD_ONE, tokenizer->line, start_col, start_pos);
            }
        } else {
            ecs_scanner_advance(&tokenizer->scanner);
            tokenizer->column++;
            token = ecs_token_create(ECS_TOKEN_WILDCARD_ONE, tokenizer->line, start_col, start_pos);
        }
    }

    else if (is_alpha(c)) {
        token = tokenize_identifier(tokenizer);
    }

    else if (c == '"' || c == '\'') {
        token = tokenize_string(tokenizer, c);
    }

    else if (c == '/' && !ecs_scanner_is_done(&tokenizer->scanner)) {
        ecs_scanner_advance(&tokenizer->scanner);
        if (!ecs_scanner_is_done(&tokenizer->scanner) && ecs_scanner_peek(&tokenizer->scanner) == '/') {
            ecs_scanner_t saved = tokenizer->scanner;
            saved.pos--;
            tokenizer->scanner = saved;
            token = tokenize_comment(tokenizer);
            if (tokenizer->skip_comments) {
                return ecs_tokenizer_next_token(tokenizer);
            }
        } else {
            ecs_scanner_t saved = tokenizer->scanner;
            saved.pos--;
            tokenizer->scanner = saved;
            token = ecs_token_create(ECS_TOKEN_ERROR, tokenizer->line, start_col, start_pos);
            ecs_string_t err = ecs_string_from_cstring("Unexpected character '/'");
            token.value.string = err;
            ecs_scanner_advance(&tokenizer->scanner);
            tokenizer->column++;
        }
    }

    else {
        ecs_scanner_advance(&tokenizer->scanner);
        tokenizer->column++;

        switch (c) {
            case '(': token = ecs_token_create(ECS_TOKEN_LPAREN, tokenizer->line, start_col, start_pos); break;
            case ')': token = ecs_token_create(ECS_TOKEN_RPAREN, tokenizer->line, start_col, start_pos); break;
            case '[': token = ecs_token_create(ECS_TOKEN_LBRACKET, tokenizer->line, start_col, start_pos); break;
            case ']': token = ecs_token_create(ECS_TOKEN_RBRACKET, tokenizer->line, start_col, start_pos); break;
            case '{': token = ecs_token_create(ECS_TOKEN_LBRACE, tokenizer->line, start_col, start_pos); break;
            case '}': token = ecs_token_create(ECS_TOKEN_RBRACE, tokenizer->line, start_col, start_pos); break;
            case ',': token = ecs_token_create(ECS_TOKEN_COMMA, tokenizer->line, start_col, start_pos); break;
            case '.': token = ecs_token_create(ECS_TOKEN_DOT, tokenizer->line, start_col, start_pos); break;
            case ':': token = ecs_token_create(ECS_TOKEN_COLON, tokenizer->line, start_col, start_pos); break;
            case ';': token = ecs_token_create(ECS_TOKEN_SEMICOLON, tokenizer->line, start_col, start_pos); break;
            case '?': token = ecs_token_create(ECS_TOKEN_OPTIONAL, tokenizer->line, start_col, start_pos); break;
            case '*': token = ecs_token_create(ECS_TOKEN_WILDCARD, tokenizer->line, start_col, start_pos); break;
            case '_': token = ecs_token_create(ECS_TOKEN_WILDCARD_ONE, tokenizer->line, start_col, start_pos); break;

            case '!':
                if (!ecs_scanner_is_done(&tokenizer->scanner) && ecs_scanner_peek(&tokenizer->scanner) == '=') {
                    ecs_scanner_advance(&tokenizer->scanner);
                    tokenizer->column++;
                    token = ecs_token_create(ECS_TOKEN_NEQ, tokenizer->line, start_col, start_pos);
                    token.length = 2;
                } else {
                    token = ecs_token_create(ECS_TOKEN_NOT, tokenizer->line, start_col, start_pos);
                }
                break;

            case '=':
                if (!ecs_scanner_is_done(&tokenizer->scanner) && ecs_scanner_peek(&tokenizer->scanner) == '=') {
                    ecs_scanner_advance(&tokenizer->scanner);
                    tokenizer->column++;
                    token = ecs_token_create(ECS_TOKEN_EQ, tokenizer->line, start_col, start_pos);
                    token.length = 2;
                } else {
                    token = ecs_token_create(ECS_TOKEN_EQ, tokenizer->line, start_col, start_pos);
                }
                break;

            case '<':
                if (!ecs_scanner_is_done(&tokenizer->scanner) && ecs_scanner_peek(&tokenizer->scanner) == '=') {
                    ecs_scanner_advance(&tokenizer->scanner);
                    tokenizer->column++;
                    token = ecs_token_create(ECS_TOKEN_LTE, tokenizer->line, start_col, start_pos);
                    token.length = 2;
                } else if (!ecs_scanner_is_done(&tokenizer->scanner) && ecs_scanner_peek(&tokenizer->scanner) == '>') {
                    ecs_scanner_advance(&tokenizer->scanner);
                    tokenizer->column++;
                    token = ecs_token_create(ECS_TOKEN_NEQ, tokenizer->line, start_col, start_pos);
                    token.length = 2;
                } else {
                    token = ecs_token_create(ECS_TOKEN_LT, tokenizer->line, start_col, start_pos);
                }
                break;

            case '>':
                if (!ecs_scanner_is_done(&tokenizer->scanner) && ecs_scanner_peek(&tokenizer->scanner) == '=') {
                    ecs_scanner_advance(&tokenizer->scanner);
                    tokenizer->column++;
                    token = ecs_token_create(ECS_TOKEN_GTE, tokenizer->line, start_col, start_pos);
                    token.length = 2;
                } else {
                    token = ecs_token_create(ECS_TOKEN_GT, tokenizer->line, start_col, start_pos);
                }
                break;

            case '&':
                if (!ecs_scanner_is_done(&tokenizer->scanner) && ecs_scanner_peek(&tokenizer->scanner) == '&') {
                    ecs_scanner_advance(&tokenizer->scanner);
                    tokenizer->column++;
                    token = ecs_token_create(ECS_TOKEN_AND, tokenizer->line, start_col, start_pos);
                    token.length = 2;
                } else {
                    token = ecs_token_create(ECS_TOKEN_ERROR, tokenizer->line, start_col, start_pos);
                    ecs_string_t err = ecs_string_from_cstring("Expected '&&'");
                    token.value.string = err;
                }
                break;

            case '|':
                if (!ecs_scanner_is_done(&tokenizer->scanner) && ecs_scanner_peek(&tokenizer->scanner) == '|') {
                    ecs_scanner_advance(&tokenizer->scanner);
                    tokenizer->column++;
                    token = ecs_token_create(ECS_TOKEN_OR, tokenizer->line, start_col, start_pos);
                    token.length = 2;
                } else {
                    token = ecs_token_create(ECS_TOKEN_PIPE, tokenizer->line, start_col, start_pos);
                }
                break;

            default: {
                token = ecs_token_create(ECS_TOKEN_ERROR, tokenizer->line, start_col, start_pos);
                ecs_string_t err = ecs_string_new();
                ecs_string_push_cstr(&err, "Unexpected character '");
                ecs_string_push(&err, c);
                ecs_string_push_cstr(&err, "'");
                ecs_string_push(&err, '\0');
                token.value.string = err;
                break;
            }
        }
    }

    ecs_vec_push(&tokenizer->tokens, &token);
    return ECS_VEC_GET(ecs_token_t, &tokenizer->tokens, tokenizer->tokens.count - 1);
}

bool ecs_tokenizer_tokenize(ecs_tokenizer_t *tokenizer) {
    while (!ecs_scanner_is_done(&tokenizer->scanner)) {
        ecs_token_t *token = ecs_tokenizer_next_token(tokenizer);
        if (token->type == ECS_TOKEN_ERROR) {
            return false;
        }
        if (token->type == ECS_TOKEN_EOF) {
            break;
        }
    }
    return true;
}

ecs_token_t *ecs_tokenizer_peek(const ecs_tokenizer_t *tokenizer) {
    if (tokenizer->current >= tokenizer->tokens.count) {
        return NULL;
    }
    return ECS_VEC_GET(ecs_token_t, &tokenizer->tokens, tokenizer->current);
}

ecs_token_t *ecs_tokenizer_peek_ahead(const ecs_tokenizer_t *tokenizer, uint64_t offset) {
    uint64_t index = tokenizer->current + offset;
    if (index >= tokenizer->tokens.count) {
        return NULL;
    }
    return ECS_VEC_GET(ecs_token_t, &tokenizer->tokens, index);
}

ecs_token_t *ecs_tokenizer_advance(ecs_tokenizer_t *tokenizer) {
    if (tokenizer->current >= tokenizer->tokens.count) {
        return NULL;
    }
    return ECS_VEC_GET(ecs_token_t, &tokenizer->tokens, tokenizer->current++);
}

bool ecs_tokenizer_is_done(const ecs_tokenizer_t *tokenizer) {
    if (tokenizer->current >= tokenizer->tokens.count) {
        return true;
    }
    ecs_token_t *token = ecs_tokenizer_peek(tokenizer);
    return token && token->type == ECS_TOKEN_EOF;
}

bool ecs_tokenizer_match(ecs_tokenizer_t *tokenizer, ecs_token_type_t type) {
    ecs_token_t *token = ecs_tokenizer_peek(tokenizer);
    if (token && token->type == type) {
        ecs_tokenizer_advance(tokenizer);
        return true;
    }
    return false;
}

bool ecs_tokenizer_expect(ecs_tokenizer_t *tokenizer, ecs_token_type_t type) {
    ecs_token_t *token = ecs_tokenizer_peek(tokenizer);
    if (!token || token->type != type) {
        return false;
    }
    ecs_tokenizer_advance(tokenizer);
    return true;
}

const char *ecs_token_type_to_string(ecs_token_type_t type) {
    switch (type) {
        case ECS_TOKEN_IDENTIFIER: return "IDENTIFIER";
        case ECS_TOKEN_NUMBER: return "NUMBER";
        case ECS_TOKEN_STRING: return "STRING";
        case ECS_TOKEN_AND: return "AND";
        case ECS_TOKEN_OR: return "OR";
        case ECS_TOKEN_NOT: return "NOT";
        case ECS_TOKEN_EQ: return "EQ";
        case ECS_TOKEN_NEQ: return "NEQ";
        case ECS_TOKEN_LT: return "LT";
        case ECS_TOKEN_GT: return "GT";
        case ECS_TOKEN_LTE: return "LTE";
        case ECS_TOKEN_GTE: return "GTE";
        case ECS_TOKEN_LPAREN: return "LPAREN";
        case ECS_TOKEN_RPAREN: return "RPAREN";
        case ECS_TOKEN_LBRACKET: return "LBRACKET";
        case ECS_TOKEN_RBRACKET: return "RBRACKET";
        case ECS_TOKEN_LBRACE: return "LBRACE";
        case ECS_TOKEN_RBRACE: return "RBRACE";
        case ECS_TOKEN_COMMA: return "COMMA";
        case ECS_TOKEN_DOT: return "DOT";
        case ECS_TOKEN_COLON: return "COLON";
        case ECS_TOKEN_SEMICOLON: return "SEMICOLON";
        case ECS_TOKEN_PIPE: return "PIPE";
        case ECS_TOKEN_OPTIONAL: return "OPTIONAL";
        case ECS_TOKEN_WILDCARD: return "WILDCARD";
        case ECS_TOKEN_WILDCARD_ONE: return "WILDCARD_ONE";
        case ECS_TOKEN_IN: return "IN";
        case ECS_TOKEN_OUT: return "OUT";
        case ECS_TOKEN_INOUT: return "INOUT";
        case ECS_TOKEN_WITH: return "WITH";
        case ECS_TOKEN_WITHOUT: return "WITHOUT";
        case ECS_TOKEN_EOF: return "EOF";
        case ECS_TOKEN_ERROR: return "ERROR";
        case ECS_TOKEN_WHITESPACE: return "WHITESPACE";
        case ECS_TOKEN_COMMENT: return "COMMENT";
        default: return "UNKNOWN";
    }
}

void ecs_token_print(const ecs_token_t *token) {
    printf("[%s] at line %llu, col %llu: ",
           ecs_token_type_to_string(token->type),
           (unsigned long long)token->line,
           (unsigned long long)token->column);

    switch (token->type) {
        case ECS_TOKEN_IDENTIFIER:
        case ECS_TOKEN_STRING:
        case ECS_TOKEN_ERROR:
            if (token->value.string.data) {
                printf("%s\n", (char *)token->value.string.data);
            }
            break;
        case ECS_TOKEN_NUMBER:
            printf("%f\n", token->value.number);
            break;
        default:
            printf("\n");
            break;
    }
}
