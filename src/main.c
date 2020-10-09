#include <gc.h>
#define malloc GC_MALLOC
#define realloc GC_REALLOC

#include "analysis.h"
#include "codegen.h"
#include "lexer.h"
#include "options.h"
#include "parser.h"

#include <stdbool.h>

bool debug_token_list = false;
bool debug_ast = false;
bool debug_dump_ir = false;
bool jit = false;

int main(int argc, char **argv) {
  char *source = NULL;
  // i=1 to skip basename
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'd': {
        if (++i < argc) {
          if (strcmp(argv[i], "lexer") == 0)
            debug_token_list = true;
          else if (strcmp(argv[i], "ast") == 0)
            debug_ast = true;
          else if (strcmp(argv[i], "IR") == 0)
            debug_dump_ir = true;
          else {
            printf("Unknown debug type: '%s'\n", argv[i]);
            exit(1);
          }
        } else {
          printf("Expected argument after -d\n");
          exit(1);
        }
        break;
      }
      case 'j': {
        jit = true;
        break;
      }
      }
    } else {
      if (source == NULL) {
        source = argv[i];
      } else {
        // @Todo: rephrase error
        printf("Only one input file is allowed: %s %s\n", source, argv[i]);
        exit(1);
      }
    }
  }
  if (source == NULL) {
    printf("No input file was provided\n");
    exit(1);
  }

  GC_INIT();
  codegen(analyse(parse(lex(source))));

  return 0;
}
