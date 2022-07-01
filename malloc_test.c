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

static void test_calloc(void)
{
  // TEST_IGNORE();
  // malloc + free + malloc should retain the previous value
  char *ptr;
  TEST_ASSERT_NOT_NULL((ptr = smalloc(10)));
  strcpy(ptr, "test");
  sfree(ptr);
  char *ptr2;
  TEST_ASSERT_NOT_NULL((ptr2 = smalloc(10)));
  TEST_ASSERT_EQUAL_STRING("test", ptr2);
  TEST_ASSERT_EQUAL(ptr, ptr2);
  // malloc + free + calloc should reset the value
  sfree(ptr);
  char *ptr3;
  TEST_ASSERT_NOT_NULL((ptr3 = scalloc(10, 1)));
  TEST_ASSERT_EQUAL_STRING("", ptr3);
  TEST_ASSERT_EQUAL(ptr2, ptr3);
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

static void test_malloc_100_1MB(void)
{
  // TEST_IGNORE();
  for (size_t i = 0; i < 100000; i++) {
    void *ptr = smalloc(1024*1024);
    TEST_ASSERT_NOT_NULL(ptr);
    sfree(ptr);
  }
}

static void test_malloc_size_zero(void)
{
  // TEST_IGNORE();
  TEST_ASSERT_NULL(smalloc(0));
}


int main(void)
{
  UnityBegin("buddy.c");

  RUN_TEST(test_calloc);
  RUN_TEST(test_calloc_overflow);
  RUN_TEST(test_malloc_happy);
  RUN_TEST(test_malloc_100_1MB);
  RUN_TEST(test_malloc_size_zero);

  return UnityEnd();
}