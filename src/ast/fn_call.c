#include "analysis.h"
#include "ast.h"
#include "codegen.h"
#include "misc.h"
#include "parser.h"

#include <llvm-c/Core.h>

expression parse_fn_call(token_buf *tb, token t, bool statement) {
  expression e;
  e.type = e_fn_call;
  struct fn_call *fnc = malloc(sizeof(struct fn_call));
  fnc->name = t;
  // @Todo: quit if not lparen
  tb_pop(tb); // skip lparen
  vec_init(&fnc->params);
  if (tb_pop(tb).type != t_rparen) {
    tb_unpop(tb);
    do {
      vec_push(&fnc->params, parse_expression(tb, false, 0));
      try_pop_types(tb, t, NULL, t_comma, t_rparen);
    } while (t.type != t_rparen);
  }
  e.fn_call = fnc;
  return e;
}

void analyze_fn_call(parser_state *p, expression *e) {
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

      // Check if parameters are type compatible
      for (int i = 0; i < fn->params.length; i++) {
        // @Todo: use casts from coerces
        if (!coerces(p, &fnc->params.data[i], fn->params.data[i].type, false)) {
          goto next;
        }
      }
      
      // Write any neccesary typecasts
      for (int i = 0; i < fn->params.length; i++) {
        // @Todo: use casts from coerces
        coerces(p, &fnc->params.data[i], fn->params.data[i].type, true);
      }
      
      fnc->fn = fn;
      return;
    }
  next:;
  }
  // @Todo: print "unknown function" if no function with matching name
  // @Todo: print type signatures
  // and "no function with matching signature" otherwise
  printf("%s: Unknown function named '%s' with type signature given\n",
         token_location(name), name.str.data);
  print_token_loc(name);
  exit(1);
}

LLVMValueRef generate_fn_call(struct state *state, expression e) {
  struct fn_call fnc = *e.fn_call;
  if (fnc.fn->builtin) {
    switch (fnc.name.str.data[0]) {
    case '+':
      return LLVMBuildAdd(state->b, exp_to_val(state, fnc.params.data[0]),
                          exp_to_val(state, fnc.params.data[1]), "addtmp");
    case '*':
      return LLVMBuildMul(state->b, exp_to_val(state, fnc.params.data[0]),
                          exp_to_val(state, fnc.params.data[1]), "multtmp");
    }
  }
  // @Todo: support normal functions
  printf("unknown function; only builtins are supported currently\n");
  exit(1);
}
