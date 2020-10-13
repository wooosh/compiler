#include "analysis.h"
#include "ast.h"
#include "codegen.h"
#include "misc.h"
#include "parser.h"

#include <llvm-c/Core.h>

expression parse_return(token_buf *tb, token t, bool statement) {
  if (!statement)
    statement_mode_error(statement, t, "return statement");

  expression e;
  e.type = e_return;
  expression *return_value = malloc(sizeof(expression));
  *return_value = parse_expression(tb, false, 0);
  e.exp = return_value;
  return e;
}

void analyze_return(parser_state *p, expression *e) {
  read_expression(p, e->exp);
}

void generate_return(struct state *state, expression e) {
  LLVMBuildRet(state->b, exp_to_val(state, *e.exp));
}
