#include <gc.h>
#define malloc GC_MALLOC
#define realloc GC_REALLOC

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

// @Todo: make lexical analysis output multiple errors
enum symbol_type { st_variable, st_function };

typedef struct symbol {
  // @Todo: add definition location
  bool is_const;
  char *name;
  type type;
} symbol;

typedef vec_t(struct symbol) vec_symbol;
typedef struct parser_state {
  vec_int_t scope_indexes;
  vec_symbol symbol_stack;
} parser_state;

void enter_scope(parser_state *p) {
  vec_push(&p->scope_indexes, p->symbol_stack.length);
}

void exit_scope(parser_state *p) {
  p->symbol_stack.length = vec_last(&p->scope_indexes);
  vec_pop(&p->scope_indexes);
}

// @Todo: delete
symbol *get_symbol(parser_state *p, char *name);

type parse_type(token tok) {
  type t;
  for (int i = 0; i < NUM_BUILTIN_TYPES; i++) {
    if (strcmp(tok.str.data, builtin_types[i]) == 0) {
      printf("%s\n", builtin_types[i]);
      t.type = i;
      return t;
    }
  }
  // @Todo: make print_token_loc take a token and an error message, so
  // we don't have to manually call token_location
  printf("%s: Unknown type\n", token_location(tok), tok.str.data);
  print_token_loc(tok);
  exit(1);
}

void read_function_signature(function *f) {
  // parse return type and param types
  f->return_type = parse_type(f->return_type_tok);
  for (int i = 0; i < f->params.length; i++) {
    f->params.data[i].type = parse_type(f->params.data[i].type_tok);
  }
}

bool coerces(expression e, type t) {
  switch (e.type) {
  case e_fn_call:
    return type_equal(e.fn_call->fn->return_type, t);
  // @Todo: add casts to for non-sint's
  case e_integer_literal:
    return t.type == tt_sint;
  default:
    printf("warning: automatically coercing unknown type\n");
    return true;
  }
}

void read_expression(parser_state *p, expression *e) {
  switch (e->type) {
  case e_fn_call: {
    // check if function exists
    struct fn_call *fnc = e->fn_call;
    token name = fnc->name;

    for (int i = 0; i < fnc->params.length; i++) {
      read_expression(p, &fnc->params.data[i]);
    }
    // @Todo: use symbol table instead of function table
    for (int i = 0; i < p->symbol_stack.length; i++) {
      // @Todo: make symbol and function table be shared for lambdas and stuff
      symbol s = p->symbol_stack.data[i];
      if (s.type.type == tt_fn && strcmp(s.name, name.str.data) == 0) {
        // @Todo: match function signature
        // @Todo: figure out constant type coercion with coerces_to(uint,
        // types[])
        function *fn = s.type.fn;
        if (fn->params.length != fnc->params.length)
          continue;

        for (int i = 0; i < fn->params.length; i++) {
          if (!coerces(fnc->params.data[i], fn->params.data[i].type)) {
            goto next;
          }
        }

        fnc->fn = fn;
        return;
      }
    next:;
    }
    // @Todo: print "unknown function" if no function with matching name
    // and "no function with matching signature" otherwise
    printf("%s: Unknown function named '%s'\n", token_location(name),
           name.str.data);
    print_token_loc(name);
    exit(1);
  }
  case e_return:
    // @Todo: read_expression of return value
    read_expression(p, e->exp);
    return;
  case e_integer_literal: {
    // No type checking needed
    return;
  }
  default:
    printf("unhandled type when reading expression\n");
    // exit(1);
  }
}

vec_param_pair builtin_params(struct param_pair *params, size_t num_params) {
  vec_param_pair vparams;
  vec_init(&vparams);
  vec_pusharr(&vparams, params, num_params);
  return vparams;
}

token builtin_token(char *str) {
  vec_char_t vstr;
  vec_init(&vstr);
  vec_pusharr(&vstr, str, strlen(str) + 1);
  return (token){t_identifier, {NULL, 0, 0, 0, 0}, vstr};
}

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

void analyse(vec_function fv) {
  // @Todo: create type tables
  // @Todo: create canonical symbol names for generics and multiple dispatch,
  // which function calls structs will contain a pointer to
  parser_state p;
  vec_init(&p.scope_indexes);
  vec_init(&p.symbol_stack);

  // @Todo: move to builtins.h
  // add builtins to symbol table
  // @Todo: add type annotations
  function add;
  add.builtin = true;
  add.return_type_tok = builtin_token("sint");
  add.return_type = (type){tt_sint};
  add.name = builtin_token("+");
  add.params = builtin_params(
      (struct param_pair[]){
          {builtin_token("a"), builtin_token("sint"), (type){tt_sint}},
          {builtin_token("b"), builtin_token("sint"), (type){tt_sint}},
      },
      2);
  symbol add_s = (symbol){true, "+", (type){tt_fn, &add}};
  vec_push(&p.symbol_stack, add_s);

  function mult;
  mult.builtin = true;
  mult.return_type_tok = builtin_token("sint");
  mult.return_type = (type){tt_sint};
  mult.name = builtin_token("*");
  mult.params = builtin_params(
      (struct param_pair[]){
          {builtin_token("a"), builtin_token("sint"), (type){tt_sint}},
          {builtin_token("b"), builtin_token("sint"), (type){tt_sint}},
      },
      2);

  // needs to be a seperate line because vec_push is a macro
  symbol mult_s = (symbol){true, "*", (type){tt_fn, &mult}};
  vec_push(&p.symbol_stack, mult_s);

  // build function table
  for (int i = 0; i < fv.length; i++) {
    read_function_signature(&fv.data[i]);
    // add function signature to symbol table
    symbol s = {true, fv.data[i].name.str.data, {tt_fn, &fv.data[i]}};
    vec_push(&p.symbol_stack, s);
  }

  // display global scope
  printf("\nGLOBAL SCOPE:\n");
  for (int i = 0; i < p.symbol_stack.length; i++) {
    symbol s = p.symbol_stack.data[i];
    printf("%s %s\n", s.is_const ? "const" : "", s.name);
  }

  // validate functions & body
  for (int i = 0; i < fv.length; i++) {
    for (int j = 0; j < fv.data[i].body.length; j++) {
      read_expression(&p, &fv.data[i].body.data[j]);
    }
  }

  printf("STARTING LLVM CODE GEN:\n");
  // codegen
  LLVMInitializeNativeAsmPrinter();
  LLVMInitializeNativeTarget();
  LLVMLoadLibraryPermanently("c");

  LLVMModuleRef module = LLVMModuleCreateWithName("main");
  LLVMSetTarget(module, LLVMGetDefaultTargetTriple());

  // generate code for functions
  for (int i = 0; i < fv.length; i++) {
    function fn_data = fv.data[i];
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
