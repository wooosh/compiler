#include "lexer.h"
void print_token_loc(token t) {
  FILE *f = fopen(t.pos.filename, "rb");
  fseek(f, t.pos.line_start, SEEK_SET);
  char *line = NULL;
  size_t len = 0;
  getline(&line, &len, f);
  printf("   |%s   |", line);
  // @Cleanup: change to printf padding
  printf("%*s", t.pos.col - 1, "");
  printf("\x1b[31m");
  for (int i = 0; i < t.pos.len; i++)
    printf("^");
  printf("\x1b[0m\n");
}
