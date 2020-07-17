#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "lexer.h"

// TODO: consider whether or not tokens should the text they represent

char* token_str(token t) {
  switch(t.type) {
    case t_lparen: return "(";
    case t_rparen: return ")";
    case t_lbrace: return "{";
    case t_rbrace: return "}";
    case t_comma: return ",";
    case t_semicolon: return ";";
    case t_literal: ;
      size_t len = snprintf(NULL, 0, "%d", t.val.integer) + 1;
      char* num = malloc(len);
      snprintf(num, len, "%d", t.val.integer);
      return num;
    case t_identifier: return t.val.str;
    case t_EOF: return "EOF";
    default: return "unknown";
  };
}

// Using a tracked file lets you automatically get a file location identifier,
// useful for errors, eg "test.c:33:35: error message"
typedef struct tracked_file {
    FILE* f;
    char* filename;
    size_t row; // AKA line
    size_t col; // AKA character
    size_t last_row_len;
} tracked_file;

// Wrapped getc that operates on tracked files 
char wgetc(tracked_file* t) {
    char c = getc(t->f);
    if (c == '\n') {
        t->row++;
        t->last_row_len = t->col;
        t->col = 1;
    } else {
        t->col++;
    }
    return c;
}

int wungetc(char c, tracked_file* t) {
    if (c == '\n') {
        t->col = t->last_row_len;
        t->row--;
    } else if (t->col > 1) {
        t->col--;
    }
    return ungetc(c, t->f);
}

void eat_whitespace(tracked_file* f) {
  char c;
  while ((c = wgetc(f)) && (c == ' ' || c == '\t' || c == '\n'));
  wungetc(c, f);
}

token read_token(tracked_file* f) {
  eat_whitespace(f);
  
  // @POTENTIALBUG: filename may be deallocated after the lexer is run, maybe
  // duplicate and free later?
  token t = {t_unknown,{f->filename, f->row, f->col}};
  
  char c = wgetc(f);
  if (c == EOF) {
    t.type = t_EOF;
    return t;
  }

  switch(c) {
    case ';':
      t.type = t_semicolon;
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

  // TODO: move this under identifiers, and check if the "identifier" starts
  // with a digit, this way we still maintain our file position with wgetc
  // Read number
  if (c == '-' || isdigit(c)) {
    // TODO: error handling
  
    int n = fscanf(f->f, "%d", &t.val.integer);
    t.type = t_literal;
    return t;
  }
  
  // Read identifier/keyword
  char* identifier = malloc(1);
  size_t size = 1;
  size_t len = 0;
  // TODO: eof handling
  while ((c = wgetc(f)) && (isalnum(c) || c == '_')) {
    if (len == size) {
      size *=2;
      identifier = realloc(identifier, size);
    }
    identifier[len] = c;
    len++;
  }
  if (len > 0) {
    wungetc(c, f);
    t.val.str = identifier;
    t.type = t_identifier;
    return t;
  }
  
  printf("%c", c);
  wgetc(f); // for debug so we don't loop forever
}

token* lex(char* filename) {
    tracked_file f = {fopen(filename, "rb"), filename, 1, 1, 0}; 
    if (f.f == NULL) perror("lexer");
    
    size_t size = 1;
    size_t len = 0;
    token* tokens = malloc(size*sizeof(token));
    token t;
    do {
        t = read_token(&f);
        if (len == size) {
            size *= 2;
            tokens = realloc(tokens, size*sizeof(token));
        }
        
        tokens[len] = t;
        len++;

        printf("%s:%zu:%zu: %s\n", t.pos.filename, t.pos.row, t.pos.col, token_str(t));
    } while (t.type != t_EOF);
    return tokens;
}
