CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
TARGET = test_toonc
LIBS = -lm

all: $(TARGET)

$(TARGET): test_toonc.c toonc.c
	$(CC) $(CFLAGS) -o $(TARGET) test_toonc.c toonc.c $(LIBS)

run: $(TARGET)
	./$(TARGET)

valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

clean:
	rm -f $(TARGET) *.o

.PHONY: all run valgrind clean