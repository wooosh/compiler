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
    t_EOF,
    t_unknown
};

typedef struct token_pos {
    char* filename;
    size_t row; // AKA line
    size_t col; // AKA character
    size_t line_start;
    size_t line_idx;
} token_pos;

// TODO: add file index
typedef struct token {
    enum token_type type;
    token_pos pos;
    union {
        char* str; // identifer, str literal
        int integer; // num literal
        enum keyword keyword; // keyword
    } val;
} token;

char* token_location(token t);
char* token_str(token t);
char* token_type_str(enum token_type t);
token* lex(char* filename);
