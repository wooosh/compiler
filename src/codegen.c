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

#include "llvm-c/Core.h"
#include "llvm-c/ExecutionEngine.h"
#include "llvm-c/Initialization.h"
#include "llvm-c/Object.h"
#include "llvm-c/Support.h"
#include "llvm-c/Target.h"
#include "llvm-c/TargetMachine.h"
#include "llvm-c/Types.h"
#include <llvm-c/Analysis.h>

// @Todo: rename all structs so naming conflicts don't happen
// codegen symbol
typedef struct c_symbol {
  // alloc'd location
  LLVMValueRef v;
  char *name;
} c_symbol;

typedef vec_t(c_symbol) vec_c_symbol;
// used to store codegen internal state
struct state {
  LLVMBuilderRef b;

  // vec_int_t scope_indexes;
  vec_c_symbol symbol_stack;
};

LLVMValueRef get_c_symbol(struct state *state, char *name) {
  for (int i = state->symbol_stack.length - 1; i >= 0; i--) {
    if (strcmp(name, state->symbol_stack.data[i].name) == 0) {
      return state->symbol_stack.data[i].v;
    }
  }
}

LLVMValueRef exp_to_val(struct state *state, expression e) {
  switch (e.type) {
  case e_integer_literal:
    // @Todo: coerce to proper type
    // @Todo: tag integer literals with their determined type during analysis
    return LLVMConstInt(LLVMInt32Type(), e.tok.integer, true);
  case e_reference:
    return LLVMBuildLoad(state->b, get_c_symbol(state, e.tok.str.data),
                         "reference");
  case e_fn_call: {
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
      // Intentional fall through
    }
    // Intentional fall through
  }
  default:
    printf("??? exp_to_val %d\n", e.type);
    exit(1);
  }
}

void codegen(parser_state p) {
  // codegen
  LLVMInitializeNativeAsmPrinter();
  LLVMInitializeNativeTarget();
  LLVMLoadLibraryPermanently("c");

  LLVMModuleRef module = LLVMModuleCreateWithName("main");
  LLVMSetTarget(module, LLVMGetDefaultTargetTriple());

  struct state state;
  // vec_init(&state.vec_int_t);
  vec_init(&state.symbol_stack);

  // generate code for functions
  for (int i = 0; i < p.fv.length; i++) {
    function fn_data = p.fv.data[i];
    printf("Generating %s\n", fn_data.name.str.data);
    // @Todo: parameters
    LLVMTypeRef params[0] = {};
    LLVMTypeRef functionType = LLVMFunctionType(LLVMInt32Type(), params, 0, 0);

    // set up function
    LLVMValueRef fn_llvm =
        LLVMAddFunction(module, fn_data.name.str.data, functionType);
    LLVMBuilderRef builder = LLVMCreateBuilder();
    LLVMBasicBlockRef entryBlock = LLVMAppendBasicBlock(fn_llvm, "entry");
    LLVMPositionBuilderAtEnd(builder, entryBlock);

    state.b = builder;
    // generate function body
    for (int j = 0; j < fn_data.body.length; j++) {
      expression e = fn_data.body.data[j];
      switch (e.type) {
      case e_declaration: {
        c_symbol s = {LLVMBuildAlloca(builder, LLVMInt32Type(), "variable"),
                      e.decl->name.str.data};
        vec_push(&state.symbol_stack, s);
        break;
      }
      case e_assign: {
        LLVMBuildStore(state.b, exp_to_val(&state, e.assign->value),
                       get_c_symbol(&state, e.assign->name.str.data));
        break;
      }
      case e_return:
        LLVMBuildRet(builder, exp_to_val(&state, *e.exp));
        break;
      default:
        printf("??? unknown statement\n");
        exit(1);
      }
    }

    char *error = NULL;
    LLVMVerifyModule(module, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error);
    LLVMDumpModule(module);

    LLVMExecutionEngineRef engine;
    LLVMLinkInMCJIT();
    LLVMBool result =
        LLVMCreateJITCompilerForModule(&engine, module, 3, &error);
    if (result) {
      printf("Failed to initialize: %s\n", error);
      return;
    }
    int (*example)() = (int (*)())LLVMGetPointerToGlobal(engine, fn_llvm);
    printf("result: %d\n", example());
  }
}
