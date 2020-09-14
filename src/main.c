#include <gc.h>
#define malloc GC_MALLOC
#define realloc GC_REALLOC

#include "lexer.h"
#include "parser.h"
#include "analysis.h"
#include "codegen.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("error: one input file must be provided\n");
    return 1;
  }
  GC_INIT();
  codegen(analyse(parse(lex(argv[1]))));

  return 0;
}
