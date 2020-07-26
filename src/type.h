#ifndef TYPE_H
#define TYPE_H
#include <stdbool.h>

#define NUM_BUILTIN_TYPES 12
const char *builtin_types[NUM_BUILTIN_TYPES];
enum type_type {
  // @Todo: float stuff
  tt_u8,
  tt_u16,
  tt_u32,
  tt_u64,
  tt_uint,
  tt_s8,
  tt_s16,
  tt_s32,
  tt_s64,
  tt_sint,
  tt_byte,
  tt_void,
  tt_ptr,
  tt_array, // @Consider: is an array just something that implements []
            // operator?
  tt_struct
};

typedef struct type {
  enum type_type type;
  union {
    // will be used for storing struct/array/ptr type info
  };
} type;

bool type_equal(type a, type b);
#endif
