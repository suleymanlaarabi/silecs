#ifndef ECS_SCANNER_H
#define ECS_SCANNER_H

#include "ecs_string.h"
#include <stdbool.h>
#include <stdint.h>

typedef bool (*ecs_predicate_t)(char);

typedef struct {
    const char *str;
    uint64_t pos;
    uint64_t len;
} ecs_scanner_t;

void ecs_scanner_init(ecs_scanner_t *scanner, const char *str);
bool ecs_scanner_is_done(const ecs_scanner_t *scanner);
void ecs_scanner_reset(ecs_scanner_t *scanner);
uint64_t ecs_scanner_position(const ecs_scanner_t *scanner);
uint64_t ecs_scanner_remaining(const ecs_scanner_t *scanner);

void ecs_scanner_advance(ecs_scanner_t *scanner);
void ecs_scanner_advance_by(ecs_scanner_t *scanner, uint64_t n);
char ecs_scanner_peek(const ecs_scanner_t *scanner);
char ecs_scanner_peek_at(const ecs_scanner_t *scanner, uint64_t offset);
char ecs_scanner_pop(ecs_scanner_t *scanner);

bool ecs_scanner_expect(ecs_scanner_t *scanner, ecs_predicate_t predicate);
bool ecs_scanner_expect_char(ecs_scanner_t *scanner, char c);
bool ecs_scanner_expect_string(ecs_scanner_t *scanner, const char *str);

ecs_string_t ecs_scanner_take_while(ecs_scanner_t *scanner, ecs_predicate_t predicate);
ecs_string_t ecs_scanner_take_until(ecs_scanner_t *scanner, ecs_predicate_t predicate);
ecs_string_t ecs_scanner_take_n(ecs_scanner_t *scanner, uint64_t n);

ecs_string_t ecs_scanner_take_identifier(ecs_scanner_t *scanner);
ecs_string_t ecs_scanner_take_number(ecs_scanner_t *scanner);
ecs_string_t ecs_scanner_take_string(ecs_scanner_t *scanner, char quote);
void ecs_scanner_skip_whitespace(ecs_scanner_t *scanner);
void ecs_scanner_skip_line(ecs_scanner_t *scanner);

bool ecs_isalpha(char c);
bool ecs_isdigit(char c);
bool ecs_isalnum(char c);
bool ecs_isspace(char c);
bool ecs_is_identifier_char(char c);
bool ecs_is_identifier_start(char c);

#endif
