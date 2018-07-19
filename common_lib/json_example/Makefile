CC = gcc
CFLAGS = -O0 -g -Wall -Wextra -pedantic-errors

all: test testcpp

.PHONY: test testcpp
test: tests.c parson.c
	$(CC) $(CFLAGS) -o $@ tests.c parson.c

clean:
	rm -f test *.o

