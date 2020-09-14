#include <gc.h>
#define malloc GC_MALLOC
#define realloc GC_REALLOC

#include "analysis.h"
#include "lexer.h"
#include "parser.h"

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

bool coerces(parser_state* p, expression e, type t) {
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
  case e_reference: {
    if (get_symbol(p, e->tok.str.data, 0).name == NULL) {
      printf("%s: Cannot reference '%s' before it is declared\n", token_location(e->tok), e->tok.str.data);
      print_token_loc(e->tok);
      exit(1);
    }
    
    break;
  }

  case e_declaration: {
    struct declaration *decl = e->decl;
    if (get_symbol(p, decl->name.str.data, p->scope_indexes.length).name !=
        NULL) {
      // @Todo: show other definition
      printf("%s: Symbol '%s' already declared\n", token_location(decl->name),
             decl->name.str.data);
      print_token_loc(decl->name);
      exit(1);
    }
    decl->type = parse_type(decl->type_tok);
    symbol s = {decl->is_const, decl->name.str.data, decl->type};
    vec_push(&p->symbol_stack, s);
    break;
  }
  
  case e_assign: {
    read_expression(p, &e->assign->value);
    symbol s = get_symbol(p, e->assign->name.str.data, 0);
    if (s.name == NULL) {
      printf("%s: Cannot assign to '%s' before it is declared\n", token_location(e->assign->name), e->assign->name.str.data);
      print_token_loc(e->assign->name);
      exit(1);
    }
    
    if (s.is_const) {
      // @Todo: add location of definition
      printf("%s: Cannot assign to '%s' because it is defined as const\n", token_location(e->assign->name), e->assign->name.str.data);
      print_token_loc(e->assign->name);
      exit(1);
    }
      
    if (!coerces(p, e->assign->value, s.type)) {
      // @Todo: add location of definition
      // @Todo: print mismatched types
      printf("%s: Mismatched types\n", token_location(e->assign->name));
      print_token_loc(e->assign->name);
      exit(1);
    };
    break;
  }

  case e_fn_call: {
    // check if function exists
    struct fn_call *fnc = e->fn_call;
    token name = fnc->name;

    for (int i = 0; i < fnc->params.length; i++) {
      read_expression(p, &fnc->params.data[i]);
    }
    // @Todo: use symbol table instead of function table
    for (int i = 0; i < p->symbol_stack.length; i++) {
      // @Todo: make symbol and function table be shared for lambdas and stuff
      symbol s = p->symbol_stack.data[i];
      if (s.type.type == tt_fn && strcmp(s.name, name.str.data) == 0) {
        // @Todo: match function signature
        // @Todo: figure out constant type coercion with coerces_to(uint,
        // types[])
        function *fn = s.type.fn;
        if (fn->params.length != fnc->params.length)
          continue;

        for (int i = 0; i < fn->params.length; i++) {
          if (!coerces(p, fnc->params.data[i], fn->params.data[i].type)) {
            goto next;
          }
        }

        fnc->fn = fn;
        return;
      }
    next:;
    }
    // @Todo: print "unknown function" if no function with matching name
    // @Todo: print type signatures
    // and "no function with matching signature" otherwise
    printf("%s: Unknown function named '%s' with type signature given\n", token_location(name),
           name.str.data);
    print_token_loc(name);
    exit(1);
  }

  case e_return:
    // @Todo: read_expression of return value
    read_expression(p, e->exp);
    return;

  case e_integer_literal: {
    // No type checking needed
    return;
  }

  default:
    printf("unhandled type when reading expression %d\n", e->type);
    // exit(1);
  }
}

vec_param_pair builtin_params(struct param_pair *params, size_t num_params) {
  vec_param_pair vparams;
  vec_init(&vparams);
  vec_pusharr(&vparams, params, num_params);
  return vparams;
}

token builtin_token(char *str) {
  vec_char_t vstr;
  vec_init(&vstr);
  vec_pusharr(&vstr, str, strlen(str) + 1);
  return (token){t_identifier, {NULL, 0, 0, 0, 0}, vstr};
}

parser_state analyse(vec_function fv) {
  // @Todo: create type tables
  // @Todo: create canonical symbol names for generics and multiple dispatch,
  // which function calls structs will contain a pointer to
  parser_state p;
  vec_init(&p.scope_indexes);
  vec_init(&p.symbol_stack);

  // @Todo: move to builtins.h
  // add builtins to symbol table
  // @Todo: add type annotations
  function *add = malloc(sizeof(function));
  add->builtin = true;
  add->return_type_tok = builtin_token("sint");
  add->return_type = (type){tt_sint};
  add->name = builtin_token("+");
  add->params = builtin_params(
      (struct param_pair[]){
          {builtin_token("a"), builtin_token("sint"), (type){tt_sint}},
          {builtin_token("b"), builtin_token("sint"), (type){tt_sint}},
      },
      2);
  symbol add_s = (symbol){true, "+", (type){tt_fn, add}};
  vec_push(&p.symbol_stack, add_s);

  function *mult = malloc(sizeof(function));
  mult->builtin = true;
  mult->return_type_tok = builtin_token("sint");
  mult->return_type = (type){tt_sint};
  mult->name = builtin_token("*");
  mult->params = builtin_params(
      (struct param_pair[]){
          {builtin_token("a"), builtin_token("sint"), (type){tt_sint}},
          {builtin_token("b"), builtin_token("sint"), (type){tt_sint}},
      },
      2);

  // needs to be a seperate line because vec_push is a macro
  symbol mult_s = (symbol){true, "*", (type){tt_fn, mult}};
  vec_push(&p.symbol_stack, mult_s);

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
