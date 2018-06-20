/*
 * mm_alloc.h
 *
 * A clone of the interface documented in "man 3 malloc".
 */

#pragma once

#include <stdlib.h>

typedef long unsigned int address_t;

/* Struct for a block of memory set by srbk(), mm_malloc() & mm_realloc() should
 * not create new blocks if not necessary because srbk() is quite expensive.
 */
typedef struct mm_b {
  size_t size;
  int free;
  struct mm_b* next;
  struct mm_b* prev;
  void* curr;
} mm_block;

void *mm_malloc(size_t size);
void *mm_realloc(void *ptr, size_t size);
void mm_free(void *ptr);
