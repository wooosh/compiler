#include "lexer.h"
#include "parser.h"

#include <stdbool.h>
#include "type.h"
#include "misc.h"
// @Todo: make lexical analysis output multiple errors
type parse_type(token tok) {
  type t;
  for (int i=0; i<NUM_BUILTIN_TYPES; i++) {
    if (strcmp(tok.str.data, builtin_types[i]) == 0) {
      printf("%s\n", builtin_types[i]);
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

void read_function_signature(function* f) {
  // parse return type and param types
  f->return_type = parse_type(f->return_type_tok);
  for (int i=0; i<f->params.length; i++) {
    f->params.data[i].type = parse_type(f->params.data[i].type_tok);
  }
  
}

bool coerces(expression e, type t) {
  switch(e.type) {
  case e_fn_call:
    return type_equal(e.fn_call->fn->return_type, t);
  default:
    printf("warning: automatically coercing unknown type\n");
    return true;
  }
}

void read_expression(expression* e, vec_function ftable) {
  switch(e->type) {
  case e_fn_call: {
    // check if function exists
    struct fn_call* fnc = e->fn_call;
    token name = fnc->name;
    
    for (int i=0; i<fnc->params.length; i++) {
          read_expression(&fnc->params.data[i], ftable);
    }

    for (int i=0; i<ftable.length; i++) {
      // @Todo: make symbol and function table be shared for lambdas and stuff
      if (strcmp(ftable.data[i].name.str.data, name.str.data) == 0) {
        // @Todo: match function signature
        // @Todo: figure out constant type coercion with coerces_to(uint, types[])
        function fn = ftable.data[i];
        if (fn.params.length != fnc->params.length) continue;
        
        for (int i=0; i<fn.params.length; i++) {
          if (!coerces(fnc->params.data[i], fn.params.data[i].type)) {
            goto next;
          }
        }
        
        fnc->fn = &ftable.data[i];
        return;
      }
      next:;
    }
    // @Todo: print "unknown function" if no function with matching name
    // and "no function with matching signature" otherwise
    printf("%s: Unknown function named '%s'\n", token_location(name), name.str.data);
    print_token_loc(name);
    exit(1);
    break;
  }
  default:
    printf("unhandled type when reading expression\n");
    //exit(1);
  }
}

void analyse(vec_function fv) {
  // @Todo: create type tables
  // @Todo: create canonical symbol names for generics and multiple dispatch,
  // which function calls structs will contain a pointer to
  
  // build function table
  for (int i=0; i<fv.length; i++) {
    read_function_signature(&fv.data[i]);
  }
  
  // validate functions & body
  for (int i=0; i<fv.length; i++) {
    for (int j=0; j<fv.data[i].body.length; j++) {
      read_expression(&fv.data[i].body.data[j], fv);
    }
  }
}

int main(int argc, char **argv) {
  analyse(parse(lex(argv[1])));

  return 0;
}
