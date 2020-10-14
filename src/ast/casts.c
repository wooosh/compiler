#include "analysis.h"
#include "ast.h"
#include "codegen.h"
#include "misc.h"
#include "parser.h"

#include <llvm-c/Core.h>

LLVMValueRef generate_cast(struct state *state, expression e) {
  if (is_scalar(e.cast->from) && is_scalar(e.cast->to)) {
    // upcast
    if (is_higher_precision(e.cast->to, e.cast->from)) {
      return LLVMBuildCast(state->b, LLVMSExt, exp_to_val(state, e.cast->exp), to_llvm_type(e.cast->to), "upcast");
    }
    // @Todo: downcast
  } 
  // @Todo: proper error
  printf("Cannot generate cast\n");
  exit(1); 
}
