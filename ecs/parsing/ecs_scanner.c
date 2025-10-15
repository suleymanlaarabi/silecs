#include "parsing/ecs_scanner.h"
#include <ctype.h>
#include <string.h>

void ecs_scanner_init(ecs_scanner_t *scanner, const char *str) {
    scanner->str = str;
    scanner->pos = 0;
    scanner->len = strlen(str);
}

bool ecs_scanner_is_done(const ecs_scanner_t *scanner) {
    return scanner->pos >= scanner->len;
}

void ecs_scanner_reset(ecs_scanner_t *scanner) {
    scanner->pos = 0;
}

uint64_t ecs_scanner_position(const ecs_scanner_t *scanner) {
    return scanner->pos;
}

uint64_t ecs_scanner_remaining(const ecs_scanner_t *scanner) {
    if (scanner->pos >= scanner->len) {
        return 0;
    }
    return scanner->len - scanner->pos;
}

void ecs_scanner_advance(ecs_scanner_t *scanner) {
    if (!ecs_scanner_is_done(scanner)) {
        scanner->pos += 1;
    }
}

void ecs_scanner_advance_by(ecs_scanner_t *scanner, uint64_t n) {
    scanner->pos += n;
    if (scanner->pos > scanner->len) {
        scanner->pos = scanner->len;
    }
}

char ecs_scanner_peek(const ecs_scanner_t *scanner) {
    if (ecs_scanner_is_done(scanner)) {
        return '\0';
    }
    return scanner->str[scanner->pos];
}

char ecs_scanner_peek_at(const ecs_scanner_t *scanner, uint64_t offset) {
    uint64_t pos = scanner->pos + offset;
    if (pos >= scanner->len) {
        return '\0';
    }
    return scanner->str[pos];
}

char ecs_scanner_pop(ecs_scanner_t *scanner) {
    char c = ecs_scanner_peek(scanner);
    ecs_scanner_advance(scanner);
    return c;
}

bool ecs_scanner_expect(ecs_scanner_t *scanner, ecs_predicate_t predicate) {
    if (ecs_scanner_is_done(scanner)) {
        return false;
    }
    char c = ecs_scanner_peek(scanner);
    if (predicate(c)) {
        ecs_scanner_advance(scanner);
        return true;
    }
    return false;
}

bool ecs_scanner_expect_char(ecs_scanner_t *scanner, char expected) {
    if (ecs_scanner_is_done(scanner)) {
        return false;
    }
    char c = ecs_scanner_peek(scanner);
    if (c == expected) {
        ecs_scanner_advance(scanner);
        return true;
    }
    return false;
}

bool ecs_scanner_expect_string(ecs_scanner_t *scanner, const char *str) {
    if (!str) {
        return false;
    }

    size_t len = strlen(str);
    if (ecs_scanner_remaining(scanner) < len) {
        return false;
    }

    for (size_t i = 0; i < len; i++) {
        if (scanner->str[scanner->pos + i] != str[i]) {
            return false;
        }
    }

    ecs_scanner_advance_by(scanner, len);
    return true;
}

ecs_string_t ecs_scanner_take_while(ecs_scanner_t *scanner, ecs_predicate_t predicate) {
    ecs_string_t string = ecs_string_new();
    while (!ecs_scanner_is_done(scanner) && predicate(ecs_scanner_peek(scanner))) {
        ecs_string_push(&string, ecs_scanner_pop(scanner));
    }
    return string;
}

ecs_string_t ecs_scanner_take_until(ecs_scanner_t *scanner, ecs_predicate_t predicate) {
    ecs_string_t string = ecs_string_new();
    while (!ecs_scanner_is_done(scanner) && !predicate(ecs_scanner_peek(scanner))) {
        ecs_string_push(&string, ecs_scanner_pop(scanner));
    }
    return string;
}

