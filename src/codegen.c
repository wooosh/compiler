#include <gc.h>
#define malloc GC_MALLOC
#define realloc GC_REALLOC

#include "analysis.h"
#include "ast.h"
#include "lexer.h"
#include "options.h"
#include "parser.h"
// @Todo: make return check that the return value is on the same row

#include "misc.h"
#include "type.h"
#include "vec.h"
#include <stdbool.h>

#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Initialization.h>
#include <llvm-c/Object.h>
#include <llvm-c/Support.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Types.h>

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
    return generate_literal(state, e);
  case e_reference:
    return generate_reference(state, e);
  case e_fn_call:
    return generate_fn_call(state, e);
  default:
    printf("??? exp_to_val %d\n", e.type);
    exit(1);
  }
}

void generate_statement(struct state *state, expression e) {
  switch (e.type) {
  // @Todo: add function call statement
  case e_if:
    return generate_if_stmt(state, e);
  case e_assign:
    return generate_assignment(state, e);
  case e_declaration:
    return generate_decl(state, e);
  case e_return:
    return generate_return(state, e);
  default:
    printf("??? unknown statement\n");
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
    state.fn = fn_llvm;
    // @Todo: make this a function that can call itself so nested stuff works
    // generate function body
    for (int j = 0; j < fn_data.body.length; j++) {
      generate_statement(&state, fn_data.body.data[j]);
    }

    if (debug_dump_ir)
      LLVMDumpModule(module);

    char *error = NULL;
    LLVMVerifyModule(module, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error);

    if (jit) {
      LLVMExecutionEngineRef engine;
      LLVMLinkInMCJIT();
      // @Todo: enter into main function
      LLVMBool result =
          LLVMCreateJITCompilerForModule(&engine, module, 3, &error);
      if (result) {
        printf("Failed to initialize: %s\n", error);
        return;
      }
      int (*example)() = (int (*)())LLVMGetPointerToGlobal(engine, fn_llvm);
      printf("%d\n", example());
    } else {
      printf("Outputting executables not implemented yet; use JIT\n");
      exit(1);
    }
  }
}
