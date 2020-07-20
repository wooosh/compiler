#include "parser.h"
#include "lexer.h"
#include "misc.h"

#include <stdbool.h>

typedef struct token_buf {
  token *tokens;
  size_t idx;
} token_buf;

// @Todo: check for EOF
token tb_pop(token_buf *tb) {
  tb->idx++;
  return tb->tokens[tb->idx - 1];
}

// @Bug: will make idx negative if used at start of array
void tb_unpop(token_buf *tb) { tb->idx--; }

// @Todo: truncate multiline tokens
// @Bug: Doesn't handle eof
int compare_types(token t, enum token_type ts[], char *desc) {
  size_t len = 0;
  for (int i = 0; ts[i] != t_EOF; i++) {
    if (ts[i] == t.type) {
      return 1;
    }
    len++;
  }
  // Didn't match any of the types
  if (len == 1) {
    if (desc == NULL) {
      printf("%s Expected %s, got %s\n", token_location(t),
             token_type_str(ts[0]), token_type_str(t.type));
    } else {
      printf("%s Expected %s (%s), got %s\n", token_location(t), desc,
             token_type_str(ts[0]), token_type_str(t.type));
    }
  } else {
    if (desc == NULL)
      printf("%s Expected ", token_location(t));
    else
      printf("%s Expected %s (", token_location(t), desc);
    char *spacer = ", ";
    for (int i = 0; i < len; i++) {
      // @Cleanup: dumb
      if (i + 2 == len) {
        spacer = " or ";
      } else if (i + 1 == len) {
        spacer = "";
      }
      printf("%s%s", token_type_str(ts[i]), spacer);
    }
    if (desc != NULL)
      printf(")");
    printf(", got %s\n", token_type_str(t.type));
  }
  print_token_loc(t);
  return 0;
}

// Sets T to the token
#define try_pop_types(TB, T, DESC, TYPES...)                                   \
  T = tb_pop(TB);                                                              \
  if (!compare_types(T, (enum token_type[]){TYPES, t_EOF}, DESC))              \
    return;
#define try_pop_type try_pop_types

expression parse_expression(token_buf *tb, bool statement);

expression parse_fn_call(token_buf *tb) {
  expression e;
  e.type = e_fn_call;
  struct fn_call *fnc = malloc(sizeof(struct fn_call));
  token t = tb_pop(tb);
  fnc->name = t;
  tb_pop(tb); // skip lparen
  fnc->params_len = 0;
  if (tb_pop(tb).type != t_rparen) {
    tb_unpop(tb);
    size_t params_size = 1;
    fnc->params = malloc(params_size * sizeof(struct expression));
    do {
      if (fnc->params_len == params_size) {
        params_size *= 2;
        fnc->params = realloc(fnc->params, params_size * sizeof(expression));
      }
      fnc->params[fnc->params_len] = parse_expression(tb, false);
      fnc->params_len++;
      try_pop_types(tb, t, NULL, t_comma, t_rparen);
    } while (t.type != t_rparen);
    e.val.fn_call = fnc;
    return e;
  }
}

void statement_mode_error(bool statement, token t, char* found) {
  char* mode;
  if (statement) mode = "statement";
  else mode = "expression";
  
  printf("%s expected %s, found %s\n", token_location(t), mode, found);
  print_token_loc(t);
  exit(1);
}

expression parse_expression(token_buf *tb, bool statement) {
  expression e;
  token t = tb_pop(tb);
  switch (t.type) {
  // @Cleanup
  case t_return:
    if (!statement) statement_mode_error(statement, t, "return statement");
    e.type = e_return;
    expression *return_value = malloc(sizeof(expression));
    *return_value = parse_expression(tb, false);
    e.val.exp = return_value;
    return e;
  case t_identifier:                            // fn call or var
    if (tb->tokens[tb->idx].type == t_lparen) { // fn call
      tb_unpop(tb);
      return parse_fn_call(tb);
    } else { // variable
      if (statement) statement_mode_error(statement, t, "variable reference");
      e.type = e_variable;
      e.val.tok = t;
      return e;
    }
  case t_literal:
    if (statement) statement_mode_error(statement, t, "integer literal");
    e.type = e_integer_literal;
    e.val.tok = t;
    return e;
  default:;
  }
  printf("???\n"); // @Todo: proper error messaget
  exit(1);
}

