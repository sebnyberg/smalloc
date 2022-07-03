#include "unity/unity.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void setUp(void)
{
}

void tearDown(void)
{
}

static void test_calloc_many(void)
{
  // TEST_IGNORE();
  char *ptr[1000];
  for (size_t i = 0; i < 1e3; i++) {
    ptr[i] = calloc(256, 1);
    TEST_ASSERT_NOT_NULL(ptr[i]);
    TEST_ASSERT_EQUAL_STRING("", ptr[i]);
    memset(ptr, 1, 256);
  }
}

static void test_malloc_happy(void)
{
  // TEST_IGNORE();
  char *greeting = malloc(10);
  TEST_ASSERT_NOT_NULL(greeting);
  memset(greeting, 0, 10);
  strcpy(greeting, "hello");
  TEST_ASSERT_EQUAL_STRING(greeting, "hello");
  char *greeting2 = malloc(50);
  memset(greeting2, 0, 50);
  strcpy(greeting2, "hi");
  TEST_ASSERT_EQUAL_STRING(greeting2, "hi");
  free(greeting);
  free(greeting2);
}

static void test_malloc_many(void)
{
  // TEST_IGNORE();
  char *ptr[2];
  for (size_t i = 0; i < 1e4; i++) {
    ptr[0] = malloc(1024*64);
    TEST_ASSERT_NOT_NULL(ptr[0]);
    strcpy(ptr[0], "hello");
    ptr[1] = malloc(1024*64);
    TEST_ASSERT_NOT_NULL(ptr[1]);
    strcpy(ptr[1], "hi");
    TEST_ASSERT_EQUAL_STRING("hello", ptr[0]);
    TEST_ASSERT_EQUAL_STRING("hi", ptr[1]);
    free(ptr[0]);
    free(ptr[1]);
  }
}

static void test_malloc_size_zero(void)
{
  // TEST_IGNORE();
  TEST_ASSERT_NULL(malloc(0));
}

static void test_realloc_zero_size_free(void)
{
  // TEST_IGNORE();
  char *ptr;
  for (size_t i = 0; i < 1e4; i++) {
    ptr = realloc(NULL, 1024);
    TEST_ASSERT_NOT_NULL(ptr);
    ptr = realloc(ptr, 1024);
    TEST_ASSERT_NOT_NULL(ptr);
    TEST_ASSERT_NULL(realloc(ptr, 0));
  }
}

static void test_realloc_large(void)
{
  // TEST_IGNORE();
  char *ptr;
  ptr = realloc(NULL, 1024*256);
  TEST_ASSERT_NOT_NULL(ptr);
  ptr = realloc(NULL, 1024*256);
  TEST_ASSERT_NOT_NULL(ptr);
  ptr = realloc(ptr, 1024*512);
  TEST_ASSERT_NOT_NULL(ptr);
}

static void test_things(void)
{
  char *ptr[100];
  ptr[0] = malloc(8);
  free(ptr[0]);
}


int main(void)
{
  UnityBegin("buddy.c");

  RUN_TEST(test_calloc_many);
  // RUN_TEST(test_calloc_overflow);
  RUN_TEST(test_malloc_happy);
  RUN_TEST(test_malloc_many);
  RUN_TEST(test_malloc_size_zero);
  RUN_TEST(test_realloc_large);
  RUN_TEST(test_realloc_zero_size_free);
  RUN_TEST(test_things);

  return UnityEnd();
}