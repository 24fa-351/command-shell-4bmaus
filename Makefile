CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g
TARGET = xsh
OBJS = main.o xsh.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c xsh.h
	$(CC) $(CFLAGS) -c main.c

xsh.o: xsh.c xsh.h
	$(CC) $(CFLAGS) -c xsh.c

clean:
	rm -f $(OBJS) $(TARGET)

run: all
	./$(TARGET)