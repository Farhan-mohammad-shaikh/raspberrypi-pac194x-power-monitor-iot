CC = gcc
CFLAGS = -Wall -O2 -lpthread
TARGET = main
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	sudo ./$(TARGET)

clean:
	rm -f $(TARGET)