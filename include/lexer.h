#ifndef LEXER_H
#define LEXER_H

#include "vec.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

enum token_type {
  t_unknown = 0, // used to null terminate parser_rules
  t_operator,
  t_return,
  t_identifier,
  t_if,
  t_else,
  t_elif,
  t_let,
  t_const,
  t_equals,
  t_literal, // only numbers for now, TODO: split into multiple types
  t_lparen,
  t_rparen,
  t_lbrace,
  t_rbrace,
  t_comma,
  t_EOF,
};

typedef struct token_pos {
  char *filename;
  size_t row; // AKA line
  size_t col; // AKA character
  size_t line_start;
  size_t len;
} token_pos;

// TODO: add file index
typedef struct token {
  enum token_type type;
  token_pos pos;
  union {
    vec_char_t str;        // identifer, str literal
    long long int integer; // num literal
  };
} token;

typedef vec_t(token) vec_token;

char *token_location(token t);
char *token_str(token t);
char *token_type_str(enum token_type t);
vec_token lex(char *filename);

#endif
