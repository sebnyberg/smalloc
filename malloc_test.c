#include "unity/unity.h"

#include <stdio.h>
#include "malloc.h"

void setUp(void)
{
}

void tearDown(void)
{
  __alloc_reset();
}

static void test_nothing(void)
{
  TEST_ASSERT_EQUAL(0, 0);
}

int main(void)
{
  UnityBegin("malloc");

  RUN_TEST(test_nothing);

  return UnityEnd();
}