#include "analysis.h"
#include "ast.h"
#include "codegen.h"
#include "misc.h"
#include "parser.h"

#include <llvm-c/Core.h>

// Reference
expression parse_reference(token_buf *tb) {
  expression e;
  e.type = e_reference;
  e.tok = tb_pop(tb);
  return e;
}

void analyze_reference(parser_state *p, expression *e) {
  if (get_symbol(p, e->tok.str.data, 0).name == NULL) {
    printf("%s: Cannot reference '%s' before it is declared\n",
           token_location(e->tok), e->tok.str.data);
    print_token_loc(e->tok);
    exit(1);
  }
}

LLVMValueRef generate_reference(struct state *state, expression e) {
  return LLVMBuildLoad(state->b, get_c_symbol(state, e.tok.str.data),
                       "reference");
}

// Assignment
expression parse_assignment(token_buf *tb) {
  struct assignment *a = malloc(sizeof(struct assignment));
  a->name = tb_pop(tb);
  tb_pop(tb); // pop equals sign
  a->value = parse_expression(tb, false, 0);

  expression e;
  e.type = e_assign;
  e.assign = a;

  return e;
}

void analyze_assignment(parser_state *p, expression *e) {
  read_expression(p, &e->assign->value);
  symbol s = get_symbol(p, e->assign->name.str.data, 0);

  // Make sure variable has been declared
  if (s.name == NULL) {
    printf("%s: Cannot assign to '%s' before it is declared\n",
           token_location(e->assign->name), e->assign->name.str.data);
    print_token_loc(e->assign->name);
    exit(1);
  }

  // Make sure variable is not const
  if (s.is_const) {
    // @Todo: add location of definition
    printf("%s: Cannot assign to '%s' because it is defined as const\n",
           token_location(e->assign->name), e->assign->name.str.data);
    print_token_loc(e->assign->name);
    exit(1);
  }

  // Make sure the types match
  if (!coerces(p, &e->assign->value, s.type, true)) {
    // @Todo: add location of definition
    // @Todo: print mismatched types
    printf("%s: Mismatched types\n", token_location(e->assign->name));
    print_token_loc(e->assign->name);
    exit(1);
  };
}

void generate_assignment(struct state *state, expression e) {
  LLVMBuildStore(state->b, exp_to_val(state, e.assign->value),
                 get_c_symbol(state, e.assign->name.str.data));
}

// Variable definitions
expression parse_let(token_buf *tb) {
  expression e;
  tb_pop(tb); // pop "let" token
  e.type = e_declaration;
  e.decl = malloc(sizeof(struct declaration));
  e.decl->is_const = false;
  try_pop_type(tb, e.decl->name, "variable name", t_identifier);
  try_pop_type(tb, e.decl->type_tok, "type", t_identifier);
  return e;
}

void analyze_decl(parser_state *p, expression *e) {
  struct declaration *decl = e->decl;
  // Make sure the symbol is not already declared in the current scope
  if (get_symbol(p, decl->name.str.data, p->scope_indexes.length).name !=
      NULL) {
    // @Todo: show other definition
    printf("%s: Symbol '%s' already declared\n", token_location(decl->name),
           decl->name.str.data);
    print_token_loc(decl->name);
    exit(1);
  }
  decl->type = parse_type(decl->type_tok);
  symbol s = {decl->is_const, decl->name.str.data, decl->type};
  vec_push(&p->symbol_stack, s);
}

void generate_decl(struct state *state, expression e) {
  c_symbol s = {
      LLVMBuildAlloca(state->b, to_llvm_type(e.decl->type), "variable"),
      e.decl->name.str.data};
  vec_push(&state->symbol_stack, s);
}
