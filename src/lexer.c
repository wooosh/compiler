#include <gc.h>
#define malloc GC_MALLOC
#define realloc GC_REALLOC

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "vec.h"

char *token_str(token t) {
  switch (t.type) {
  case t_let:
    return "let";
  case t_lparen:
    return "(";
  case t_rparen:
    return ")";
  case t_lbrace:
    return "{";
  case t_rbrace:
    return "}";
  case t_comma:
    return ",";
  case t_literal:;
    size_t len = snprintf(NULL, 0, "%d", t.integer) + 1;
    char *num = malloc(len);
    snprintf(num, len, "%d", t.integer);
    return num;
  case t_operator:
  case t_identifier:
    return t.str.data;
  case t_return:
    return "return";
  case t_EOF:
    return "EOF";
  default:
    return "unknown";
  };
}

char *token_type_str(enum token_type t) {
  switch (t) {
  case t_let:
    return "let";
  case t_operator:
    return "operator";
  case t_lparen:
    return "'('";
  case t_rparen:
    return "')'";
  case t_lbrace:
    return "'{'";
  case t_rbrace:
    return "'}'";
  case t_comma:
    return "','";
  case t_literal:
    return "integer literal";
  case t_identifier:
    return "identifier";
  case t_EOF:
    return "EOF";
  default:
    return "unknown";
  };
}

// Using a tracked file lets you automatically get a file location identifier,
// useful for errors, eg "test.c:33:35: error message"
typedef struct tracked_file {
  FILE *f;
  char *filename;
  size_t row; // AKA line
  size_t col; // AKA character
  size_t last_row_len;
  // @Bug: will break on multiline tokens
  size_t line_start;
  size_t last_line_start;
  size_t idx;
} tracked_file;

// Wrapped getc that operates on tracked files
char wgetc(tracked_file *t) {
  char c = getc(t->f);
  t->idx++;
  if (c == '\n') {
    t->row++;
    t->last_row_len = t->col;
    t->col = 1;

    t->last_line_start = t->line_start;
    t->line_start = t->idx;
  } else {
    t->col++;
  }
  return c;
}

int wungetc(char c, tracked_file *t) {
  t->idx--;
  if (c == '\n') {
    t->col = t->last_row_len;
    t->row--;
    t->line_start = t->last_line_start;
  } else if (t->col > 1) {
    t->col--;
  }
  return ungetc(c, t->f);
}

void set_token_len(tracked_file *tf, token *t) {}

char *token_location(token t) {
  size_t len = snprintf(NULL, 0, "%s:%zu:%zu", t.pos.filename, t.pos.row,
                        t.pos.col + 1) +
               1;
  char *buf = malloc(len);
  snprintf(buf, len, "%s:%zu:%zu", t.pos.filename, t.pos.row, t.pos.col + 1);
  return buf;
}

void eat_whitespace(tracked_file *f) {
  char c;
  while ((c = wgetc(f)) && (c == ' ' || c == '\t' || c == '\n'))
    ;
  wungetc(c, f);
}

token read_token(tracked_file *f) {
  eat_whitespace(f);

  token t = {t_unknown, {f->filename, f->row, f->col, f->line_start}};

  char c = wgetc(f);
  if (c == EOF) {
    t.type = t_EOF;
    return t;
  }

  switch (c) {
  case '+':
  case '*':
    t.type = t_operator;
    vec_init(&t.str);
    vec_push(&t.str, c);
    vec_push(&t.str, 0);
    return t;
  case '=':
    t.type = t_equals;
    return t;
  case ',':
    t.type = t_comma;
    return t;
  case '(':
    t.type = t_lparen;
    return t;
  case ')':
    t.type = t_rparen;
    return t;
  case '{':
    t.type = t_lbrace;
    return t;
  case '}':
    t.type = t_rbrace;
    return t;
  }

  wungetc(c, f);

  // Read identifier/keyword/number
  vec_char_t str;
  vec_init(&str);
  bool is_num = true;

  // @Todo: eof handling
  while ((c = wgetc(f)) && (isalnum(c) || c == '_')) {
    // @Todo: prevent identifiers that start with a number
    // @Todo: handle hex, binary, negatives etc
    // @Todo: floats
    if (is_num && !isdigit(c))
      is_num = false;
    vec_push(&str, c);
  }
  str.data[str.length] = '\0';
  if (str.length > 0) {
    wungetc(c, f);
    if (is_num) {
      // @Todo: read into unsigned variable, and negate if first char is '-'
      sscanf(str.data, "%d", &t.integer);
      t.type = t_literal;
      return t;
    } else {
      // Check if keyword
      if (strcmp("return", str.data) == 0) {
        t.type = t_return;
        return t;
      } else if (strcmp("let", str.data) == 0) {
        t.type = t_let;
        return t;
      }

      // No match with keywords, identifier
      t.str = str;
      t.type = t_identifier;
      return t;
    }
  }

  printf("%s Found unknown character: '%c'\n", token_location(t), c);
  exit(1);
}

vec_token lex(char *filename) {
  tracked_file f = {fopen(filename, "rb"), filename, 1, 1, 0};
  if (f.f == NULL)
    perror("lexer");

  vec_token tokens;
  vec_init(&tokens);

  token t;
  do {
    t = read_token(&f);
    t.pos.len = f.col - t.pos.col;
    vec_push(&tokens, t);
  } while (t.type != t_EOF);

  for (int i = 0; i < tokens.length; i++) {
    t = tokens.data[i];
    printf("%s:%zu:%zu: %s\n", t.pos.filename, t.pos.row, t.pos.col,
           token_str(t));
  }

  fclose(f.f);
  return tokens;
}
