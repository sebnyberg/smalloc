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

/*
Linked list allocator.
*/

typedef struct list_head_t {
	struct list_head_t *next;
	size_t size;
	size_t isfree;
} list_head_t;

// Head is a zero-size sentinel dummy that starts the linked list chain.
static list_head_t head;

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

static list_head_t*
alloc_block(size_t size)
{
	void *ptr = sbrk(sizeof(list_head_t)+size);
	if (ptr == (void *)-1) {
		return NULL;
	}

	list_head_t *l = (list_head_t*)ptr;
	memset(l, 0, sizeof(list_head_t));
	l->size = size;
	return l;
}

static list_head_t*
find_block(size_t size)
{
	list_head_t *prev = &head;
	list_head_t *curr = prev->next;
	size = MAX(8, pow2_ceil(size));
	while (curr != NULL && (curr->isfree == 0 || curr->size < size)) {
		prev = curr;
		curr = curr->next;
	}
	if (curr != NULL) {
		curr->isfree = 0;
		return curr;
	}
	prev->next = alloc_block(size);
	return prev->next;
}


void*
malloc(size_t size)
{
	if (size == 0) {
		return NULL;
	}
	list_head_t *block = find_block(size);
	return block+1;
}


void
free(void *ptr)
{
	if (ptr == NULL) {
		return;
	}
	list_head_t *l = (list_head_t*)(ptr) - 1;
	l->isfree = 1;
	return;
}

void
*realloc(void *ptr, size_t size)
{
	if (ptr == NULL) {
		return malloc(size);
	}
	if (size == 0) {
		free(ptr);
		return NULL;
	}

	list_head_t *l = (list_head_t*)(ptr) - 1;
	if (size <= l->size) {
		return ptr;
	}

	void *new_ptr = malloc(size);
	if (new_ptr == NULL) {
		return ptr;
	}

	memcpy(new_ptr, ptr, l->size);
	free(ptr);
	return new_ptr;
}

void
*calloc(size_t nmemb, size_t size)
{
	size_t rqsize = nmemb * size;
	void* ptr = malloc(rqsize);
	if (ptr == NULL) {
		return NULL;
	}
	memset(ptr, 0, rqsize);
	return ptr;
}
