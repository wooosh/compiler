#ifndef TYPE_H
#define TYPE_H
enum type_type {
  // @Todo: float stuff
  tt_u8, tt_u16, tt_u32, tt_u64, tt_uint,
  tt_i8, tt_i16, tt_i32, tt_i64, tt_int,
  tt_byte,
  tt_ptr,
  tt_array, // @Consider: is an array just something that implements [] operator?
  tt_struct
};

typedef struct type {
  enum type_type type;
  union {
    // will be used for storing struct/array/ptr type info
  } val;
} type;
#endif
