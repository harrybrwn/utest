#ifndef _UTEST_H
#define _UTEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

struct utest_runner;

typedef void (*TestMethod)(struct utest_runner*);
typedef int (*AssertionMsgFunc)(const char*, ...);

/**
 * Holds all metadata for a single test
 */
typedef struct utest_case
{
    void (*setup)(void);
    void (*teardown)(void);
    int ignore;
    int capture_output;

    TestMethod test;
    char* name;
    int status;
    char* output;
} UTestCase;

typedef struct utest_runner
{
    UTestCase* test;
    AssertionMsgFunc fail;
    AssertionMsgFunc warning;
} UTestRunner;

/* Internal global variable, DO NOT TOUCH */
extern UTestCase *_current_test;

/**
 * A timer that keeps track of the time it was started and ended.
 */
typedef struct utest_timer {
    struct timeval start, end;
} ut_timer_t;

/**
 * Start the timer
 */
void ut_timer_start(struct utest_timer* timer);

/**
 * End the timer
 */
void ut_timer_end(struct utest_timer* timer);

/**
 * Get the number of seconds that the timer was running
 */
double ut_timer_sec(struct utest_timer timer);

/**
 * 8 bit unsigned integer.
 */
typedef unsigned int byte_t __attribute__((__mode__(QI)));

/**
 * Run all the tests. This should be used in the main function if the AUTOTEST
 * macro is not defined.
 */
int RunTests(void);

/**
 * Search for string `str` in an array `arr` having length `len`.
 *
 * Returns 1 if `str` is found and 0 otherwise.
 */
int str_arr_contains(char** arr, size_t len, char* str);

/**
 * Create 'n_strings' random strings all having a length of 'str_length'.
 *
 * The resulting array of char pointers must be freed when done with.
 */
char** random_strings(int n_strings, int str_length);

// internal
void utest_build_testcase(UTestCase, TestMethod, char *);
int assertion_failure(const char* fmt, ...);
int utest_warning(const char* fmt, ...);

/**
 * Capture output from stdout
 *
 * Example:
 *   char* buf = NULL;
 *   size_t len;
 *   while (utest_capture_output(&buf, &len)) {
 *       printf("all output in this block will be stored in the buffer");
 *   }
 *
 * This function relies on static variables and should only be run inside
 * a single thread at any given time.
 */
int utest_capture_output(char **buf, size_t*);

/**
 * Compare two byte buffers of length `len`
 *
 * Returns 1 for a match, 0 for no match.
 */
int binary_compare(byte_t* left, byte_t* right, size_t len);

// internal utilities
/**
 * Test to see that two arrays of character pointers are identicle.
 *
 * Returns 1 if all elements in `arr1` equal all elements in `arr2` otherwise
 * it returns 0.
 */
int arr_eq_s(char** arr1, char** arr2, size_t len);

/* internal */
int strcomp(char* one, char* two);

#if defined(AUTOTEST) && !defined(_MAIN_DEFINED) && !defined(_UTEST_IMPL)
#define _MAIN_DEFINED
int main(void) {
    return RunTests();
}
#endif /* AUTOTEST && !_MAIN_DEFINED && !_UTEST_IMPL */

#define _UTEST_PRINT_FMT(A)\
    (_Generic((A),         \
        char*: "%s",       \
        const char*: "%s", \
        int: "%d"))

#define _ARR_EQ_DECL(SUFFIX, TYPE)                   \
int arr_unordered_eq_##SUFFIX(TYPE*, TYPE*, size_t); \
int arr_eq_##SUFFIX(TYPE*, TYPE*, size_t);

_ARR_EQ_DECL(s, char*)

#undef _ARR_EQ_DECL

#define _EQ_EXPR(A, B)                                                           \
(_Generic((A),                                                                   \
    char*:       strcomp(((char*)(uintptr_t)(A)), ((char*)(uintptr_t)(B))) == 0, \
    const char*: strcomp(((char*)(uintptr_t)(A)), ((char*)(uintptr_t)(B))) == 0, \
    default: A == B)                                                             \
)

/**
 * Causes the current test to fail giving `EXP` as an error message
 */
#define FAIL(EXP)                                                       \
    (_current_test->status += assertion_failure("TEST(%s) %s:%d '%s'\n",\
                _current_test->name, __FILE__, __LINE__, EXP))

