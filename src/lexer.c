#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"

char *token_str(token t) {
  switch (t.type) {
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
    size_t len = snprintf(NULL, 0, "%d", t.val.integer) + 1;
    char *num = malloc(len);
    snprintf(num, len, "%d", t.val.integer);
    return num;
  case t_identifier:
    return t.val.str;
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
  size_t len =
      snprintf(NULL, 0, "%s:%zu:%zu", t.pos.filename, t.pos.row, t.pos.col) + 1;
  char *buf = malloc(len);
  snprintf(buf, len - 1, "%s:%zu:%zu", t.pos.filename, t.pos.row, t.pos.col);
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
  char *str = malloc(1);
  size_t size = 1;
  size_t len = 0;
  bool is_num = true;

  // @Todo: eof handling
  while ((c = wgetc(f)) && (isalnum(c) || c == '_')) {
    if (len == size) {
      size *= 2;
      str = realloc(str, size);
    }
    // @Todo: prevent identifiers that start with a number
    // @Todo: handle hex, binary, negatives etc
    // @Todo: floats
    if (is_num && !isdigit(c))
      is_num = false;
    str[len] = c;
    len++;
  }
  if (len > 0) {
    wungetc(c, f);
    if (is_num) {
      // @Todo: read into unsigned variable, and negate if first char is '-'
      sscanf(str, "%d", &t.val.integer);
      t.type = t_literal;
      return t;
    } else {
      // Check if keyword
      if (strcmp("return", str) == 0) {
        t.type = t_return;
        return t;
      }

      // No match with keywords, identifier
      t.val.str = str;
      t.type = t_identifier;
      return t;
    }
  }

  printf("%s Found unknown character: '%c'\n", token_location(t), c);
  exit(1);
}

token *lex(char *filename) {
  tracked_file f = {fopen(filename, "rb"), filename, 1, 1, 0};
  if (f.f == NULL)
    perror("lexer");

  size_t size = 1;
  size_t len = 0;
  token *tokens = malloc(size * sizeof(token));
  token t;
  do {
    t = read_token(&f);
    t.pos.len = f.col - t.pos.col;
    if (len == size) {
      size *= 2;
      tokens = realloc(tokens, size * sizeof(token));
    }

    tokens[len] = t;
    len++;

    printf("%s:%zu:%zu: %s\n", t.pos.filename, t.pos.row, t.pos.col,
           token_str(t));
  } while (t.type != t_EOF);

  fclose(f.f);
  return tokens;
}
