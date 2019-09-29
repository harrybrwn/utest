#include "utest.h"
#include "test_cases.h"

TEST(eq) {
    eq(0, 0);
    eq(NULL, NULL);
    char* a = "one";
    eq("one", a);
    char* b = "one";
    eq(b, "one");
    not_eq("eno", b);
    not_eq(b, "eno");
    eqn(a, b, strlen(a));
}

TEST(should_be_ignored, .ignore = 1)
{
    _ASSERT_FAIL("this test should not run!");
    exit(1);
}

TEST(bin_compare)
{
    {
        __typeof__("one") _left = "one";
        __typeof__("ono") _right = "ono";
        int res = binary_compare((byte_t*)(uintptr_t)&_left,
                                 (byte_t*)(uintptr_t)&_right,
                                 sizeof(_left));
        if (res) {
            assertion_failure("(%s:%d) binary compare returned unexpected result\n", __FILE__, __LINE__);
            runner->current_test->status += 1;
        }
    }

    {
        __typeof__("one") _l = "one";
        __typeof__("one") _r = "one";
        int res = binary_compare((byte_t*)(uintptr_t)&_l,
                                 (byte_t*)(uintptr_t)&_r,
                                 sizeof(_l));
        if (!res) {
            assertion_failure("(%s:%d) binary compare returned unexpected result\n", __FILE__, __LINE__);
            runner->current_test->status += 1;
        }
    }

    {
        __typeof__("one") _l = "one";
        __typeof__("one") _r = "one";
        int res = binary_compare((byte_t*)(uintptr_t)&_l,
                                 (byte_t*)(uintptr_t)&_r,
                                 sizeof(_r));
        if (!res) {
            assertion_failure("(%s:%d) binary compare returned unexpected result\n", __FILE__, __LINE__);
            runner->current_test->status += 1;
        }
    }

    {
        __typeof__(20UL) _l = 20UL;
        __typeof__(33UL) _r = 33UL;
        int res = binary_compare((byte_t*)(uintptr_t)&_l,
                                 (byte_t*)(uintptr_t)&_r,
                                 sizeof(_l));
        if (res) {
            assertion_failure("(%s:%d) binary compare returned unexpected result\n", __FILE__, __LINE__);
            runner->current_test->status += 1;
        }
    }

    {
        int *ptr = malloc(sizeof(int));
        *ptr = 10;
        int a = *ptr;
        __typeof__(a) _l = a;
        __typeof__(10) _r = 10;
        int res = binary_compare((byte_t*)(uintptr_t)&_l,
                                 (byte_t*)(uintptr_t)&_r,
                                 sizeof(_r));
        if (!res) {
            assertion_failure("(%s:%d) binary compare returned unexpected result\n", __FILE__, __LINE__);
            runner->current_test->status += 1;
        }
        free(ptr);
    }

    {
        int* left = malloc(5 * sizeof(int));
        int* right = malloc(5 * sizeof(int));
        for (int i = 0; i < 5; i++)  { left[i] = i; }
        for (int i = 5; i >= 0; i--) { right[i] = i; }

        __typeof__(left) _l = left;
        __typeof__(right) _r = right;
        int res = binary_compare((byte_t*)(uintptr_t)&_l,
                                 (byte_t*)(uintptr_t)&_r,
                                 sizeof(_l));
        if (res) {
            assertion_failure("(%s:%d) binary compare returned unexpected result\n", __FILE__, __LINE__);
            runner->current_test->status += 1;
        }
    }

    {
        struct test_struct {
            float b;
            int a;
            char c;
        } __attribute__((packed));
        struct test_struct t = {40, 5.5f, 'a'};

        __typeof__(t) _l = t;
        __typeof__(((struct test_struct){40, 5.5f, 'a'})) _r = {40, 5.5f, 'a'};
        int res = binary_compare((byte_t*)(uintptr_t)&_l,
                                 (byte_t*)(uintptr_t)&_r,
                                 sizeof(_l));
        if (!res) {
            assertion_failure("(%s:%d) binary compare returned unexpected result\n", __FILE__, __LINE__);
            runner->current_test->status += 1;
        }
    }
}

TEST(assert_equal)
{
    assert_eq(0, 0);
    for (int i = 0; i < 5; i++) {
        assert_eq(i, i);
        int a = i;
        assert_eqn(&i, &a, sizeof(int));
    }

    for (size_t i = 0; i < 100UL; i++) {
        assert_eq(i, i);
        size_t a = i;
        eqn(&i, &a, sizeof(i));
    }

    for (char i = 'a'; i < 'z'; i++) {
        assert_eq(i, i);
        char a = i;
        eqn(&i, &a, 1);
    }

    eq("one", "one");
    eqn("one", "one", sizeof("one"));
    not_eq("one", "two");
    not_eqn("one", "one-", sizeof("one"));
}

