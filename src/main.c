#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

enum keyword {
    k_return
};

enum token_type {
    t_keyword,
    t_identifier,
    t_literal, // only numbers for now, TODO: split into multiple types
    t_lparen,
    t_rparen,
    t_lbrace,
    t_rbrace,
    t_comma,
    t_semicolon,
    t_EOF
};

// TODO: add file index
typedef struct token {
    enum token_type type;
    union {
        char* str; // identifer, str literal
        int integer; // num literal
        enum keyword keyword; // keyword
    } val;
} token;

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
    default: return "unknown";
  };
}

void eat_whitespace(FILE* f) {
  char c;
  while ((c = getc(f)) && (c == ' ' || c == '\t' || c == '\n'));
  ungetc(c, f);
}

token read_token(FILE* f) {
  token t;
  eat_whitespace(f);
  char c = getc(f);
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
  
  ungetc(c, f);
  // Read number
  if (c == '-' || isdigit(c)) {
    // TODO: error handling
    int n = fscanf(f, "%d", &t.val.integer);
    t.type = t_literal;
    return t;
  }
  
  // Read identifier/keyword
  char* identifier = malloc(1);
  size_t size = 1;
  size_t len = 0;
  // TODO: eof handling
  while ((c = getc(f)) && (isalnum(c) || c == '_')) {
    if (len == size) {
      size *=2;
      identifier = realloc(identifier, size);
    }
    identifier[len] = c;
    len++;
  }
  if (len > 0) {
    ungetc(c, f);
    t.val.str = identifier;
    t.type = t_identifier;
    return t;
  }
  
  printf("%c", c);
  getc(f); // for debug so we don't loop forever
  
}

void lex(char* filename) {
    FILE *f;
    f = fopen(filename, "rb");

    token t;
    while ((t = read_token(f)).type != t_EOF) printf("%s\n", token_str(t));
}

int main(int argc, char** argv) {
    lex(argv[1]);
    
    return 0;
}
