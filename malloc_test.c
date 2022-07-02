#include "unity/unity.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "malloc.h"

void setUp(void)
{
  __alloc_init();
}

void tearDown(void)
{
  __alloc_reset_tree();
}

static void test_calloc_many(void)
{
  // TEST_IGNORE();
  char *ptr;
  for (size_t i = 0; i < 1e4; i++) {
    ptr = scalloc(1024, 64);
    TEST_ASSERT_NOT_NULL(ptr);
    TEST_ASSERT_EQUAL_STRING("", ptr);
    memset(ptr, 1, 1024*64);
    sfree(ptr);
  }
}

static void test_calloc_overflow(void)
{
  void *ptr = scalloc(1<<31, 1<<31);
  TEST_ASSERT_NULL(ptr);
  TEST_ASSERT_EQUAL(errno, EOVERFLOW);
}

static void test_malloc_happy(void)
{
  // TEST_IGNORE();
  char *greeting = smalloc(10);
  TEST_ASSERT_NOT_NULL(greeting);
  memset(greeting, 0, 10);
  strcpy(greeting, "hello");
  TEST_ASSERT_EQUAL_STRING(greeting, "hello");
  char *greeting2 = smalloc(50);
  memset(greeting2, 0, 50);
  strcpy(greeting2, "hi");
  TEST_ASSERT_EQUAL_STRING(greeting2, "hi");
  sfree(greeting);
  sfree(greeting2);
}

static void test_malloc_many(void)
{
  // TEST_IGNORE();
  char *ptr[2];
  for (size_t i = 0; i < 1e4; i++) {
    ptr[0] = smalloc(1024*64);
    TEST_ASSERT_NOT_NULL(ptr[0]);
    strcpy(ptr[0], "hello");
    ptr[1] = smalloc(1024*64);
    TEST_ASSERT_NOT_NULL(ptr[1]);
    strcpy(ptr[1], "hi");
    TEST_ASSERT_EQUAL_STRING("hello", ptr[0]);
    TEST_ASSERT_EQUAL_STRING("hi", ptr[1]);
    sfree(ptr[0]);
    sfree(ptr[1]);
  }
}

static void test_malloc_size_zero(void)
{
  // TEST_IGNORE();
  TEST_ASSERT_NULL(smalloc(0));
}

static void test_realloc_zero_size_free(void)
{
  // TEST_IGNORE();
  char *ptr;
  for (size_t i = 0; i < 1e4; i++) {
    ptr = srealloc(NULL, 1024*64);
    TEST_ASSERT_NOT_NULL(ptr);
    ptr = srealloc(ptr, 1024*256);
    TEST_ASSERT_NOT_NULL(ptr);
    srealloc(ptr, 0);
  }
}


int main(void)
{
  UnityBegin("buddy.c");

  RUN_TEST(test_calloc_many);
  RUN_TEST(test_calloc_overflow);
  RUN_TEST(test_malloc_happy);
  RUN_TEST(test_malloc_many);
  RUN_TEST(test_malloc_size_zero);
  RUN_TEST(test_realloc_zero_size_free);

  return UnityEnd();
}