#ifndef ANALYSIS_H
#define ANALYSIS_H
#include "parser.h"

typedef struct symbol {
  // @Todo: add definition location
  bool is_const;
  char *name;
  type type;
} symbol;

typedef vec_t(struct symbol) vec_symbol;
typedef struct parser_state {
  vec_int_t scope_indexes;
  vec_symbol symbol_stack;
  vec_function fv;
} parser_state;

void enter_scope(parser_state *p);
void exit_scope(parser_state *p);
symbol get_symbol(parser_state *p, char *name, int max_scope);

// @Todo: s/parse/analyze/
type parse_type(token tok);

// will rewrite expression if write_cast is set
bool coerces(parser_state *p, expression *e, type t, bool write_cast);

void read_expression(parser_state *p, expression *e);

parser_state analyse(vec_function fv);
#endif
