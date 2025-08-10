# GUI Game Launcher Makefile

TARGET = game-launcher-gui

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
GTKFLAGS = `pkg-config --cflags --libs gtk4`

SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(GTKFLAGS)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	mkdir -p ~/.local/bin
	cp $(TARGET) ~/.local/bin/

uninstall:
	rm -f ~/.local/bin/$(TARGET)

# Check GTK4 installation
check-deps:
	@echo "Checking GTK4 installation..."
	@pkg-config --exists gtk4 && echo "✓ GTK4 found" || (echo "❌ GTK4 not found. Install with: sudo pacman -S gtk4" && exit 1)

.PHONY: all clean install uninstall check-deps
