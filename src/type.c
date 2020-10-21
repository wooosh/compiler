#include "type.h"
#include <llvm-c/Core.h>
// used to convert strings into types
const char *builtin_types[tt_end] = {
    [tt_u8] = "u8",     [tt_u16] = "u16",   [tt_u32] = "u32", [tt_u64] = "u64",

    [tt_s8] = "s8",     [tt_s16] = "s16",   [tt_s32] = "s32", [tt_s64] = "s64",

    [tt_bool] = "bool", [tt_void] = "void",
};

const int num_bits[tt_end] = {
    [tt_u8] = 8, [tt_u16] = 16, [tt_u32] = 32, [tt_u64] = 64,

    [tt_s8] = 8, [tt_s16] = 16, [tt_s32] = 32, [tt_s64] = 64,
};

bool is_scalar(type a) { return a.type > scalar_base && a.type < scalar_end; }

// if a bits > b bits
bool is_higher_precision(type a, type b) {
  return num_bits[a.type] > num_bits[b.type];
}

bool is_signed(type a) {
  return a.type == tt_int_literal || (a.type >= tt_s8 && a.type <= tt_s64);
}

// @Todo: handle structs and stuff
bool type_equal(type a, type b) { return a.type == b.type; }
