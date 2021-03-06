CC ?= gcc
AR ?= ar
CFLAGS = -Wall -g
INCLUDES = -Iinclude/

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c,objects/%.o,$(SRCS))
OUTPUT = libcerial.a

all: folders objects/$(OUTPUT)

test: folders objects/$(OUTPUT)
	cd test && make

valgrind: folders objects/$(OUTPUT)
	cd test && make valgrind

folders:
	mkdir -p objects

clean:
	cd test && make clean
	rm -rf objects

objects/$(OUTPUT): $(OBJS)
	$(AR) rcs $@ $(OBJS)

objects/%.o: src/%.c $(wildcard include/*.h) $(wildcard src/*.h)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<