long fac(int x)
{
    if (x <= 0)
        return 1;
    return x * fac(x - 1);
}

TEST(factorial)
{
    assert_eq(fac(1), 1);
    eq(fac(2), 2);
    eq(6, fac(3));
    eq(fac(4), 6 * 4);
    eq(6*5*4, fac(5));

    assert_not_eq(fac(-1), -1);
    assert_eq(1, fac(-1));
    not_eq(fac(-10),-10);
    eq(fac(-10), 1);
}

#pragma GCC diagnostic push
// for the string comparisons in the default case in assert_eq
#pragma GCC diagnostic ignored "-Waddress"

TEST(utest_tests)
{
    assert_eq("one", "one");
    const char* a = "what?";
    assert_eq(a, "what?");
    assert_eq(1, 1);
    assert_eq('a', 'a');

    char *what = "some text";
    eq(what, "some text");
}

#pragma GCC diagnostic pop

TEST(arr_equals_int)
{
    {
        int stack_arr[] = {1, 2, 3, 4, 5, 6};
        int *arr = malloc(6 * sizeof(int));
        for (int i = 0; i < 6; i++)
            arr[i] = stack_arr[i];
        assert_eqn(arr, arr, 6);
        free(arr);
        int a[] = {1, 2, 3, 4 , 5, 6};
        assert_eqn(a, stack_arr, 6);
    }

    {
        int arr[] = {5, 4, 3, 2, 1};
        int arr2[] = {1, 2, 3, 4, 5};
        assert_not_eqn(arr, arr2, sizeof(arr)/sizeof(int));
        int arr3[] = {5, 4, 3, 2, 1};
        eqn(arr, arr3, sizeof(arr)/sizeof(int));
    }
}

TEST(arr_equals_float)
{
    float stack_arr[] = {1.2f, 2.1f, 3.6f, 4.1f, 5.9f, 6.0f};
    float *arr = malloc(6 * sizeof(float));
    for (int i = 0; i < 6; i++)
        arr[i] = stack_arr[i];
    assert_eqn(arr, arr, 6);
    free(arr);
}

TEST(arr_equals_string)
{
    char* arr[] = {"one", "two", "three"};
    char* arr2[] = {"one", "two", "three"};

    assert_eqn(arr, arr2, 3);
    eqn(arr, arr2, 3);

    char* arr3[] = {"three", "two", "one"};
    not_eqn(arr, arr3, 3);
}

TEST(assert_equals_string)
{
    char *base_str = "one";
    eq(base_str, "one");
    eq("one", base_str);

    char buf[4];
    for (int i = 0; i < 4; i++)
        buf[i] = base_str[i];
    eq(base_str, (char*)buf);
    eq((char*)buf, base_str);
    eq("one", (char*)buf);
    eq((char*)buf, "one");
    eq("one", "one");
}

TEST(struct_equals)
{
    struct test {
        int a;
        float b;
        struct {
            size_t size;
        } inner;
    };

    struct test a;
    struct test b;
    memset(&a, 0, sizeof(struct test));
    memset(&b, 0, sizeof(struct test));
    a.a = 1;
    a.b = 5.9f;
    a.inner.size = 20;
    b.a = 1;
    b.b = 5.9f;
    b.inner.size = 20;

    eq(a.a, a.a);
    eqn(&a, &b, sizeof(struct test));

    int res = binary_compare((byte_t*)(uintptr_t)&a, (byte_t*)(uintptr_t)&b, sizeof(struct test));
    assert(res == 1);
    eq(res, 1); // just for the extra milage

    b.inner.size = 200;
    not_eqn(&a, &b, sizeof(struct test));
}

int testing = 0;

TEST(output_capture_test, .ignore = 0)
{
    char buf[256];

    assert(utest_capture_output(NULL, 0) == 1);
    printf("hello");
    assert(utest_capture_output(buf, 256) == 0);
    assert_eqn(buf, "hello", 6);

    bzero(buf, 256);

    assert(utest_capture_output(buf, 256) == 1);
    printf("this is a test\n");
    printf("for multi-line...\n");
    printf("output captures");
    assert(utest_capture_output(buf, 256) == 0);
    // eq(buf,
    //     "this is a test\n"
    //     "for multi-line...\n"
    //     "output captures"
    // );
}

TEST(stuff, .ignore = 1)
{
    printf("\n");

    char buf[256];
    utest_capture_output(NULL, 0);

    printf("capturing output...\n");
    printf("is this even working !!!!!!!!!!!!!!\n");

    utest_capture_output(buf, 256);

    printf("buffer('\n");
    for (int i = 0; (i < 256) && (buf[i] != '\0'); i++) {
        printf("%c", buf[i]);
    }
    printf("')\n");
}