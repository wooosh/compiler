#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
struct param_pair {
  token name;
  token type;
};

typedef struct function {
  token return_type;
  token name;
  size_t params_len;
  struct param_pair *params;
} function;

enum expression_type {
  // statements
  e_fn_call,
  e_return,
  /*
  e_declaration,
  e_assignment,
  e_if, // Includes else
  e_while,
  e_for,
  e_match
  */

  // non-statements
  e_integer_literal,
  e_variable // @Consider: change to ref?
};

struct fn_call;
struct expression;

typedef struct expression {
  enum expression_type type;
  // @Todo: work position in?
  union {
    struct expression *exp; // return value
    token tok;
    struct fn_call *fn_call;
  } val;
} expression;

struct fn_call {
  token name;
  size_t params_len;
  expression *params;
};


void parse(token* tokens);
#endif
