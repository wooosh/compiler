#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "stdbool.h"
#include "type.h"

enum expression_type {
  // statements
  e_fn_call,
  e_return,
  e_assign,
  /*
  e_assignment,
  e_if, // Includes else
  e_while,
  e_for,
  e_match
  */

  // non-statements
  e_integer_literal,
  e_reference, // @Consider: change to ref?
  e_declaration,
};

struct fn_call;
struct expression;
struct declaration;

typedef struct expression {
  enum expression_type type;
  // @Todo: work position in?
  union {
    struct expression *exp;  // return value
    token tok;               // variable reference
    struct fn_call *fn_call; // @Todo: figure out why this is a pointer
    struct declaration *decl;
    struct assignment *assign;
  };
} expression;
typedef vec_t(expression) vec_expression;

struct assignment {
  token name;
  expression value;
};

// @Todo: clean up inconsistent typedefs
struct declaration {
  token name;
  token type_tok;
  type type;
  expression initial_value;
  bool is_const;
  // @Todo: add optional assign here
};

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