// @Cleanup: clean recursive expression printer
void print_expression(expression e) {
  switch (e.type) {
  case e_return:
    printf("return ");
    print_expression(*e.val.exp);
    break;
  case e_integer_literal:
    printf("%d", e.val.tok.val.integer);
    break;
  case e_fn_call:
    printf("%s(", e.val.fn_call->name.val.str);
    for (int i = 0; i < e.val.fn_call->params_len; i++) {
      print_expression(e.val.fn_call->params[i]);
      if (i < e.val.fn_call->params_len - 1)
        printf(", ");
    }
    printf(")");
    break;
  case e_variable:
    printf(e.val.tok.val.str.data);
    break;
  default:
    printf("???\n");
  }
}

// @Cleanup: use token_str
void print_function(function fn) {
  printf("\nname: %s\nreturn type: %s\nparams:\n", fn.name.val.str.data,
         fn.return_type.val.str.data);
  for (int i = 0; i < fn.params_len; i++) {
    printf("  %s %s\n", fn.params[i].type.val.str.data, fn.params[i].name.val.str.data);
  }
  for (int i = 0; i < fn.body_len; i++) {
    print_expression(fn.body[i]);
    printf("\n");
  }
}

// @Bug: handle EOF
struct function_list parse(vec_token tokens) {
  // @Cleanup: super ugly
  token_buf tb_pre = {(token*)tokens.data, 0};
  token_buf* tb = &tb_pre;
  // Continually try to parse functions
  token t;
  
  size_t fl_size = 1;
  struct function_list fl =  {0, malloc(sizeof(function) * fl_size)};
  
  while (tb->tokens[tb->idx].type != t_EOF) {
    function fn;
    try_pop_type(tb, fn.return_type, "return type", t_identifier);
    try_pop_type(tb, fn.name, "function name", t_identifier);
    try_pop_type(tb, t, "opening parenthesis", t_lparen); // opening paren

    // if next token is not closing paren, read params
    fn.params_len = 0;
    if (tb_pop(tb).type != t_rparen) {
      tb_unpop(tb);
      // Read parameters
      size_t params_size = 1;
      fn.params = malloc(params_size * sizeof(struct param_pair));
      do {
        if (fn.params_len == params_size) {
          params_size *= 2;
          fn.params =
              realloc(fn.params, params_size * sizeof(struct param_pair));
        }
        try_pop_type(tb, fn.params[fn.params_len].type, "parameter type",
                     t_identifier);
        try_pop_type(tb, fn.params[fn.params_len].name, "parameter name",
                     t_identifier);
        fn.params_len++;
        try_pop_types(tb, t, NULL, t_comma, t_rparen);
      } while (t.type != t_rparen);
    }

    // Start parsing the function body
    try_pop_type(tb, t, NULL, t_lbrace);
    // @Cleanup: maybe restructure into do while?
    size_t body_size = 1;
    fn.body = malloc(sizeof(expression) * body_size);
    fn.body_len = 0;
    while (tb->tokens[tb->idx].type != t_rbrace) {
      if (fn.body_len == body_size) {
        body_size *= 2;
        fn.body = realloc(fn.body, body_size * sizeof(expression));
      }
      fn.body[fn.body_len] = parse_expression(tb, true);
      fn.body_len++;
    }
    tb_pop(tb);
    print_function(fn);
    if (fl.len == fl_size) {
        fl_size *= 2;
        fl.functions = realloc(fl.functions, fl_size * sizeof(function));
    }
    fl.functions[fl.len] = fn;
    fl.len++;
  }
  return fl;
}
