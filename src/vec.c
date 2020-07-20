#include "vec.h"
#include <stddef.h>
#include <stdlib.h>

// @Todo: error handling

vec* v_init(size_t elem_size) {
  vec* v = malloc(sizeof(vec));
  v->elem_size = elem_size;
  v->size = 1;
  v->len = 0;
  v->data = malloc(v->elem_size * v->size);
  return v;
}

void* v_get(vec* v, size_t idx) {
  return &(v->data[idx * v->elem_size]);
}

void v_set(vec* v, size_t idx, void* data) {
  ((char*)v->data)[idx * v->elem_size] = data;
}

void v_push(vec* v, void* data) {
  if (v->len == v->size) {
    v->size *= 2;
    v->data = realloc(v->data, v->elem_size * v->size);
  }
  v_set(v, v->len, data);
  v->len++;
}
