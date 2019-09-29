#ifndef _UTEST_H
#define _UTEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

struct utest_runner;

typedef void (*TestMethod)(struct utest_runner*);
typedef int (*AssertionMsgFunc)(const char*, ...);

typedef struct utest_suite
{
    void (*setup)(void);
    void (*teardown)(void);
    int ignore;
    int capture_output;

    TestMethod test;
    char* name;
    int status;
    char* output;
} UTestSuite;

typedef struct utest_runner
{
    UTestSuite* current_test;
    AssertionMsgFunc fail;
    AssertionMsgFunc warning;
} UTestRunner;

extern int n_Tests;
extern UTestSuite *_current_test;
extern UTestSuite **AllTests;

// 8 bit unsigned integer.
typedef unsigned int byte_t __attribute__((__mode__(QI)));

int RunTests(void);

int assertion_failure(const char* fmt, ...);
int utest_warning(const char* fmt, ...);

int utest_capture_output(char **buf);

/**
 * Return: 1 for a match, 0 for no match.
 */
int binary_compare(byte_t* left, byte_t* right, size_t len);

int arr_eq_s(char** arr1, char** arr2, size_t len);
int strcomp(char* one, char* two);

#if defined(AUTOTEST) && !defined(_MAIN_DEFINED) && !defined(_UTEST_IMPL)
#define _MAIN_DEFINED
int main(void) {
    return RunTests();
}
#endif /* AUTOTEST && _MAIN_DEFINED && _UTEST_IMPL */

#define _ARR_EQ_DECL(SUFFIX, TYPE)                   \
int arr_unordered_eq_##SUFFIX(TYPE*, TYPE*, size_t); \
int arr_eq_##SUFFIX(TYPE*, TYPE*, size_t);

_ARR_EQ_DECL(i, int)
_ARR_EQ_DECL(ui, unsigned int)
_ARR_EQ_DECL(l, long)
_ARR_EQ_DECL(ul, unsigned long)
_ARR_EQ_DECL(f, float)
_ARR_EQ_DECL(d, double)
#undef _ARR_EQ_DECL

#define _EQ_EXPR(A, B)                                                          \
(_Generic((A),                                                                  \
    char*:       strcomp(((char*)(uintptr_t)(A)), ((char*)(uintptr_t)(B))) == 0, \
    const char*: strcomp(((char*)(uintptr_t)(A)), ((char*)(uintptr_t)(B))) == 0, \
    default: A == B)                                                            \
)

#define _ARRAY_EQ_EXPR(A, B, LEN)                                             \
(_Generic((A),                                                                \
    void*:              memcmp((void*)A,             (void*)B, LEN) == 0,     \
    char*:              strncmp((char*)(A),          (char*)(B), LEN) == 0,   \
    char[LEN]:          strncmp((char*)A,            (char*)B, LEN) == 0,     \
    const char*:        strncmp((char*)(A),          (char*)(B), LEN) == 0,   \
    char**:             arr_eq_s((char**)A,          (char**)B, LEN),         \
    char*[LEN]:         arr_eq_s((char**)A,          (char**)B, LEN),         \
    int*:               arr_eq_i((int*)A,            (int*)B, LEN),           \
    int[LEN]:           arr_eq_i((int*)A,            (int*)B, LEN),           \
    unsigned int*:      arr_eq_ui((unsigned int*)A,  (unsigned int*)B, LEN),  \
    unsigned int[LEN]:  arr_eq_ui((unsigned int*)A,  (unsigned int*)B, LEN),  \
    long*:              arr_eq_l((long*)A,           (long*)B, LEN),          \
    long[LEN]:          arr_eq_l((long*)A,           (long*)B, LEN),          \
    unsigned long*:     arr_eq_ul((unsigned long*)A, (unsigned long*)B, LEN), \
    unsigned long[LEN]: arr_eq_ul((unsigned long*)A, (unsigned long*)B, LEN), \
    float*:             arr_eq_f((float*)A,          (float*)B, LEN),         \
    float[LEN]:         arr_eq_f((float*)A,          (float*)B, LEN),         \
    double*:            arr_eq_d((double*)A,         (double*)B, LEN),        \
    double[LEN]:        arr_eq_d((double*)A,         (double*)B, LEN),        \
    default:            memcmp((void*)A,             (void*)B, LEN) == 0)     \
)

