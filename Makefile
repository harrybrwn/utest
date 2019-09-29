CC=gcc
CFLAGS=-Wall -Wextra -g -I.

tests/%: tests/%.c utest.o
	$(CC) $(CFLAGS) -DAUTOTEST $^ -o $@

utest.o: utest.c utest.h
	$(CC) $(CFLAGS) -c $< -o $@

test: tests/test
	@tests/test

cov: test.gcno utest.gcno
	@./a.out > /dev/null
	gcov utest test

test.gcno utest.gcno: tests/test.c utest.c
	@$(CC) $(CFLAGS) -DAUTOTEST -fprofile-arcs -ftest-coverage $^

clean:
	$(RM) tests/test *.o *.out *.gcno *.gcov *.gcda

.PHONY: clean test