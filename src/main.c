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

void print_token_loc(token t) {
	FILE* f = fopen(t.pos.filename, "rb");
	fseek(f, t.pos.line_start, SEEK_SET);
	char* line = NULL;
	size_t len = 0;
	getline(&line, &len, f);
	printf("   |%s   |", line);
	// @Cleanup: change to printf padding
	printf("%*s", t.pos.col-1, "");
	printf("\x1b[31m");
	for (int i=0; i<t.pos.len; i++) printf("^");
	printf("\x1b[0m\n");
}

// @Todo: truncate multiline tokens
// @Bug: Doesn't handle eof
int compare_types(token t, enum token_type ts[], char* desc) {
	size_t len = 0;
	for (int i=0; ts[i] != t_EOF; i++) {
		if (ts[i] == t.type) {
			return 1;
		}
		len++;
	}
	// Didn't match any of the types
	if (len == 1) {
		if (desc == NULL) {
			printf(
				"%s Expected %s, got %s\n",
				token_location(t),
				token_type_str(ts[0]),
				token_type_str(t.type)
			);
		} else {
			printf(
				"%s Expected %s (%s), got %s\n",
				token_location(t),
				desc,
				token_type_str(ts[0]),
				token_type_str(t.type)
			);
		}
	} else {
		if (desc == NULL)
			printf("%s Expected ", token_location(t));
		else
			printf("%s Expected %s (", token_location(t), desc);
		char* spacer = ", ";
		for (int i=0; i<len; i++) {
			// @Cleanup: dumb
			if (i + 2 == len) {
				spacer = " or ";
			} else if (i + 1 == len) {
				spacer = "";
			}
			printf("%s%s", token_type_str(ts[i]), spacer);
		}
		if (desc != NULL) printf(")");
		printf(", got %s\n", token_type_str(t.type));
	}
	print_token_loc(t);
	return 0;
}

// Sets T to the token
#define try_pop_types(TB, T, DESC, TYPES...) \
	T = tb_pop(TB); \
	if (!compare_types(T, (enum token_type[]) {TYPES, t_EOF}, DESC)) return;
#define try_pop_type try_pop_types

// @Consider: Expected return type (identifier), got literal
// @Consider: macro to automatically get value of token if its a type
// @Consider: how to handle unexpected tokens
void parse(token_buf* tb) {
    // Continually try to parse functions
    token t;
    //compare_types(t_identifier, (enum token_type[]){t_EOF, t_literal}); 
    
    while(1) {
    	// @Todo: add parse_type()
        try_pop_type(tb, t, "return type", t_identifier);
        printf("%s\n", t.val.str);
        try_pop_type(tb, t, "function name", t_identifier);
        printf("%s\n", t.val.str);
        try_pop_type(tb, t, "opening parenthesis", t_lparen); // opening paren
        do {
        	token param_type, param_name;
		try_pop_type(tb, param_type, "parameter type", t_identifier);
		try_pop_type(tb, param_name, "parameter name", t_identifier);
		printf("param: %s %s\n", param_type.val.str, param_name.val.str);
       		try_pop_types(tb, t, NULL, t_comma, t_rparen);
       	} while (t.type != t_rparen);
    }
}

int main(int argc, char** argv) {
    token_buf tb = {lex(argv[1]), 0};
    parse(&tb);

    return 0;
}