/**
 * Same as `FAIL` but with a printf format string
 */
#define FAILF(FMT, ...) \
    (_current_test->status += assertion_failure("TEST(%s) %s:%d " FMT, \
                _current_test->name, __FILE__, __LINE__, __VA_ARGS__))

#define _ASSERT_FAIL(LEFT, OP, RIGHT) \
    (_current_test->status += assertion_failure("TEST(%s) %s:%d '%s'\n",\
                _current_test->name, __FILE__, __LINE__, #LEFT OP #RIGHT))

#ifndef assert
#undef assert
/**
 * Assert that `exp` is true and fail the current test otherwise
 */
#define assert(exp) (exp) ? ((void)0) : FAIL(#exp)
#endif

/**
 * Fail the current test if A is not equal to B
 */
#define assert_eq(A, B)               \
    ({__typeof__(A) _A = A;           \
    __typeof__(B) _B = B;             \
    (_EQ_EXPR(_A, _B)) ?              \
        ((void)0) :                   \
        _ASSERT_FAIL(A, " == ", B);})

/**
 * Fail the current test if A is equal to B
 */
#define assert_not_eq(A, B)  \
    ({__typeof__(A) _A = A;  \
    __typeof__(B) _B   = B;  \
    (!_EQ_EXPR(_A, _B)) ?    \
        ((void)0) :          \
        _ASSERT_FAIL(A, " != ", B);})

/**
 * Assert that the memory stored at two address for length `LEN` are equal
 */
#define assert_eqn(A, B, LEN)     \
    (binary_compare((byte_t*)(uintptr_t)A, (byte_t*)(uintptr_t)B, LEN)) ? \
        ((void)0) :               \
        _ASSERT_FAIL(A, " == ", B)

/**
 * Assert that the memory stored at two address for length `LEN` are not equal
 */
#define assert_not_eqn(A, B, LEN)                                          \
    (!binary_compare((byte_t*)(uintptr_t)A, (byte_t*)(uintptr_t)B, LEN)) ? \
        ((void)0) :                                                        \
        _ASSERT_FAIL(A, " != ", B)

#define eq(A, B)         assert_eq(A, B)
#define not_eq(A, B)     assert_not_eq(A, B)
#define eqn(A, B, L)     assert_eqn(A, B, L)
#define not_eqn(A, B, L) assert_not_eqn(A, B, L)

#define TEST_NAME(NAME) _utest_test_##NAME
#define _TEST_DECL(NAME) void TEST_NAME(NAME)(UTestRunner* utest __attribute__((unused)))

/**
 * The TEST macro is what creates a test.
 *
 * NAME: is the name of the test, has no type just type it in plain text
 * Other Options:
 *   .ignore: if this is not zero, then the test will not be run
 *   .setup: a function pointer that runs before the test
 *   .teardown: a function pointer that runs after the test is complete
 *
 * Example:
 *  TEST(my_test) {
 *      assert(1 == 1);
 *  }
 *
 *  TEST(ignored_test, .ignore = 1) {
 *      assert(false);
 *  }
 */
#define TEST(NAME, ...)                                    \
    _TEST_DECL(NAME);                                      \
    __attribute__((constructor))                           \
    void _add_##NAME##_to_tests(void) {                    \
        UTestCase opt = { __VA_ARGS__ };                   \
        utest_build_testcase(opt, TEST_NAME(NAME), #NAME); \
    }                                                      \
    _TEST_DECL(NAME)

#define UTEST_OPT_IGNORE .ignore = 1

/**
 * Capture the output of a block of code.
 *
 * Example:
 *   char* buf = NULL;
 *   CATCH_OUTPUT(buf) {
 *       printf("hello there\n");
 *   }
 *   printf("%s", buf); // hello there\n
 */
#define CATCH_OUTPUT(BUFFER)             \
    char *BUFFER = NULL;                 \
    size_t BUFFER##_length;              \
    _current_test->capture_output = 1;   \
    if (_current_test->output != NULL) { \
        free(_current_test->output);     \
        _current_test->output = NULL;    \
    }                                    \
    while (utest_capture_output(&BUFFER, &BUFFER##_length))

#define CURRENT_TEST_NAME (_current_test->name)

#ifdef __cplusplus
}
#endif

#endif /* _UTEST_H */