#define _ASSERT_FAIL(exp)\
    (_current_test->status += assertion_failure("TEST(%s) %s:%d '%s'\n",\
                _current_test->name, __FILE__, __LINE__, exp))

#ifndef assert
#undef assert
#define assert(exp) (exp) ? ((void)0) : _ASSERT_FAIL(#exp);
#endif

#define assert_eq(A, B)               \
    ({__typeof__(A) _A = A;           \
    __typeof__(B) _B = B;             \
    (_EQ_EXPR(_A, _B)) ?              \
        ((void)0) :                   \
        _ASSERT_FAIL(#A " == " #B);})

#define assert_not_eq(A, B)  \
    ({__typeof__(A) _A = A;  \
    __typeof__(B) _B   = B;  \
    (!_EQ_EXPR(_A, _B)) ?    \
        ((void)0) :          \
        _ASSERT_FAIL(#A " != " #B);})

/**
 * assert that the memory stored at two address for length `LEN` are equal
 */
#define assert_eqn(A, B, LEN)     \
    (binary_compare((byte_t*)(uintptr_t)A, (byte_t*)(uintptr_t)B, LEN)) ? \
        ((void)0) :               \
        _ASSERT_FAIL(#A " == " #B)

/**
 * assert that the memory stored at two address for length `LEN` are not equal
 */
#define assert_not_eqn(A, B, LEN)  \
    (!binary_compare((byte_t*)(uintptr_t)A, (byte_t*)(uintptr_t)B, LEN)) ? \
        ((void)0) :                \
        _ASSERT_FAIL(#A " != " #B)

#define eq(A, B)         assert_eq(A, B)
#define not_eq(A, B)     assert_not_eq(A, B)
#define eqn(A, B, L)     assert_eqn(A, B, L)
#define not_eqn(A, B, L) assert_not_eqn(A, B, L)

#define _TEST_NAME(NAME) _utest_##NAME
#define _TEST_DECL(NAME) void _TEST_NAME(NAME)(UTestRunner* runner __attribute__((unused)))

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
#define TEST(NAME, ...)                                                                      \
    _TEST_DECL(NAME);                                                                        \
    __attribute__((constructor))                                                             \
    void _add_##NAME##_to_tests(void) {                                                      \
        UTestSuite temp = { __VA_ARGS__ };                                                   \
                                                                                             \
        UTestSuite* newtest = malloc(sizeof(UTestSuite));                                    \
        newtest->name = #NAME;                                                               \
        newtest->test = _TEST_NAME(NAME);                                                    \
        newtest->status = 0;                                                                 \
        newtest->ignore = temp.ignore;                                                       \
        newtest->capture_output = temp.capture_output;                                       \
        newtest->output = NULL;                                                              \
                                                                                             \
        if (temp.setup != NULL)                                                              \
            newtest->setup = temp.setup;                                                     \
        else                                                                                 \
            newtest->setup = NULL;                                                           \
        if (temp.teardown != NULL)                                                           \
            newtest->teardown = temp.teardown;                                               \
        else                                                                                 \
            newtest->teardown = NULL;                                                        \
        if (!temp.ignore) {                                                                  \
            AllTests = (UTestSuite**)realloc(AllTests, (n_Tests + 1) * sizeof(UTestSuite*)); \
            AllTests[n_Tests++] = newtest;                                                   \
        } else                                                                               \
            free(newtest);                                                                   \
    }                                                                                        \
    _TEST_DECL(NAME)

#define RECORD_OUTPUT(BUFFER)        \
char *BUFFER = NULL;                 \
_current_test->capture_output = 1;   \
if (_current_test->output != NULL) { \
    free(_current_test->output);     \
    _current_test->output = NULL;    \
}                                    \
while (utest_capture_output(&BUFFER))

#if !defined(_UTEST_IMPL)
// runs before any tests
__attribute__((constructor(101)))
void __setup(void)
{
    AllTests = (UTestSuite**)malloc(0);
    n_Tests = 0;
}

__attribute__((destructor))
void __cleanup(void)
{
    for (int i = 0; i < n_Tests; i++) {
        free(AllTests[i]);
    }
    free(AllTests);
}
#endif /* !defined(_UTEST_IMPL) */

#ifdef __cplusplus
}
#endif

#endif /* _UTEST_H */