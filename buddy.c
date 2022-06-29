#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "malloc.h"

// Total and min size must be a power of two.
#define __TOTAL_SIZE (1048576)
#define __MIN_SIZE (32)

// Allocmap could be a bitmap, but it would make things more complicated.
#define __ALLOCMAP_SIZE (__TOTAL_SIZE/__MIN_SIZE)

static char *mem;
static char *alloc;

void
__alloc_reset()
{
  __alloc_init();
  memset(alloc, 0, __ALLOCMAP_SIZE);
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
  alloc = mem + __TOTAL_SIZE;
  memset(alloc, 0, __ALLOCMAP_SIZE);
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
malloc(size_t size)
{
  (void) size;
  __alloc_init();
  size = __pow2_ceil(size);

  return NULL;
}

void*
realloc(void *ptr, size_t size)
{
  (void) ptr;
  (void) size;
  __alloc_init();
  size = __pow2_ceil(size);

  return NULL;
}

void
*calloc(size_t nmemb, size_t size)
{
  (void) nmemb;
  (void) size;
  __alloc_init();
  size = __pow2_ceil(size);

  return NULL;
}

void
free(void *ptr)
{
  (void) ptr;
  __alloc_init();
}