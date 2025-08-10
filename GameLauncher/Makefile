# Game Launcher Makefile

TARGET = game-launcher

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	cp $(TARGET) ~/.local/bin/

uninstall:
	rm -f ~/.local/bin/$(TARGET)

.PHONY: all clean install uninstall
