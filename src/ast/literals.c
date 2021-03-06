#include "analysis.h"
#include "ast.h"
#include "codegen.h"
#include "misc.h"
#include "parser.h"

#include <llvm-c/Core.h>

expression parse_literal(token_buf *tb) {
  expression e;
  token t = tb_pop(tb); // get literal token
  e.type = e_integer_literal;
  e.int_literal = malloc(sizeof(struct int_literal));
  e.int_literal->val = t;
  return e;
}

LLVMValueRef generate_literal(struct state *state, expression e) {
  // @Todo: coerce to proper type
  // @Todo: tag integer literals with their determined type during analysis
  return LLVMConstInt(to_llvm_type(e.int_literal->type),
                      e.int_literal->val.integer, true);
}
