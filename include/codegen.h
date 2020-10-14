#ifndef CODEGEN_H
#define CODEGEN_H
#include "analysis.h"
#include "parser.h"
#include "type.h"

#include <llvm-c/Core.h>

// @Todo: prefix structs
typedef struct c_symbol {
  // alloc'd location
  LLVMValueRef v;
  char *name;
} c_symbol;

typedef vec_t(c_symbol) vec_c_symbol;
// used to store codegen internal state
struct state {
  LLVMBuilderRef b;
  LLVMValueRef fn; // Current function in codegen
  // vec_int_t scope_indexes;
  vec_c_symbol symbol_stack;
};

LLVMValueRef get_c_symbol(struct state *state, char *name);

LLVMTypeRef to_llvm_type(type a);
LLVMValueRef exp_to_val(struct state *state, expression e);
void generate_statement(struct state *state, expression e);
void codegen(parser_state p);
#endif
