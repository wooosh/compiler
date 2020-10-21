#include "analysis.h"

token builtin_token(char *str) {
  vec_char_t vstr;
  vec_init(&vstr);
  vec_pusharr(&vstr, str, strlen(str) + 1);
  return (token){t_identifier, {NULL, 0, 0, 0, 0}, vstr};
}

token type_token(type t) { return builtin_token("type"); }

void add_builtin(parser_state *p, char *name, type return_type, type *args,
                 char **argnames, size_t num_args) {
  function *fn = malloc(sizeof(function));

  fn->builtin = true;
  fn->return_type_tok = type_token(return_type);
  fn->return_type = return_type;
  fn->name = builtin_token(name);

  vec_param_pair vparams;
  vec_init(&fn->params);
  for (int i = 0; i < num_args; i++) {
    struct param_pair p = {builtin_token(argnames[i]), type_token(args[i]),
                           args[i]};
    vec_push(&fn->params, p);
  }

  symbol fn_s = (symbol){true, name, (type){tt_fn, fn}};
  vec_push(&p->symbol_stack, fn_s);
}

void add_builtins(parser_state *p) {
  // @Todo: add math operators for all types
  add_builtin(p, "+", (type){tt_s64}, (type[]){{tt_s64}, {tt_s64}},
              (char *[]){"a", "b"}, 2);
  add_builtin(p, "*", (type){tt_s64}, (type[]){{tt_s64}, {tt_s64}},
              (char *[]){"a", "b"}, 2);
  add_builtin(p, "-", (type){tt_s64}, (type[]){{tt_s64}, {tt_s64}},
              (char *[]){"a", "b"}, 2);
}
