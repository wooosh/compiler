#ifndef TYPE_H
#define TYPE_H
#include <stdbool.h>

enum type_type {
  // @Todo: float stuff
  // @Todo: add (un)signed base and (un)signed end
  // NOTE: scalar types are in promotion order
  scalar_base,
  // unsigned int
  tt_u8,
  tt_u16,
  tt_u32,
  tt_u64,

  // signed int
  tt_s8,
  tt_s16,
  tt_s32,
  tt_s64,

  tt_int_literal,
  scalar_end,

  // misc
  tt_bool,
  tt_void,
  tt_ptr,
  tt_array,
  tt_struct,
  tt_fn,
  tt_end 
};

const char *builtin_types[tt_end];

typedef struct function function;
typedef struct type {
  enum type_type type;
  union {
    struct function *fn;
    // ptr
    // struct
    // array
  };
} type;

bool is_higher_precision(type a, type b);
bool is_scalar(type a);
bool is_signed(type a);
bool type_equal(type a, type b);
#endif
