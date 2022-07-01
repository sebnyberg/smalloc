#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

#include "malloc.h"

#define DEBUG 1
#define MAX(x, y) (x > y ? x : y)
#define debug_print(...) \
		do { if (DEBUG) fprintf(stderr, ##__VA_ARGS__); } while (0)

// Total and min size must be a power of two
// Important! If the total number of blocks exceed int32_t, then the allocmap
// type must be changed. Also, the ratio of __MIN_SIZE to allocmap type
// determines the amount of memory "waste" per alloced block. Larger and fewer
// blocks result in much lower waste.
#define __TOTAL_SIZE (1024*1024)
#define __MIN_SIZE (64)
#define __NBLOCKS (__TOTAL_SIZE/__MIN_SIZE)

// Allocmap could be a bitmap, but it would make things more complicated.
#define __ALLOCMAP_SIZE ((__NBLOCKS)*2*sizeof(uint32_t))

static char *mem;

// Memfree is a tree in 1d-form.
//
// For details on how memfree is updated as alloc and free happens, see each
// respective function.
static uint32_t *memfree;

static inline size_t
left_child(size_t idx)
{
  return idx*2 + 1;
}

static inline size_t
right_child(size_t idx)
{
  return idx*2 + 2;
}

static inline size_t
parent(size_t idx)
{
  return ((idx+1) >> 1) - 1;
}

static inline size_t
is_pow2(size_t idx)
{
  // https://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
  return (idx & (idx - 1)) == 0;
}

static size_t
pow2_ceil(size_t x)
{
  // https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
  x--;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x++;
  return x;
}

// static inline size_t
// tree_level(size_t idx)
// {
//   size_t level = 1;
//   while (idx > 0) {
//     level++;
//     idx = (idx-1) >> 1;
//   }
//   return level;
// }

void
__alloc_reset()
{
  __alloc_init();
  /* Create the tree
            __TOTAL_SIZE
            /           \
    __TOTAL_SIZE/2   __TOTAL_SIZE/2
    /        \          /         \
  ...       ...       ...         ...
  */
  size_t size = __TOTAL_SIZE*2;
  for (size_t i = 0; i < 2 * __NBLOCKS - 1; i++) {
    if (is_pow2(i+1)) {
      size /= 2;
    }
    memfree[i] = size;
  }
}

void
__alloc_init()
{
  if (mem != NULL) {
    return;
  }
  mem = sbrk(__TOTAL_SIZE + __ALLOCMAP_SIZE);
  if (mem == NULL) {
    perror("memory init err");
    exit(errno);
  }
  memfree = (uint32_t *)(mem + __TOTAL_SIZE);
  __alloc_reset();
}


void*
smalloc(size_t size)
{
  if (size == 0) return NULL;

  __alloc_init();

  size = MAX(pow2_ceil(size), __MIN_SIZE);

  if (size > memfree[0]) {
    errno = ENOMEM;
    return NULL;
  }

  // Find block (if any) that accomodates the request
  ssize_t idx = 0;
  size_t block_size = __TOTAL_SIZE;

  for (; block_size != size; block_size /= 2) {
    if (memfree[left_child(idx)] >= size) {
      idx = left_child(idx);
    } else {
      idx = right_child(idx);
    }
  }

  memfree[idx] = 0;
  size_t offset_bytes = block_size*(idx + 1) - __TOTAL_SIZE;
  void *addr = (void *) (mem + offset_bytes);

  return addr;
}

void*
srealloc(void *ptr, size_t size)
{
  (void) ptr;
  (void) size;
  __alloc_init();
  size = pow2_ceil(size);

  return NULL;
}

void
*scalloc(size_t nmemb, size_t size)
{
  (void) nmemb;
  (void) size;
  __alloc_init();
  size = pow2_ceil(size);

  return NULL;
}

void
sfree(void *ptr)
{
  (void) ptr;

  __alloc_init();
}