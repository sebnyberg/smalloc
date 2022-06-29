#include <stdlib.h>

// __tree_level finds the level of a tree given an index in the 1d array.
size_t __tree_level(size_t idx);

// __alloc_reset ensures that memory is initialized and free.
void __alloc_reset();

// __alloc_init ensures that memory is initialized.
void __alloc_init();

void* smalloc(size_t size);

void* srealloc(void *ptr, size_t size);

void *scalloc(size_t nmemb, size_t size);

void sfree(void *ptr);