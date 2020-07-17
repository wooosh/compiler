#include "lexer.h"

typedef struct token_buf {
    token* tokens;
    size_t index;
} token_buf;


int main(int argc, char** argv) {
    token_buf toks = {lex(argv[1]), 0};

//    printf("test\n");
    return 0;
}
