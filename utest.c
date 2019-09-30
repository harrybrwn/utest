#define _UTEST_IMPL
#include "utest.h"
#undef _UTEST_IMPL
#include <stdarg.h>
#include <unistd.h>

int n_Tests;
UTestCase *_current_test;
UTestCase **AllTests;

__attribute__((constructor(101)))
void __setup(void)
{
    AllTests = (UTestCase**)malloc(0);
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

static void RunnerInit(UTestRunner*);
static int RunTest(UTestRunner*);
static int PrintIgnored(void);

#define COL_OK      "\x1b[1;32m"
#define COL_WARNING "\x1b[1;35m"
#define COL_ERROR   "\x1b[1;31m"
#define COL_RESET   "\x1b[0m"
#define MSG_OK   COL_OK "Ok" COL_RESET
#define MSG_FAIL  COL_ERROR "Fail" COL_RESET

int RunTests(void)
{
    int status = 0;
    int n = n_Tests;
    UTestRunner runner;
    RunnerInit(&runner);

    n -= PrintIgnored();

    for (int i = 0; i < n; i++)
    {
        _current_test = AllTests[i];
        if (_current_test->ignore) {
            continue;
        }

        runner.test = _current_test;
        status += RunTest(&runner);
    }
    _current_test = NULL;

    if (status == 0)
        printf("\n" MSG_OK ": %d of %d tests passed\n", n - status, n);
    else
        printf("\n" MSG_FAIL ": %d of %d tests passed\n", n - status, n);

    return status;
}

static int RunTest(UTestRunner* r) {
    if (r->test->ignore)
        return 0;

    if (r->test->setup != NULL)
        r->test->setup();

    r->test->test(r);

    if (r->test->teardown != NULL)
        r->test->teardown();

    if (r->test->capture_output)
        free(r->test->output);
    if (r->test->status == 0)
        printf(".");
    else
        printf("x");
    return r->test->status;
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

void BuildTestCase(UTestCase opt, TestMethod tst, char *name) {
    UTestCase* newtest = malloc(sizeof(UTestCase));
    newtest->name = name;
    newtest->test = tst;
    newtest->status = 0;
    newtest->ignore = opt.ignore;
    newtest->capture_output = opt.capture_output;
    newtest->output = NULL;

    if (opt.setup != NULL)
        newtest->setup = opt.setup;
    else
        newtest->setup = NULL;

    if (opt.teardown != NULL)
        newtest->teardown = opt.teardown;
    else
        newtest->teardown = NULL;

    AllTests = (UTestCase**)realloc(
        AllTests, (n_Tests + 1) * sizeof(UTestCase*));
    AllTests[n_Tests++] = newtest;
}

static int PrintIgnored(void)
{
    int i, n = 0;
    for (i = 0; i < n_Tests; i++) {
        if (AllTests[i]->ignore) {
            printf(COL_WARNING "Ignoring testcase: " COL_RESET "'%s'\n", AllTests[i]->name);
            n++;
        }
    }
    return n;
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
    vfprintf(stdout, fmtbuf, args);
    va_end(args);
    return 1;
}

size_t pipe_read_util(int fd, char** buffer) {
    char buf[256];
    size_t buffer_len = 0;
    size_t read_count = read(fd, buf, sizeof(buf)-1);

    if (read_count > 0) {
        if (*buffer == NULL) {
            *buffer = malloc(read_count + 1);
        }
        memcpy(*buffer, buf, read_count + 1);
    }
    buffer_len = read_count;

    while (read_count == sizeof(buf) - 1) {
        read_count = read(fd, buf, sizeof(buf) - 1);
        buffer_len += read_count;

        *buffer = realloc(*buffer, buffer_len + 1);
        memcpy(*buffer + buffer_len - read_count, buf, read_count + 1);
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
        write(outpipe[1], "", 1);
        close(outpipe[1]);

        dup2(stdout_save, STDOUT_FILENO);

        pipe_read_util(outpipe[0], buf);
        _current_test->output = *buf;
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

#define _ARR_EQ_IMPL(SUFFIX, TYPE)                              \
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

int arr_unordered_eq_s(char** a1, char** a2, size_t len)
{
    size_t i, k;
    for (i = 0; i < len; i++) {
        for (k = 0; k < len; k++) {
            if (strcmp(a1[i], a2[k]) == 0) {
                goto Found;
            }
        }
        return 0;
    Found:;
    }
    return 1;
}

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