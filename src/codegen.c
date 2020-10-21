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

LLVMTypeRef to_llvm_type(type a) {
  switch (a.type) {
  case tt_void:
    return LLVMVoidType();

  case tt_u8:
  case tt_s8:
    return LLVMInt8Type();

  case tt_u16:
  case tt_s16:
    return LLVMInt16Type();

  case tt_u32:
  case tt_s32:
    return LLVMInt32Type();

  case tt_u64:
  case tt_s64:
    return LLVMInt64Type();

  // @Todo: proper error
  default:
    printf("can't translate type %d\n", a.type);
    exit(1);
  }
}

LLVMValueRef exp_to_val(struct state *state, expression e) {
  switch (e.type) {
  case e_cast:
    return generate_cast(state, e);
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

  LLVMBuilderRef builder = LLVMCreateBuilder();

  LLVMValueRef main_fn = NULL;
  // declare all functions in the symbol table
  for (int i = 0; i < p.fv.length; i++) {
    // @Todo: move long struct member chains into variables
    // @Todo: parameters
    // @Todo: move parameter set up to another function
    LLVMTypeRef *params = malloc(sizeof(LLVMTypeRef)*p.fv.data[i].params.length);
    for (int j=0; j < p.fv.data[i].params.length; j++) {
      params[j] = to_llvm_type(p.fv.data[i].params.data[j].type);
    }


    LLVMTypeRef functionType =
        LLVMFunctionType(to_llvm_type(p.fv.data[i].return_type), params, p.fv.data[i].params.length, 0);

    // set up function
    p.fv.data[i].fn_llvm =
        LLVMAddFunction(module, p.fv.data[i].name.str.data, functionType);

    if (strcmp(p.fv.data[i].name.str.data, "main") == 0) {
      main_fn = p.fv.data[i].fn_llvm;
    }

    c_symbol s = {p.fv.data[i].fn_llvm, p.fv.data[i].name.str.data};
    vec_push(&state.symbol_stack, s);
  }

  // generate code for functions
  for (int i = 0; i < p.fv.length; i++) {
    function fn_data = p.fv.data[i];
    LLVMBasicBlockRef entryBlock =
        LLVMAppendBasicBlock(fn_data.fn_llvm, "entry");

    LLVMPositionBuilderAtEnd(builder, entryBlock);

    state.b = builder;
    state.fn = fn_data.fn_llvm;
    for (int j = 0; j < fn_data.params.length; j++) {
      c_symbol s = {                                                                 
      LLVMBuildAlloca(state.b, to_llvm_type(fn_data.params.data[j].type), "param"),         
      fn_data.params.data[j].name.str.data};
      LLVMBuildStore(state.b, LLVMGetParam(state.fn, j), s.v);
      vec_push(&state.symbol_stack, s);
    }

    // @Todo: make this a function that can call itself so nested stuff works
    // generate function body
    for (int j = 0; j < fn_data.body.length; j++) {
      generate_statement(&state, fn_data.body.data[j]);
    }
  }

  if (debug_dump_ir)
    LLVMDumpModule(module);

  char *error = NULL;
  LLVMVerifyModule(module, LLVMAbortProcessAction, &error);
  LLVMDisposeMessage(error);

  if (jit) {
    if (main_fn == NULL) {
      // @Todo: proper error message
      printf("no main fn found\n");
      exit(1);
    }
    LLVMExecutionEngineRef engine;
    LLVMLinkInMCJIT();
    // @Todo: enter into main function
    LLVMBool result =
        LLVMCreateJITCompilerForModule(&engine, module, 3, &error);
    if (result) {
      printf("Failed to initialize: %s\n", error);
      return;
    }
    int (*example)() = (int (*)())LLVMGetPointerToGlobal(engine, main_fn);
    printf("%d\n", example());
  } else if (!jit) {
    printf("Outputting executables not implemented yet; use JIT\n");
    exit(1);
  }
}
