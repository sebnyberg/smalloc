#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

#define DEBUG 0
#define debug_print(...) \
		do { if (DEBUG) fprintf(stderr, ##__VA_ARGS__); } while (0)

#define MAX(x, y) (x > y ? x : y)

#define __TOTAL_SIZE (1024*1024*32)
#define __MIN_SIZE (8)
#define __NBLOCKS (__TOTAL_SIZE/__MIN_SIZE)

#define __ALLOCMAP_SIZE ((__NBLOCKS)*2*sizeof(uint32_t))

static char *mem = NULL;
static uint32_t *spacetree = NULL;

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

static void
__alloc_reset_tree()
{
	uint32_t size = __TOTAL_SIZE*2;
	for (uint32_t i = 0; i < 2 * __NBLOCKS - 1; i++) {
		if (is_pow2(i+1)) {
			size /= 2;
		}
		spacetree[i] = size;
	}
}

static void
__alloc_init()
{
	if (mem != NULL) {
		return;
	}
	mem = (char *)sbrk(__TOTAL_SIZE);
	if (mem == (void*)-1)  {
		errno = ENOMEM;
		return;
	}
	spacetree = sbrk(__ALLOCMAP_SIZE);
	debug_print("init data_addr: %p space_addr: %p\n", mem, (void*)spacetree);
	__alloc_reset_tree();
}


void*
malloc(size_t size)
{
	if (mem == NULL) {
		__alloc_init();
	}

	if (size == 0) {
		return NULL;
	}

	size = MAX(pow2_ceil(size), __MIN_SIZE);

	if (size > spacetree[0]) {
		debug_print("size %ld larger than space in tree %d\n", size, spacetree[0]);
		// errno = ENOMEM;
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

	size_t offset_bytes = block_size * (idx + 1) - __TOTAL_SIZE;
	void *addr = (void *) (mem + offset_bytes);

	// Update tree
	for (ssize_t i = idx; i > 0;) {
		i = parent(i);
		spacetree[i] = MAX(spacetree[left_child(i)], spacetree[right_child(i)]);
	}

	debug_print("malloc ptr:%p, size:%ld block_size:%ld idx:%ld offset_bytes:%ld\n", addr, size, block_size, idx, offset_bytes);
	return addr;
}

void
free(void *ptr)
{
	if (ptr == NULL) {
		return;
	}

	ssize_t offset_bytes = (ssize_t)((char *)ptr - mem);
	ssize_t idx = offset_bytes / (__MIN_SIZE);
	idx += (__NBLOCKS - 1);
	size_t size = __MIN_SIZE;
	while (idx > 0 && spacetree[idx] != 0) {
		idx = parent(idx);
		size *= 2;
	}
	spacetree[idx] = size;
	size_t l, r;
	while (idx > 0) {
		size *= 2;
		idx = parent(idx);

		l = spacetree[left_child(idx)];
		r = spacetree[right_child(idx)];
		if (l + r == size) {
			spacetree[idx] = size;
		} else {
			spacetree[idx] = MAX(l, r);
		}
	}
	debug_print("free ptr:%p largest_block:%d\n", ptr, spacetree[0]);
}

void
*realloc(void *ptr, size_t size)
{
	debug_print("realloc ptr:%p to_size:%ld\n", ptr, size);
	if (ptr == NULL) {
		return malloc(size);
	}
	if (size == 0) {
		free(ptr);
		return NULL;
	}

	// Determine old size
	ssize_t offset_bytes = (size_t)((char *)ptr - mem);
	ssize_t idx = offset_bytes / (__MIN_SIZE);
	idx += (__NBLOCKS - 1);
	size_t old_size = __MIN_SIZE;
	while (idx > 0 && spacetree[idx] != 0) {
		idx = parent(idx);
		old_size *= 2;
	}
	if (spacetree[idx] != 0) {
		// Could not find block pointed to by ptr
		debug_print("could not find block pointed to by ptr %p\n", ptr);
		return NULL;
	}
	if (size <= old_size) {
		debug_print("no alloc needed for realloc\n");
		return ptr;
	}

	void *new_ptr = malloc(size);
	debug_print("realloc addr:%p new_ptr:%p old_size:%ld new_size:%ld\n", ptr, new_ptr, old_size, size);


	if (new_ptr == NULL) {
		// errno already set by malloc
		return ptr;
	}

	memcpy(new_ptr, ptr, old_size);
	free(ptr);
	return new_ptr;
}

void
*calloc(size_t nmemb, size_t size)
{
	size_t rqsize = nmemb * size;
	debug_print("calloc size:%ld\n", rqsize);
	void* ptr = malloc(rqsize);
	if (ptr == NULL) {
		return NULL;
	}
	memset(ptr, 0, rqsize);
	return ptr;
}
