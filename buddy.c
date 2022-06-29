#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "malloc.h"

#define DEBUG 1
#define debug_print(...) \
		do { if (DEBUG) fprintf(stderr, ##__VA_ARGS__); } while (0)

// Total and min size must be a power of two
#define __TOTAL_SIZE (1024*1024)
#define __MIN_SIZE (32)

// Allocmap could be a bitmap, but it would make things more complicated.
#define __ALLOCMAP_SIZE ((__TOTAL_SIZE/__MIN_SIZE)*2)

static char *mem;
static char *memfree;

void
__alloc_reset()
{
  __alloc_init();
  memset(memfree, 0, __ALLOCMAP_SIZE);
  memfree[0] = 1;
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
  memfree = mem + __TOTAL_SIZE;
  memset(memfree, 0, __ALLOCMAP_SIZE);
  memfree[0] = 1;
}

size_t
__tree_level(size_t idx)
{
  size_t level = 1;
  while (idx > 0) {
    level++;
    idx = (idx-1) >> 1;
  }
  return level;
}

static inline size_t
left_child(size_t idx)
{
  return (idx >> 1) + 1;
}

static inline size_t
right_child(size_t idx)
{
  return (idx >> 1) + 2;
}

static inline size_t
parent(size_t idx)
{
  return ((idx+1) >> 1) - 1;
}

// Taken from
// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
static
size_t
__pow2_ceil(size_t x)
{
  x--;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x++;
  return x;
}

void*
smalloc(size_t size)
{
  if (size == 0) return NULL;

  __alloc_init();

  size = __pow2_ceil(size);
  if (size < __MIN_SIZE) {
    size = __MIN_SIZE;
  }
  if (size >= __TOTAL_SIZE)  {
    errno = ENOMEM;
    return NULL;
  }
  debug_print("finding block of size: %ld\n", size);

  // Try each size from small to large until a free block is found
  ssize_t block_idx = -1;
  ssize_t block_size = -1;
  for (ssize_t k = size; block_idx == -1 && k <= __TOTAL_SIZE; k <<= 1) {
    size_t nelem = __TOTAL_SIZE / k;
    for (size_t i = nelem - 1; i < nelem-1+nelem; i++) {
      if (memfree[i] == 1) {
        block_idx = i;
        block_size = k;
        break;
      }
    }
  }
  if (block_idx == -1) {
    errno = ENOMEM;
    return NULL;
  }
  debug_print("found block of size %ld at index %ld\n", block_size, block_idx);

  size_t nelem = (__TOTAL_SIZE / block_size);
  block_idx -= (nelem - 1);
  void *addr = (void *) (mem + block_idx);
  debug_print("return addr: %p\n", addr);

  // Mark block as unavailable
  memfree[block_idx] = 0;

  /* Mark leftover blocks as available

  Consider this tree with __TOTAL_SIZE = 4:

      (4)            1
                   /   \
      (2)        0       0
               /  \    /   \
      (1)     0     0  0     0

  The buddy algorithm finds the first free (marked as 1) address such that its
  size is larger than or equal to the requested (adjusted) size.

  The block is marked as occupied (0). Then the remainder is used to mark new
  free blocks until the remainder is zero.

  For example, allocating a 1-byte segment occupies the first address, then
  marks a 2-byte and 1-byte segment as free:

      (4)            0
                   /    \
      (2)        0        1
               /  \     /   \
      (1)     0     1  0     0

  Mallocing a 3-byte segment gives:

      (4)            0
                   /    \
      (2)        0        0
               /  \     /   \
      (1)     0     0  0     1
  */

 ssize_t remainder = block_size - size;
 block_size >>= 1;
 while (remainder > 0) {
  size_t r = right_child(block_idx);
  if (remainder >= block_size) {
    // Mark right child as free and go left
    memfree[r] = 1;
    remainder -= block_size;
    block_idx = r-1;
  } else {
    // Go right
    block_idx = r;
  }
  block_size >>= 1;
 }

  return addr;
}

void*
srealloc(void *ptr, size_t size)
{
  (void) ptr;
  (void) size;
  __alloc_init();
  size = __pow2_ceil(size);

  return NULL;
}

void
*scalloc(size_t nmemb, size_t size)
{
  (void) nmemb;
  (void) size;
  __alloc_init();
  size = __pow2_ceil(size);

  return NULL;
}

void
sfree(void *ptr)
{
  (void) ptr;

  /*
  Consider this tree with __TOTAL_SIZE = 8:


                          0                 = 8
                      /      \
                   1            0           = 4
                /    \        /    \
              0       0      1       0      = 2
            /  \    /  \    /  \    /  \
           0    0  0    0  0    0  1    0   = 1


                          0                 = 8
                      /      \
                   0            0           = 4
                /    \        /    \
              0       0      1       0      = 2
            /  \    /  \    /  \    /  \
           0    0  0    1  0    0  1    0   = 1

  Let's say we receive the address 0.

  So we mark the left child as available, then go to the right node:

                     0
                   /   \
          free > 1       0
               /  \    /   \
              0     0  0     0

  There's still 1 size left over, so we mark the left child as available:

                  0
                /   \
              1       0
            /  \    /   \
          0     0  1     0
                   ^     ^ occupied block
                 free

  */

  __alloc_init();
}