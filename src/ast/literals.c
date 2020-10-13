#include "analysis.h"
#include "ast.h"
#include "codegen.h"
#include "misc.h"
#include "parser.h"

#include <llvm-c/Core.h>

expression parse_literal(token_buf *tb, token t, bool statement) {
  if (statement)
    statement_mode_error(statement, t, "integer literal");
  expression e;
  e.type = e_integer_literal;
  e.tok = t;
  return e;
}

LLVMValueRef generate_literal(struct state *state, expression e) {
  // @Todo: coerce to proper type
  // @Todo: tag integer literals with their determined type during analysis
  return LLVMConstInt(LLVMInt32Type(), e.tok.integer, true);
}
