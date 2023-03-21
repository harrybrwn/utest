# utest
A lightweight, single file, testing framework for c/c++

## Setup

This is not necessary for using this framework but it is how I usually set
things up.

First, create a file `tests/test.c` to put the starter code in. The `AUTOTEST`
macro is important here because it tells the library to automatically insert
a main function to run all the tests.

```c
#define AUTOTEST
#include "utest/utest.h"

#include <stdio.h>

TEST(hello_world)
{
    printf("hello, world\n");
    assert_not_eq(0, 1);
}
```

Next, install the helper makefile.

```
curl -sLO https://raw.githubusercontent.com/harrybrwn/utest/master/utest.mk
```

Finally, we setup the build system with a few commands in a Makefile.

```makefile
UTEST_BIN=./tests/test
UTEST_DEPS=example-file.o example-file.h

example-file.o: example-file.c
  $(CC) -c $< -o $@

include utest.mk
```

Now build and run the tests.

```
make test
```

## Functions and Macros

- `int RunTests(void)` Run all the tests.
- `void ut_timer_start(struct utest_timer*)` Start a timer.
- `void ut_timer_end(struct utest_timer*)` End the timer.
- `double ut_timer_se(struct utest_timer)` Give the duration of the timer in
  seconds.
- `#define FAIL(EXP)` Fail the current test with the error message in `EXP`.
- `#define FAILF(FMT, EXP)` Same as `FAIL` except with a user format string.
- `#define assert(EXP)` Fails the test if `EXP` is not evaluated to be true.
- `#define assert_eq(A, B)` Fails the test if `A` and `B` are not equal.
- `#define assert_not_eq(A, B)` Fails the test if `A` and `B` are equal.
- `#define assert_eqn(A, B, N)` Fails the test if `A` and `B` (having length
  `N`) are not equal.
- `#define assert_not_eqn(A, B, B)` Fails the test if `A` and `B` (having length
  `N`) are equal.
- `#define TEST(NAME, ...)` Define a unit test having the name `NAME`. This
  macro acts as a function header that does not define the function body. Each
  test definition can be given options as well. A common option is the option to
  automatically ignore a test with the `.ignore = 1` option. See the `UTestCase`
  type. To use the macro, it should have a function body as if `TEST(test_name)`
  was a function definition such as `void test_name()`. This will look something
  like the following.

```c
TEST(test_name) { /* test body */ }

TEST(ignored_test, .ignore = 1)
{
    /* ... */
}
```

- `#define CATCH_OUTPUT(BUFFER)` Capture the output of a block of code and store
  it in a character buffer named `BUFFER` with length `BUFFER_length`.
- `#define CURRENT_TEST_NAME` Name of the current test being run.
- `#define eq(A, B)` An alias for `assert_eq`.
- `#define not_eq(A, B)` An alias for `assert_not_eq`.
- `#define eqn(A, B, N)` An alias for `assert_eqn`.
- `#define not_eqn(A, B, N)` An alias for `assert_not_eqn`.

## Types and Structs

- `UTestCase` A struct that holds all the metadata for one test.
