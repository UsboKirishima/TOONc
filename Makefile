CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
TARGET = test_toonc
LIBS = -lm

all: $(TARGET) libtoonc.a

# Target for the main test executable
$(TARGET): test_toonc.c toonc.c
	$(CC) $(CFLAGS) -o $(TARGET) test_toonc.c toonc.c $(LIBS)

# Target for the static library
libtoonc.a: toonc.o
	ar rcs libtoonc.a toonc.o

toonc.o: toonc.c toonc.h
	$(CC) $(CFLAGS) -c toonc.c -o toonc.o

# Run the main test program
run: $(TARGET)
	./$(TARGET)

# Run the examples
run-examples: libtoonc.a
	$(MAKE) -C examples run

valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

clean:
	rm -f $(TARGET) *.o libtoonc.a
	$(MAKE) -C examples clean

.PHONY: all run run-examples valgrind clean