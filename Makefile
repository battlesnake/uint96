CC = gcc
CFLAGS = -DTEST -DSHORT_PASS -O2 -fsanitize=undefined -fsanitize=address -Wall -Wextra -Wno-missing-braces

sources = $(wildcard test_*.c)

tests = $(sources:%.c=%)

targets = $(sources:test_%.c=%)

.PHONY: all $(targets)

all: $(targets)

clean:
	rm -rf -- $(tests) test_*.d

$(targets): %: test_%
	./$<

$(tests): test_%: test_%.c
	$(CC) $(CFLAGS) -MD -MF $@.d -o $@ $^

test_uint96: uint96.c

-include: $(wildcard *.d)
