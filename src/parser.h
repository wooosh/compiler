#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "stdbool.h"
#include "type.h"

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
    struct fn_call *fn_call; // @Todo: figure out why this is a pointer
  };
} expression;
typedef vec_t(expression) vec_expression;

struct param_pair {
  token name;
  token type_tok;
  type type;
};

typedef vec_t(struct param_pair) vec_param_pair;
typedef struct function {
  type return_type;
  token return_type_tok;

  token name;

  vec_param_pair params;
  vec_expression body;

  bool builtin;
} function;

typedef vec_t(function) vec_function;
vec_function parse(vec_token tokens);

struct fn_call {
  token name;
  vec_expression params;
  function *fn;
};
#endif
