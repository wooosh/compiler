#include "analysis.h"
#include "ast.h"
#include "codegen.h"
#include "parser.h"

#include <llvm-c/Core.h>

expression parse_if_stmt(token_buf *tb, token t, bool statement) {
  expression e;
  if (!statement)
    statement_mode_error(statement, t, "if statement");
  // initialize struct to be an if statment
  e.type = e_if;
  e.if_stmt = malloc(sizeof(struct if_stmt));
  vec_init(&e.if_stmt->body);
  vec_init(&e.if_stmt->else_body);

  e.if_stmt->cond = parse_expression(tb, false, 0);

  // @Todo: make parse_block
  token bracket;
  try_pop_type(tb, bracket, "'{'", t_lbrace);
  do {
    vec_push(&e.if_stmt->body, parse_expression(tb, true, 0));
  } while (tb_peek(tb).type != t_rbrace);
  tb_pop(tb); // pop closing brace

  if (tb_peek(tb).type == t_else) {
    tb_pop(tb); // pop else
    token bracket;
    try_pop_type(tb, bracket, "'{'", t_lbrace);
    do {
      vec_push(&e.if_stmt->else_body, parse_expression(tb, true, 0));
    } while (tb_peek(tb).type != t_rbrace);
    tb_pop(tb); // pop closing brace
  }
  return e;
}

void analyze_if_stmt(parser_state *p, expression *e) {
  // @Todo: analyze_vec_expression
  // @Todo: analyze condition
  enter_scope(p);
  for (int i = 0; i < e->if_stmt->body.length; i++) {
    read_expression(p, &e->if_stmt->body.data[i]);
  }
  exit_scope(p);
  enter_scope(p);
  for (int i = 0; i < e->if_stmt->else_body.length; i++) {
    read_expression(p, &e->if_stmt->else_body.data[i]);
  }
  exit_scope(p);
}

void generate_if_stmt(struct state *state, expression e) {
  LLVMValueRef cond = exp_to_val(state, e.if_stmt->cond);
  // @Todo: cast to boolean instead of using int32
  LLVMValueRef cmp =
      LLVMBuildICmp(state->b, LLVMIntEQ, cond,
                    LLVMConstInt(LLVMInt32Type(), 1, true), "ifcond");

  LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(state->fn, "then");
  LLVMBasicBlockRef else_block =
      LLVMCreateBasicBlockInContext(LLVMGetGlobalContext(), "else");
  LLVMBasicBlockRef merge_block =
      LLVMCreateBasicBlockInContext(LLVMGetGlobalContext(), "ifcont");

  LLVMBuildCondBr(state->b, cmp, then_block, else_block);

  // Emit then block
  LLVMPositionBuilderAtEnd(state->b, then_block);

  // @Todo: create new scope in if statement
  vec_expression body = e.if_stmt->body;
  for (int i = 0; i < body.length; i++) {
    generate_statement(state, body.data[i]);
  }

  // Add a terminator if the block does not already have one
  then_block = LLVMGetInsertBlock(state->b);
  if (LLVMGetBasicBlockTerminator(then_block) == NULL)
    LLVMBuildBr(state->b, merge_block);

  // Emit else block
  LLVMAppendExistingBasicBlock(state->fn, else_block);
  LLVMPositionBuilderAtEnd(state->b, else_block);

  vec_expression else_body = e.if_stmt->else_body;
  for (int i = 0; i < else_body.length; i++) {
    generate_statement(state, else_body.data[i]);
  }

  else_block = LLVMGetInsertBlock(state->b);
  if (LLVMGetBasicBlockTerminator(else_block) == NULL)
    LLVMBuildBr(state->b, merge_block);

  // Emit merge block
  LLVMAppendExistingBasicBlock(state->fn, merge_block);
  LLVMPositionBuilderAtEnd(state->b, merge_block);
}
