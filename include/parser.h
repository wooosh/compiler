#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "stdbool.h"
#include "type.h"

typedef struct token_buf {
  token *tokens;
  size_t idx;
} token_buf;

token tb_pop(token_buf *tb);
void tb_unpop(token_buf *tb);
token tb_peek(token_buf *tb);

enum expression_type {
  // statements
  e_fn_call,
  e_return,
  e_assign,
  e_if, // Includes else
  e_declaration,

  /*
  e_while,
  e_for,
  e_match
  */

  // non-statements
  e_integer_literal,
  e_reference,
  e_cast
};

typedef struct expression {
  enum expression_type type;
  // @Todo: work position in?
  union {
    struct expression *exp;  // return value
    token tok;               // variable reference @Todo: fix
    struct fn_call *fn_call; // @Todo: figure out why this is a pointer
    struct declaration *decl;
    struct assignment *assign;
    struct if_stmt *if_stmt;
    struct cast *cast;
    struct int_literal* int_literal;
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

// @Todo: split AST and parser stuff into seperate headers
void statement_mode_error(bool statement, token t, char *found);
expression parse_expression(token_buf *tb, bool statement, int rbp);
// @Todo: rename to reflect token_types, not actual types
int compare_types(token t, enum token_type ts[], char *desc);

#define try_pop_types(TB, T, DESC, TYPES...)                                   \
  T = tb_pop(TB);                                                              \
  if (!compare_types(T, (enum token_type[]){TYPES, t_EOF}, DESC))              \
    exit(1);
#define try_pop_type try_pop_types
#endif
