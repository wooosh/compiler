#include "type.h"
const char* builtin_types[] = {
  "u8", "u16", "u32", "u64", "uint",
  "s8", "s16", "s32", "s64", "sint",
  "byte",
  "void",
};

// @Todo: handle structs and stuff
bool type_equal(type a, type b) {
  return a.type == b.type;
}
