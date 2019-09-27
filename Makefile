CC=gcc
CFLAGS=-Wall -Wextra -g -I.

tests/test: tests/test.c tests/test_cases.h utest.h
	$(CC) $(CFLAGS) -DAUTOTEST -o $@ $<

test: tests/test
	@tests/test

clean:
	$(RM) tests/test tests/fail

.PHONY: clean test
