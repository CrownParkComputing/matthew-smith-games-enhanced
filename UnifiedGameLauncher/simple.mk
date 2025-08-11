CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -O2

# Package config
GTK_CFLAGS = $(shell pkg-config --cflags gtk4)
GTK_LIBS = $(shell pkg-config --libs gtk4)

TARGET = unified-game-launcher

.PHONY: all clean

all: $(TARGET)

$(TARGET): simple_launcher.cpp
	$(CC) $(CFLAGS) $(GTK_CFLAGS) simple_launcher.cpp -o $(TARGET) $(GTK_LIBS)

clean:
	rm -f $(TARGET)