ecs_string_t ecs_scanner_take_n(ecs_scanner_t *scanner, uint64_t n) {
    ecs_string_t string = ecs_string_new();
    for (uint64_t i = 0; i < n && !ecs_scanner_is_done(scanner); i++) {
        ecs_string_push(&string, ecs_scanner_pop(scanner));
    }
    return string;
}

ecs_string_t ecs_scanner_take_identifier(ecs_scanner_t *scanner) {
    ecs_string_t string = ecs_string_new();

    if (!ecs_scanner_is_done(scanner) && ecs_is_identifier_start(ecs_scanner_peek(scanner))) {
        ecs_string_push(&string, ecs_scanner_pop(scanner));

        while (!ecs_scanner_is_done(scanner) && ecs_is_identifier_char(ecs_scanner_peek(scanner))) {
            ecs_string_push(&string, ecs_scanner_pop(scanner));
        }
    }

    return string;
}

ecs_string_t ecs_scanner_take_number(ecs_scanner_t *scanner) {
    ecs_string_t string = ecs_string_new();

    while (!ecs_scanner_is_done(scanner) && ecs_isdigit(ecs_scanner_peek(scanner))) {
        ecs_string_push(&string, ecs_scanner_pop(scanner));
    }

    if (!ecs_scanner_is_done(scanner) && ecs_scanner_peek(scanner) == '.') {
        if (!ecs_scanner_is_done(scanner) && ecs_isdigit(ecs_scanner_peek_at(scanner, 1))) {
            ecs_string_push(&string, ecs_scanner_pop(scanner));
            while (!ecs_scanner_is_done(scanner) && ecs_isdigit(ecs_scanner_peek(scanner))) {
                ecs_string_push(&string, ecs_scanner_pop(scanner));
            }
        }
    }

    return string;
}

ecs_string_t ecs_scanner_take_string(ecs_scanner_t *scanner, char quote) {
    ecs_string_t string = ecs_string_new();

    if (ecs_scanner_expect_char(scanner, quote)) {
        while (!ecs_scanner_is_done(scanner)) {
            char c = ecs_scanner_peek(scanner);

            if (c == quote) {
                ecs_scanner_advance(scanner);
                break;
            }

            if (c == '\\' && !ecs_scanner_is_done(scanner)) {
                ecs_scanner_advance(scanner);
                if (!ecs_scanner_is_done(scanner)) {
                    char escaped = ecs_scanner_pop(scanner);
                    switch (escaped) {
                        case 'n': ecs_string_push(&string, '\n'); break;
                        case 't': ecs_string_push(&string, '\t'); break;
                        case 'r': ecs_string_push(&string, '\r'); break;
                        case '\\': ecs_string_push(&string, '\\'); break;
                        case '"': ecs_string_push(&string, '"'); break;
                        case '\'': ecs_string_push(&string, '\''); break;
                        case '0': ecs_string_push(&string, '\0'); break;
                        default: ecs_string_push(&string, escaped); break;
                    }
                }
            } else {
                ecs_string_push(&string, ecs_scanner_pop(scanner));
            }
        }
    }

    return string;
}

void ecs_scanner_skip_whitespace(ecs_scanner_t *scanner) {
    while (!ecs_scanner_is_done(scanner) && ecs_isspace(ecs_scanner_peek(scanner))) {
        ecs_scanner_advance(scanner);
    }
}

void ecs_scanner_skip_line(ecs_scanner_t *scanner) {
    while (!ecs_scanner_is_done(scanner)) {
        char c = ecs_scanner_peek(scanner);
        ecs_scanner_advance(scanner);
        if (c == '\n') {
            break;
        }
    }
}

bool ecs_isalpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool ecs_isdigit(char c) {
    return c >= '0' && c <= '9';
}

bool ecs_isalnum(char c) {
    return ecs_isalpha(c) || ecs_isdigit(c);
}

bool ecs_isspace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool ecs_is_identifier_start(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool ecs_is_identifier_char(char c) {
    return ecs_is_identifier_start(c) || ecs_isdigit(c);
}
