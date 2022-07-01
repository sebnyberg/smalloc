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

// Spacetree is a tree in 1d-form.
//
// Each node in the tree contains the maximum size of any one block in its
// subtree, including itself. Thus, spacetree[0] = __TOTAL_SIZE.
//
// On malloc, spacetree[0] signals whether the request can be completed or not.
// That is, if spacetree[0] is >= requested size, then there exists at least one
// block in the tree which is large enough to accomodate the request.
// The allocated block is set to size zero, and its parents are updated
// accordingly.
//
// On free, the lowest block with size zero that starts with the given address
// is the block to free. On free, if the node and its sibling (its buddy) have
// equal size, the parent is reset to its original size (that of both siblings).
//
static uint32_t *spacetree;

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
  return ((idx+1) / 2) - 1;
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

void
__alloc_reset_tree()
{
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
    spacetree[i] = size;
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
  spacetree = (uint32_t *)(mem + __TOTAL_SIZE);
  __alloc_reset_tree();
}


void*
smalloc(size_t size)
{
  if (size == 0) return NULL;

  __alloc_init();

  size = MAX(pow2_ceil(size), __MIN_SIZE);

  if (size > spacetree[0]) {
    errno = ENOMEM;
    return NULL;
  }

  // Find leftmost block that accomodates the request
  ssize_t idx = 0;
  size_t block_size = __TOTAL_SIZE;
  for (; block_size != size; block_size /= 2) {
    if (spacetree[left_child(idx)] >= size) {
      idx = left_child(idx);
    } else {
      idx = right_child(idx);
    }
  }

  spacetree[idx] = 0;

  // Each level in the tree is offset by its number of nodes - 1.
  // Thus, multiplying its block size with (idx+1) adds the total size + offset.
  size_t offset_bytes = block_size * (idx + 1) - __TOTAL_SIZE;
  void *addr = (void *) (mem + offset_bytes);

  // Update tree
  while (idx > 0) {
    idx = parent(idx);
    spacetree[idx] = MAX(spacetree[left_child(idx)], spacetree[right_child(idx)]);
  }

  return addr;
}

void*
srealloc(void *ptr, size_t size)
{
  if (ptr == NULL) {
    return smalloc(size);
  }
  if (size == 0) {
    free(ptr);
  }

  __alloc_init();

  // Out of bounds check
  if (ptr < (void *)mem || ptr >= (void *)(mem + __TOTAL_SIZE)) {
    errno = EINVAL;
    return NULL;
  }

  // Determine old size
  ssize_t idx = (size_t)((char *)ptr - mem);
  idx += (__NBLOCKS - 1);
  uint32_t old_size = __MIN_SIZE;
  while (idx > 0 && spacetree[idx] != 0) {
    idx = parent(idx);
    old_size *= 2;
  }

  if (spacetree[idx] != 0) {
    // Could not find block pointed to by ptr
    return NULL;
  }
  if (size <= old_size) {
    return ptr;
  }

  void *new_ptr = smalloc(size);
  if (new_ptr == NULL) {
    // errno already set by malloc
    return NULL;
  }

  memcpy(new_ptr, ptr, old_size);
  sfree(ptr);
  return new_ptr;
}

void
*scalloc(size_t nmemb, size_t size)
{
  if (nmemb == 0 || size == 0) {
    return NULL;
  }
  size_t rqsize = nmemb * size;
  if (rqsize / nmemb != size) {
    errno = EOVERFLOW;
    return NULL;
  }
  void* ptr = smalloc(rqsize);
  if (ptr == NULL) {
    return ptr;
  }
  memset(ptr, 0, rqsize);
  return ptr;
}

void
sfree(void *ptr)
{
  // Out of bounds check
  if (ptr < (void *)mem || ptr >= (void *)(mem + __TOTAL_SIZE)) {
    return;
  }
  // Note: I assume there is no way to send an address which is not on an
  // address boundary. Nobody in their right mind would attempt to hack such a
  // thing in C, would they?

  ssize_t idx = (size_t)((char *)ptr - mem);
  idx += (__NBLOCKS - 1);
  uint32_t size = __MIN_SIZE;
  while (idx > 0 && spacetree[idx] != 0) {
    idx = parent(idx);
    size *= 2;
  }
  spacetree[idx] = size;
  size_t l, r;
  while (idx > 0) {
    idx = parent(idx);

    l = spacetree[left_child(idx)];
    r = spacetree[right_child(idx)];
    if (l == r) {
      spacetree[idx] = size;
    } else {
      spacetree[idx] = MAX(l, r);
    }
    size *= 2;
  }

  __alloc_init();
}