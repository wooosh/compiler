#ifndef AST_H
#define AST_H
// header for everything in ast/
#include "analysis.h"
#include "codegen.h"
#include "parser.h"

#include <llvm-c/Core.h>

// Casts
struct cast {
  expression exp;
  type from;
  type to;
};

LLVMValueRef generate_cast(struct state *state, expression e);

// Function calls
struct fn_call {
  token name;
  vec_expression params;
  function *fn;
};

expression parse_fn_call(token_buf *tb, token t, bool statement);
void analyze_fn_call(parser_state *p, expression *e);
LLVMValueRef generate_fn_call(struct state *state, expression e);

// Literals
// @Todo: distinguish between literal types, as eventually we will have >1 type
// of literals (eg arrays/strings/struct definitions, numbers)
struct int_literal {
  token val;
  type type;
};
expression parse_literal(token_buf *tb, token t, bool statement);
LLVMValueRef generate_literal(struct state *state, expression e);

// Variables
struct assignment {
  token name;
  expression value;
};

expression parse_assignment(token_buf *tb, token t, bool statement);
void analyze_assignment(parser_state *p, expression *e);
void generate_assignment(struct state *state, expression e);

expression parse_reference(token_buf *tb, token t, bool statement);
void analyze_reference(parser_state *p, expression *e);
LLVMValueRef generate_reference(struct state *state, expression e);

struct declaration {
  token name;
  token type_tok;
  type type;
  expression initial_value;
  bool is_const;
  // @Todo: add optional assign here
};

expression parse_let(token_buf *tb, token t, bool statement);
void analyze_decl(parser_state *p, expression *e);
void generate_decl(struct state *state, expression e);

// Control flow
expression parse_return(token_buf *tb, token t, bool statement);
void analyze_return(parser_state *p, expression *e);
void generate_return(struct state *state, expression e);

struct if_stmt {
  expression cond;
  vec_expression body;
  vec_expression else_body;
};

expression parse_if_stmt(token_buf *tb, token t, bool statement);
void analyze_if_stmt(parser_state *p, expression *e);
void generate_if_stmt(struct state *state, expression e);
#endif
