CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = -L.. -ltoonc
LDLIBS = -lm

# Find all .c files in the current directory
SOURCES = $(wildcard *.c)
# Create a list of executables from the source files
TARGETS = $(patsubst %.c,%,$(SOURCES))

# The default goal is to build all examples
all: $(TARGETS)

# A generic rule for building an executable from a .c file
%: %.c ../libtoonc.a
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LDLIBS)

# Create the static library if it doesn't exist
../libtoonc.a: ../toonc.c ../toonc.h
	$(MAKE) -C .. libtoonc.a

run: all
	@echo "--- Running all examples ---"
	@for target in $(TARGETS); do \
		./$$target; \
	done
	@echo "--- All examples finished ---"

clean:
	rm -f $(TARGETS)

.PHONY: all run clean
