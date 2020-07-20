#ifndef VEC_H
#define VEC_H
#include <stddef.h>
typedef struct vec {
  size_t elem_size;
  size_t size;
  size_t len;
  void* data;
} vec;

vec* v_init(size_t elem_size);
void* v_get(vec* v, size_t idx);
void v_set(vec* v, size_t idx, void* data);
void v_push(vec* v, void* data);

#endif
