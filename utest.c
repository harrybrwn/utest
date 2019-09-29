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
#define MSG_FAIL  COL_ERROR "Fail" COL_RESET

int RunTests(void)
{
    int status = 0;
    UTestRunner runner;
    RunnerInit(&runner);

    for (int i = 0; i < n_Tests; i++)
    {
        _current_test = AllTests[i];
        if (_current_test->ignore) {
            continue;
        }

        runner.current_test = AllTests[i];

        if (_current_test->setup != NULL)
            _current_test->setup();

        _current_test->test(&runner);

        if (_current_test->teardown != NULL)
            _current_test->teardown();

        if (_current_test->capture_output == 1)
            free(_current_test->output);

        if (_current_test->status != 0)
            status += 1;
        printf(".");
    }

    if (status == 0)
        printf("\n" MSG_OK ": %d of %d tests passed\n", n_Tests - status, n_Tests);
    else
        printf("\n" MSG_FAIL ": %d of %d tests passed\n", n_Tests - status, n_Tests);

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
    runner->warning = utest_warning;
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

int utest_warning(const char* fmt, ...)
{
    char fmtbuf[256];
    va_list args;
    snprintf(fmtbuf, sizeof(fmtbuf), COL_WARNING "Test Warning:" COL_RESET " %s", fmt);

    va_start(args, fmt);
    vfprintf(stderr, fmtbuf, args);
    va_end(args);
    return 1;
}

size_t read_util(int fd, char** buffer) {
    char buf[256];
    size_t buffer_len = 0;
    size_t read_count = read(fd, buf, sizeof(buf)-1);

    if (read_count > 0) {
        if (*buffer == NULL) {
            *buffer = malloc(read_count + 1);
            _current_test->output = *buffer;
        }
        memcpy(*buffer, buf, read_count + 1);
    }
    buffer_len = read_count;

    while (read_count == sizeof(buf) - 1) {
        read_count = read(fd, buf, sizeof(buf) - 1);
        buffer_len += read_count;

        *buffer = realloc(*buffer, buffer_len + 1);
        memcpy(*buffer + buffer_len - read_count, buf, read_count + 1);
        _current_test->output = *buffer;
    }
    return buffer_len;
}

int utest_capture_output(char **buf)
{
    static int init = 1;
    static int stdout_save = -1;
    static int outpipe[2] = {-1, -1};

    fflush(stdout);
    if (init) // initialize output capture
    {
        if (pipe(outpipe) != 0) {
            fprintf(stderr, "couldn't create output capture pipe\n");
            exit(1);
        }
        if ((stdout_save = dup(STDOUT_FILENO)) == -1)
            fprintf(stderr, "couldn't copy stdout\n");
        if (dup2(outpipe[1], STDOUT_FILENO) == -1)
            fprintf(stderr, "couldn't rediect stdout to pipe\n");

        init = 0; // done with initialization
        return 1;
    }
    else // end output capture
    {
        int terminated = write(outpipe[1], "", 1) == 1;
        close(outpipe[1]);

        dup2(stdout_save, STDOUT_FILENO);

        if (terminated) {
            // ssize_t readlen;
            // if ((readlen = read(outpipe[0], buf, len - 1)) < 0)
            //     fprintf(stderr, "read error\n");
            read_util(outpipe[0], buf);
        }
        close(outpipe[0]);

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