#ifndef TEST_FRAMEWORK
#define TEST_FRAMEWORK

#include <stdio.h>
#include <stdlib.h>

#define GREEN  "\033[0;32m"
#define RED    "\033[0;31m"
#define YELLOW "\033[0;33m"
#define RESET  "\033[0m"

extern int tests_run;
extern int tests_passed;

#define TEST_ASSERT(condition, message) \
  do { \
    tests_run++; \
    if (condition) { \
      tests_passed++; \
      printf(GREEN "✓ %s\n" RESET, message); \
    } else { \
      printf(RED "✗ %s\n" RESET, message); \
    } \
  } while(0)

#define TEST_SUITE_INIT() \
  int tests_run = 0; \
  int tests_passed = 0;

#define TEST_SUITE_SUMMARY(suite_name) \
  do { \
    printf("\n==============================\n"); \
    printf("Test Results for %s: %d/%d tests passed\n", suite_name, tests_passed, tests_run); \
    if (tests_passed == tests_run) { \
      printf(GREEN "🎉 All tests passed!\n" RESET); \
    } else { \
      printf(RED "❌ Some tests failed!\n" RESET); \
    } \
  } while(0)\

#define TEST_MAIN(suite_name, test_functions...) \
  int main() {\
    printf("Running %s Tests...\n", suite_name); \
    printf("\n==============================\n"); \
    test_functions; \
    TEST_SUITE_SUMMARY(suite_name); \
    return (tests_passed == tests_run) ? 0 : 1; \
  }

#endif

