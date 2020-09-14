#include <gc.h>
#define malloc GC_MALLOC
#define realloc GC_REALLOC

#include "lexer.h"
#include "parser.h"
#include "analysis.h"

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

LLVMValueRef exp_to_val(LLVMBuilderRef builder, expression e) {
  switch (e.type) {
  case e_integer_literal:
    // @Todo: coerce to proper type
    // @Todo: tag integer literals with their determined type during analysis
    return LLVMConstInt(LLVMInt32Type(), e.tok.integer, true);
  case e_fn_call: {
    struct fn_call fnc = *e.fn_call;
    if (fnc.fn->builtin) {
      switch (fnc.name.str.data[0]) {
      case '+':
        return LLVMBuildAdd(builder, exp_to_val(builder, fnc.params.data[0]),
                            exp_to_val(builder, fnc.params.data[1]), "addtmp");
      case '*':
        return LLVMBuildMul(builder, exp_to_val(builder, fnc.params.data[0]),
                            exp_to_val(builder, fnc.params.data[1]), "multtmp");
      }
      // Intentional fall through
    }
    // Intentional fall through
  }
  default:
    printf("???\n");
    exit(1);
  }
}

void codegen(parser_state p) {
  printf("STARTING LLVM CODE GEN:\n");
  // codegen
  LLVMInitializeNativeAsmPrinter();
  LLVMInitializeNativeTarget();
  LLVMLoadLibraryPermanently("c");

  LLVMModuleRef module = LLVMModuleCreateWithName("main");
  LLVMSetTarget(module, LLVMGetDefaultTargetTriple());

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

    // generate function body
    for (int j = 0; j < fn_data.body.length; j++) {
      expression e = fn_data.body.data[i];
      switch (e.type) {
      case e_return:
        LLVMBuildRet(builder, exp_to_val(builder, *e.exp));
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
        LLVMCreateJITCompilerForModule(&engine, module, 0, &error);
    if (result) {
      printf("Failed to initialize: %s\n", error);
      return;
    }
    int (*example)() = (int (*)())LLVMGetPointerToGlobal(engine, fn_llvm);
    printf("result: %d\n", example());
  }
  
}
