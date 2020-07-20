#include "lexer.h"
#include "parser.h""

int main(int argc, char **argv) {
  parse(lex(argv[1]));

  return 0;
}
