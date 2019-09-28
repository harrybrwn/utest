CC=gcc
CFLAGS=-Wall -Wextra -g -I.


tests/%: tests/%.c utest.o
	$(CC) $(CFLAGS) -DAUTOTEST $^ -o $@

utest.o: utest.c utest.h
	$(CC) $(CFLAGS) -c $< -o $@

test: tests/test
	@tests/test

clean:
	$(RM) tests/test *.o

.PHONY: clean test