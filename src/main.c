#include "lexer.h"

// @TODO: type system
// @TODO: parser
// @TODO: backend

typedef struct token_buf {
    token* tokens;
    size_t idx;
} token_buf;

// @TODO: check for EOF
token tb_pop(token_buf* tb) {
    tb->idx++;
    return tb->tokens[tb->idx-1];
}

typedef struct function {
    // @TODO: parameters
    // @TODO: return value
    char* name;
} function;

void parse(token_buf* tb) {
    // Continually try to parse functions
    token t;
    while(1) {
        t = tb_pop(tb);
        if (t.type != t_identifier) printf("%s\n", token_location(t));
	if (t.type == t_EOF) break;
    }
}

int main(int argc, char** argv) {
    token_buf tb = {lex(argv[1]), 0};
    parse(&tb);

    return 0;
}
