CC ?= cc
CFLAGS ?= -Wall -Wextra -std=c99 -pedantic

.PHONY: test clean

test: test_ternary
	./test_ternary

test_ternary: test_ternary.c c-ternary.h
	$(CC) $(CFLAGS) -o test_ternary test_ternary.c -lm

clean:
	rm -f test_ternary
