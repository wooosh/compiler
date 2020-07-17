#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// @TODO: move tokens into own file(s)

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

char* token_str(token t);
void lex(char* filename);
