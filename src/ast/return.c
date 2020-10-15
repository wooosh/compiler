#include "analysis.h"
#include "ast.h"
#include "codegen.h"
#include "misc.h"
#include "parser.h"

#include <llvm-c/Core.h>

expression parse_return(token_buf *tb) {
  tb_pop(tb); // pop return token off

  expression e;
  e.type = e_return;
  expression *return_value = malloc(sizeof(expression));
  *return_value = parse_expression(tb, false, 0);
  e.exp = return_value;
  return e;
}

void analyze_return(parser_state *p, expression *e) {
  read_expression(p, e->exp);

  if (!coerces(p, e->exp, p->current_fn.return_type, true)) {
    // @Todo: proper error
    printf("ERROR INVALID CAST\n");
  }
}

void generate_return(struct state *state, expression e) {
  LLVMBuildRet(state->b, exp_to_val(state, *e.exp));
}
