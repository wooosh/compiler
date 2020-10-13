#include <gc.h>
#define malloc GC_MALLOC
#define realloc GC_REALLOC

#include "analysis.h"
#include "ast.h"
#include "builtins.h"
#include "lexer.h"
#include "parser.h"

// @Todo: rename parser_state to analyzer_state
// @Todo: make return check that the return value is on the same row

#include "misc.h"
#include "type.h"
#include "vec.h"
#include <stdbool.h>

// @Todo: make lexical analysis output multiple errors

void enter_scope(parser_state *p) {
  vec_push(&p->scope_indexes, p->symbol_stack.length);
}

void exit_scope(parser_state *p) {
  p->symbol_stack.length = vec_last(&p->scope_indexes);
  vec_pop(&p->scope_indexes);
}

// max_scope is the highest scope level to search, used for checking
// if a symbol already exists in the local scope. 0 means search all scopes
symbol get_symbol(parser_state *p, char *name, int max_scope) {
  for (int i = p->symbol_stack.length - 1;
       i >= 0 && i >= p->scope_indexes.data[max_scope]; i--) {
    if (strcmp(name, p->symbol_stack.data[i].name) == 0) {
      return p->symbol_stack.data[i];
    }
  }
  return (symbol){false, NULL};
}

type parse_type(token tok) {
  type t;
  for (int i = 0; i < NUM_BUILTIN_TYPES; i++) {
    if (strcmp(tok.str.data, builtin_types[i]) == 0) {
      t.type = i;
      return t;
    }
  }
  // @Todo: make print_token_loc take a token and an error message, so
  // we don't have to manually call token_location
  printf("%s: Unknown type\n", token_location(tok), tok.str.data);
  print_token_loc(tok);
  exit(1);
}

void read_function_signature(function *f) {
  // parse return type and param types
  f->return_type = parse_type(f->return_type_tok);
  for (int i = 0; i < f->params.length; i++) {
    f->params.data[i].type = parse_type(f->params.data[i].type_tok);
  }
}

bool coerces(parser_state *p, expression e, type t) {
  switch (e.type) {
  case e_reference:
    return type_equal(get_symbol(p, e.tok.str.data, 0).type, t);
  case e_fn_call:
    return type_equal(e.fn_call->fn->return_type, t);
  // @Todo: add casts to for non-sint's
  case e_integer_literal:
    return t.type == tt_sint;
  default:
    printf("warning: automatically coercing unknown type\n");
    return true;
  }
}

void read_expression(parser_state *p, expression *e) {
  switch (e->type) {
  case e_fn_call:
    return analyze_fn_call(p, e);
  case e_reference:
    return analyze_reference(p, e);
  case e_assign:
    return analyze_assignment(p, e);
  case e_declaration:
    return analyze_decl(p, e);
  case e_return:
    return analyze_return(p, e);
  case e_if:
    return analyze_if_stmt(p, e);
  case e_integer_literal:
    return;
  default:
    printf("unhandled type when reading expression %d\n", e->type);
    // exit(1);
  }
}

parser_state analyse(vec_function fv) {
  // @Todo: analyze type definitions and create type lookups in parser state
  // @Todo: create canonical symbol names for generics and multiple dispatch,
  // which function calls structs will contain a pointer to
  parser_state p;
  vec_init(&p.scope_indexes);
  vec_init(&p.symbol_stack);

  // @Todo: move to builtins.h
  // add builtins to symbol table
  add_builtins(&p);

  // build function table
  for (int i = 0; i < fv.length; i++) {
    read_function_signature(&fv.data[i]);
    // add function signature to symbol table
    symbol s = {true, fv.data[i].name.str.data, {tt_fn, &fv.data[i]}};
    vec_push(&p.symbol_stack, s);
  }

  /*
  // display global scope
  printf("\nGLOBAL SCOPE:\n");
  for (int i = 0; i < p.symbol_stack.length; i++) {
    symbol s = p.symbol_stack.data[i];
    printf("%s %s\n", s.is_const ? "const" : "", s.name);
  }*/

  // validate functions & body
  for (int i = 0; i < fv.length; i++) {
    enter_scope(&p);
    for (int j = 0; j < fv.data[i].body.length; j++) {
      read_expression(&p, &fv.data[i].body.data[j]);
    }
    exit_scope(&p);
  }

  p.fv = fv;
  return p;
}
