#ifndef _UTEST_H
#define _UTEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>


typedef void (*TestMethod)(void);

int n_Tests;
TestMethod* AllTests;
int _utest_result = 0;

int RunTests(void) {
    for (int i = 0; i < n_Tests; i++)
    {
        AllTests[i]();
        printf(".");
    }

    if (_utest_result == 0)
        printf("\n\033[0;32mOk\033[0m\n");
    else
        printf("\n\033[0;31mFail\033[0m\n");

    return _utest_result;
}

#if defined(AUTOTEST) && !defined(_MAIN_DEFINED)
#define _MAIN_DEFINED
int main(void) {
    return RunTests();
}
#endif /* AUTOTEST */

int assertion_failure(const char* fmt, ...);

#define _ARR_UNORDERED_EQ(SUFFIX, TYPE)                         \
int arr_unordered_eq_##SUFFIX(TYPE *a1, TYPE *a2, size_t len) { \
    for (size_t i = 0; i < len; i++) {                          \
        for (size_t k = 0; k < len; k++) {                      \
            if (a1[i] == a2[k]) {                               \
                goto Found;                                     \
            }                                                   \
        }                                                       \
        return 0;                                               \
    Found:;                                                     \
    }                                                           \
    return 1;                                                   \
}

#define _ARR_ORDERED_EQ(SUFFIX, TYPE)                     \
int arr_eq_##SUFFIX(TYPE *arr1, TYPE *arr2, size_t len) { \
    for (size_t i = 0; i < len; i++)                      \
        if (arr1[i] != arr2[i])                           \
            return 0;                                     \
    return 1;                                             \
}

#define _ARR_EQ_ALL(SUFFIX, TYPE) \
_ARR_ORDERED_EQ(SUFFIX, TYPE)     \
_ARR_UNORDERED_EQ(SUFFIX, TYPE)

_ARR_EQ_ALL(i, int)
_ARR_EQ_ALL(ui, unsigned int)
_ARR_EQ_ALL(l, long)
_ARR_EQ_ALL(ul, unsigned long)
_ARR_EQ_ALL(f, float)
_ARR_EQ_ALL(d, double)

int arr_eq_s(char** arr1, char** arr2, size_t len) {
    for (size_t i = 0; i < len; i++)
        if (strcmp(arr1[i], arr2[i]) != 0)
            return 0;
    return 1;
}

#define _EQ_EXPR(A, B)                                                          \
(_Generic((A),                                                                  \
    char*:       strcmp(((char*)(uintptr_t)(A)), ((char*)(uintptr_t)(B))) == 0, \
    const char*: strcmp(((char*)(uintptr_t)(A)), ((char*)(uintptr_t)(B))) == 0, \
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

#ifndef assert
#undef assert
#define assert(exp) (exp) ? ((void)0) : (_utest_result += assertion_failure("%s:%d '%s'\n", __FILE__, __LINE__, #exp));
#endif

#define assert_eq(A, B) \
    (_EQ_EXPR(A, B)) ?  \
        ((void)0) :     \
        (_utest_result += assertion_failure("%s:%d '%s'\n", __FILE__, __LINE__, #A " eq " #B))

#define assert_not_eq(A, B)  \
    (!_EQ_EXPR(A, B)) ?      \
        ((void)0) :          \
        (_utest_result += assertion_failure("%s:%d '%s'\n", __FILE__, __LINE__, #A " eq " #B))

#define assert_n_eq(A, B, LEN)    \
    (_ARRAY_EQ_EXPR(A, B, LEN)) ? \
        ((void)0) :               \
        (_utest_result += assertion_failure("%s:%d '%s'\n", __FILE__, __LINE__, #A " eq " #B))

#define assert_n_not_eq(A, B, LEN) \
    (!_ARRAY_EQ_EXPR(A, B, LEN)) ? \
        ((void)0) :                \
        (_utest_result += assertion_failure("%s:%d '%s'\n", __FILE__, __LINE__, #A " eq " #B))

#define eq(A, B)         assert_eq(A, B)
#define eqn(A, B, L)     assert_n_eq(A, B, L)
#define not_eqn(A, B, L) assert_n_not_eq(A, B, L)

#define _TEST_DECL(NAME) void _utest_##NAME(void)

#define TEST(NAME)                                                                     \
    _TEST_DECL(NAME);                                                                  \
    __attribute__((constructor))                                                       \
    void _add_##NAME##_to_tests(void) {                                                \
        AllTests = (TestMethod*)realloc(AllTests, (n_Tests + 1) * sizeof(TestMethod)); \
        AllTests[n_Tests++] = _utest_##NAME;                                           \
    }                                                                                  \
    _TEST_DECL(NAME)

// runs before any tests
__attribute__((constructor(101)))
void __setup(void)
{
    AllTests = (TestMethod*)malloc(sizeof(TestMethod*));
    n_Tests = 0;
}

__attribute__((destructor))
void __cleanup(void)
{
    free(AllTests);
}

int assertion_failure(const char* fmt, ...)
{
    char fmtbuf[256];
    va_list args;
    snprintf(fmtbuf, sizeof(fmtbuf), "\033[0;31mAssertion Failure:\033[0m %s", fmt);

    va_start(args, fmt);
    vfprintf(stderr, fmtbuf, args);
    va_end(args);

    // raise(SIGABRT);
    return 1;
}

#ifdef __cplusplus
}
#endif

#endif /* _UTEST_H */