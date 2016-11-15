CC ?= gcc
CFLAGS = -Wall -g -std=c99
INCLUDES = -I../include/
LIBRARY = libcerial.a

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,objects/%,$(SRCS))
TESTS = $(patsubst %.c,objects/%.test,$(SRCS))
VALGRINDS = $(patsubst %.c,objects/%.valgrind,$(SRCS))

all: folders $(OBJS) $(TESTS)

valgrind: folders $(OBJS) $(VALGRINDS)

folders:
	mkdir -p objects

clean:
	rm -rf objects

objects/%.test: objects/%
	./$<

objects/%.valgrind: objects/%
	valgrind --error-exitcode=1 --leak-check=full --track-origins=yes --suppressions=valgrind-osx.suppressions $<

objects/%: %.c ../objects/$(LIBRARY)
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@ ../objects/$(LIBRARY)
