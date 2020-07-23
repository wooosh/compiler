#include "lexer.h"
#include "parser.h"

typedef struct test {
  union {
    char* str;
    int num;
  };
} test;

int main(int argc, char **argv) {
  test x;
  x.str = "bruh";
  printf("%s\n", x.str);
  parse(lex(argv[1]));

  return 0;
}
