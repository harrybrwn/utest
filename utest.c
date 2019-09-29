#define _UTEST_IMPL
#include "utest.h"
#include <stdarg.h>
#include <unistd.h>

int n_Tests;
UTestSuite *_current_test;
UTestSuite **AllTests;

static void RunnerInit(UTestRunner*);

#define COL_OK      "\x1b[1;32m"
#define COL_WARNING "\x1b[1;35m"
#define COL_ERROR   "\x1b[1;31m"
#define COL_RESET   "\x1b[0m"
#define MSG_OK   COL_OK "Ok" COL_RESET
#define MSG_ERR  COL_ERROR "Fail" COL_RESET

int RunTests(void)
{
    int status = 0;
    UTestRunner runner;
    RunnerInit(&runner);

    for (int i = 0; i < n_Tests; i++)
    {
        _current_test = AllTests[i];
        runner.current_test = AllTests[i];
        if (!_current_test->ignore)
            _current_test->test(&runner);

        if (_current_test->status != 0)
            status += 1;
        printf(".");
    }

    if (status == 0)
        printf("\n" MSG_OK ": %d of %d tests passed\n", n_Tests - status, n_Tests);
    else
        printf("\n" MSG_ERR ": %d of %d tests passed\n", n_Tests - status, n_Tests);

    return status;
}

static int RunnerFail(const char* fmt, ...)
{
    char fmtbuf[256];
    va_list args;
    snprintf(fmtbuf, sizeof(fmtbuf), COL_ERROR "Assertion Failure:" COL_RESET " %s", fmt);

    va_start(args, fmt);
    vfprintf(stderr, fmtbuf, args);
    va_end(args);
    abort();
}

static void RunnerInit(UTestRunner* runner)
{
    runner->fail = RunnerFail;
    runner->warning = assertion_warning;
}

int assertion_failure(const char* fmt, ...)
{
    char fmtbuf[256];
    va_list args;
    snprintf(fmtbuf, sizeof(fmtbuf), COL_ERROR "Assertion Failure:" COL_RESET " %s", fmt);

    va_start(args, fmt);
    vfprintf(stderr, fmtbuf, args);
    va_end(args);
    return 1;
}

int assertion_warning(const char* fmt, ...)
{
    char fmtbuf[256];
    va_list args;
    snprintf(fmtbuf, sizeof(fmtbuf), COL_WARNING "Assertion Warning:" COL_RESET " %s", fmt);

    va_start(args, fmt);
    vfprintf(stderr, fmtbuf, args);
    va_end(args);
    return 1;
}

int utest_capture_output(char *buf, size_t len)
{
    static int init = 1;
    static int stdout_save = -1;
    static int outpipe[2] = {-1, -1};

    fflush(stdout);
    if (init) // initialize output capture
    {
        if ((stdout_save = dup(STDOUT_FILENO)) == -1) {
            fprintf(stderr, "couldn't copy stdout\n");
            exit(1);
        }

        if (pipe(outpipe) != 0)
            fprintf(stderr, "couldn't create output capture pipe\n");
        if (dup2(outpipe[1], STDOUT_FILENO) == -1)
            fprintf(stderr, "couldn't rediect stdout to pipe\n");
        close(outpipe[1]);

        init = 0; // done with initialization
        return 1;
    }
    else // end output capture
    {
        ssize_t readlen;
        if ((readlen = read(outpipe[0], buf, len - 1)) < 0)
            fprintf(stderr, "read error\n");
        // buf[readlen] = '\0';

        if (dup2(stdout_save, STDOUT_FILENO) == -1)
            fprintf(stderr, "couldn't reset stdout\n");

        init = 1; // should run init stage next time capture_output is run
        outpipe[1] = -1; outpipe[0] = -1;
        stdout_save = -1;
        return 0;
    }
}

/**
 * Return: 1 for a match, 0 for not the same.
 */
int binary_compare(byte_t* left, byte_t* right, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (left[i] != right[i]) {
            return 0;
        }
    }
    return 1;
}

#define _ARR_EQ_IMPL(SUFFIX, TYPE)\
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
}                                                               \
int arr_eq_##SUFFIX(TYPE *arr1, TYPE *arr2, size_t len) {       \
    for (size_t i = 0; i < len; i++)                            \
        if (arr1[i] != arr2[i])                                 \
            return 0;                                           \
    return 1;                                                   \
}

_ARR_EQ_IMPL(i, int)
_ARR_EQ_IMPL(ui, unsigned int)
_ARR_EQ_IMPL(l, long)
_ARR_EQ_IMPL(ul, unsigned long)
_ARR_EQ_IMPL(f, float)
_ARR_EQ_IMPL(d, double)

#undef _ARR_EQ_IMPL

int arr_eq_s(char** arr1, char** arr2, size_t len) {
    for (size_t i = 0; i < len; i++)
        if (strcmp(arr1[i], arr2[i]) != 0)
            return 0;
    return 1;
}

int strcomp(char* one, char* two) {
    while (*one)
        if (*one++ != *two++)
            return 1;
    return 0;
}