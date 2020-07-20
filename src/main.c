#include "lexer.h"
#include <stdbool.h>

// @Todo: type system
// @Todo: parser
// @Todo: backend

typedef struct token_buf {
  token *tokens;
  size_t idx;
} token_buf;

token tb_peak(token_buf *tb) {
  return tb->tokens[tb->idx];
}

// @Todo: check for EOF
token tb_pop(token_buf *tb) {
  tb->idx++;
  return tb->tokens[tb->idx - 1];
}

// @Bug: will make idx negative if used at start of array
void tb_unpop(token_buf *tb) {
  tb->idx--;
}
  

struct param_pair {
  token name;
  token type;
};

typedef struct function {
  token return_type;
  token name;
  size_t params_len;
  struct param_pair *params;
} function;

void print_token_loc(token t) {
  FILE *f = fopen(t.pos.filename, "rb");
  fseek(f, t.pos.line_start, SEEK_SET);
  char *line = NULL;
  size_t len = 0;
  getline(&line, &len, f);
  printf("   |%s   |", line);
  // @Cleanup: change to printf padding
  printf("%*s", t.pos.col - 1, "");
  printf("\x1b[31m");
  for (int i = 0; i < t.pos.len; i++)
    printf("^");
  printf("\x1b[0m\n");
}

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


// @Cleanup: use token_str
void print_function(function fn) {
  printf("\nname: %s\nreturn type: %s\nparams:\n", fn.name.val.str,
         fn.return_type.val.str);
  for (int i = 0; i < fn.params_len; i++) {
    printf("  %s %s\n", fn.params[i].type.val.str, fn.params[i].name.val.str);
  }
}

enum expression_type {
  // statements
  e_fn_call,
  e_return,
  
  // non-statements
  e_integer_literal,
  e_variable // @Consider: change to ref?
};

struct fn_call;
struct expression;

typedef struct expression {
  enum expression_type type;
  // @Todo: work position in?
  union {
    struct expression* exp; // return value
    int num;
    char* name;
    struct fn_call* fn_call;
  } val;
} expression;

struct fn_call {
  char* name;
  size_t params_len;
  expression* params;
};

expression parse_expression(token_buf *tb, bool statement) {
  expression e;
  token t = tb_pop(tb);
  switch(t.type) {
    // @Cleanup
    case t_return:
      e.type = e_return;
      expression* return_value = malloc(sizeof(expression));
      *return_value = parse_expression(tb, false);
      e.val.exp = return_value;
      return e;
    case t_identifier: // fn call or var
      if (tb->tokens[tb->idx].type == t_lparen) { // fn call
        e.type = e_fn_call;
        struct fn_call* fnc = malloc(sizeof(expression));
        fnc->name = t.val.str;
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
      } else { // variable
        e.type = e_variable;
        e.val.name = t.val.str;
        return e;
      }
      case t_literal:
        e.type = e_integer_literal;
        e.val.num = t.val.integer;
        return e;
      default:;
  }
  printf("???\n"); // @Todo: proper error message
  exit(0);
}

// @Cleanup: clean recursive expression printer
void print_expression(expression e) {
  switch(e.type) {
    case e_return:
      printf("return ");
      print_expression(*e.val.exp);
      break;
    case e_integer_literal:
      printf("%d", e.val.num);
      break;
    case e_fn_call:
      printf("%s(", e.val.fn_call->name);
      for (int i=0; i<e.val.fn_call->params_len; i++) {
        print_expression(e.val.fn_call->params[i]);
        if (i < e.val.fn_call->params_len - 1)
        printf(", ");
      }
      printf(")");
      break;
    case e_variable:
      printf(e.val.name);
      break;
    default:
      printf("???\n");
  }
}

// @Bug: handle EOF
void parse(token_buf *tb) {
  // Continually try to parse functions
  token t;
  // compare_types(t_identifier, (enum token_type[]){t_EOF, t_literal});

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
          fn.params = realloc(fn.params, params_size * sizeof(struct param_pair));
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
    while (tb->tokens[tb->idx].type != t_rbrace) {
      print_expression(parse_expression(tb, true));
      printf("\n");
    }
    tb_pop(tb);
    print_function(fn);
  }
}

int main(int argc, char **argv) {
  token_buf tb = {lex(argv[1]), 0};
  parse(&tb);

  return 0;
}
