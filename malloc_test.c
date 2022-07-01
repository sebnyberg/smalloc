#include "unity/unity.h"

#include <stdio.h>
#include <string.h>
#include "malloc.h"

void setUp(void)
{
}

void tearDown(void)
{
  // __alloc_reset();
}

static void test_happy(void)
{
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
}

// static void test_malloc_100_1MB(void)
// {
//   void *ptr = malloc(0);
//   for (size_t i = 0; i < 100; i++) {

//   }
//   TEST_ASSERT_NULL(ptr);
// }

// static void test_malloc_zero(void)
// {
//   // void *ptr = malloc(0);
//   // TEST_ASSERT_NULL(ptr);
// }

int main(void)
{
  UnityBegin("buddy.c");

  RUN_TEST(test_happy);
  // RUN_TEST(test_malloc_zero);

  return UnityEnd();
}