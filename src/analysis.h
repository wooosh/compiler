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

parser_state analyse(vec_function fv);
#endif
