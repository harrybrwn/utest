#include "utest.h"
#include "test_cases.h"

long fac(int x)
{
    if (x == 0)
        return 1;
    return x * fac(x - 1);
}

TEST(factorial)
{
    assert_eq(fac(1), 1);
    assert(fac(2) == 2);
    assert(fac(3) == 6);
    assert(fac(4) == 6 * 4);
    assert_eq(fac(5), 5 * 6 * 4);
}

#pragma GCC diagnostic push
// for the string comparisons in the default case in assert_eq
// #pragma GCC diagnostic ignored "-Waddress"
#pragma GCC diagnostic ignored "-Waddress"

TEST(utest_tests)
{
    assert_eq("one", "one");

    const char* a = "what?";
    assert_eq(a, "what?");
    assert_eq(1, 1);
    assert_eq('a', 'a');
}

#pragma GCC diagnostic pop

TEST(arr_equals_int)
{
    int stack_arr[] = {1, 2, 3, 4, 5, 6};
    int *arr = malloc(6 * sizeof(int));
    for (int i = 0; i < 6; i++)
        arr[i] = stack_arr[i];
    assert_n_eq(arr, arr, 6);
    free(arr);

    int a[] = {1, 2, 3, 4 , 5, 6};
    assert_n_eq(a, stack_arr, 6);
}

TEST(arr_equals_float)
{
    float stack_arr[] = {1.2f, 2.1f, 3.6f, 4.1f, 5.9f, 6.0f};
    float *arr = malloc(6 * sizeof(float));
    for (int i = 0; i < 6; i++)
        arr[i] = stack_arr[i];
    assert_n_eq(arr, arr, 6);
    free(arr);
}

TEST(arr_equals_string)
{
    char* arr[] = {"one", "two", "three"};

    assert_n_eq(arr, arr, 3);
    eqn(arr, arr, 3);
    char* arr2[] = {"three", "two", "one"};
    not_eqn(arr, arr2, 3);
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
    b.a = 1;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    eq(a.a, a.a);
#pragma GCC diagnostic pop
    eqn(&a, &b, sizeof(struct test));

    b.inner.size = 200;
    not_eqn(&a, &b, sizeof(struct test));
}