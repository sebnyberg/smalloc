#include <stdlib.h>

// __alloc_reset resets the space tree.
void __alloc_reset_tree();

// __alloc_init ensures that memory is initialized.
void __alloc_init();

void* smalloc(size_t size);

void* srealloc(void *ptr, size_t size);

void *scalloc(size_t nmemb, size_t size);

void sfree(void *ptr);