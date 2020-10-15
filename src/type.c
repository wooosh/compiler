#include "type.h"
#include <llvm-c/Core.h>
// @Todo: use designated initializers to ensure enum value matches the string
// @Todo: s/NULL/"invalid"
const char *builtin_types[] = {
    "invalid", "u8",  "u16",         "u32",     "u64",  "s8",   "s16",
    "s32",     "s64", "int_literal", "invalid", "bool", "void",
};

const int num_bits[] = {8, 16, 32, 64, 8, 16, 32, 64};

bool is_scalar(type a) { return a.type > scalar_base && a.type < scalar_end; }

// if a bits > b bits
bool is_higher_precision(type a, type b) {
  return num_bits[a.type - scalar_base - 1] >
         num_bits[b.type - scalar_base - 1];
}

bool is_signed(type a) {
  return a.type == tt_int_literal || (a.type >= tt_s8 && a.type <= tt_s64);
}

// @Todo: handle structs and stuff
bool type_equal(type a, type b) { return a.type == b.type; }
