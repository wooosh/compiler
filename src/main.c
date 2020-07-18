#include "lexer.h"

// @Todo: type system
// @Todo: parser
// @Todo: backend

typedef struct token_buf {
    token* tokens;
    size_t idx;
} token_buf;

// @Todo: check for EOF
token tb_pop(token_buf* tb) {
    tb->idx++;
    return tb->tokens[tb->idx-1];
}

typedef struct function {
    // @Todo: parameters
    // @Todo: return value
    char* name;
} function;

#define try_pop_type(T, TB, TYPE)  \
	T = tb_pop(TB); \
	if (T.type != TYPE) { \
		printf( \
			"%s Expected type %s, got type %s\n", \
			token_location(T), \
			token_type_str(TYPE), \
			token_type_str(T.type) \
		); \
		if (T.type == t_EOF) break; \
        	else continue; \
       	}

void parse(token_buf* tb) {
    // Continually try to parse functions
    token t;
    while(1) {
    
        // pop return type
        try_pop_type(t, tb, t_identifier);
       	//printf("%s\n", t.val.str);
    }
}

int main(int argc, char** argv) {
    token_buf tb = {lex(argv[1]), 0};
    parse(&tb);

    return 0;
}
