CC = gcc
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
CFLAGS = -std=c99 -Wall -Wextra -O2

# Game directories
MANIC_SRC_DIR = ../ManicMiner/src
JETSET_SRC_DIR = ../JetSetWilly/src

# Package config
GTK_CFLAGS = $(shell pkg-config --cflags gtk4)
GTK_LIBS = $(shell pkg-config --libs gtk4)
SDL_CFLAGS = $(shell pkg-config --cflags sdl2)
SDL_LIBS = $(shell pkg-config --libs sdl2)

# Build version
BUILD = -DBUILD=\"v3.0.$(shell date +'%y.%m.%d')\"

# Source files for Manic Miner (excluding main.c)
MANIC_SRCS = $(filter-out $(MANIC_SRC_DIR)/main.c, $(wildcard $(MANIC_SRC_DIR)/*.c))
MANIC_OBJS = $(patsubst $(MANIC_SRC_DIR)/%.c, build/manic_%.o, $(MANIC_SRCS))

# Source files for Jet Set Willy (excluding main.c)
JETSET_SRCS = $(filter-out $(JETSET_SRC_DIR)/main.c, $(wildcard $(JETSET_SRC_DIR)/*.c))
JETSET_OBJS = $(patsubst $(JETSET_SRC_DIR)/%.c, build/jetset_%.o, $(JETSET_SRCS))

# All object files
ALL_OBJS = $(MANIC_OBJS) $(JETSET_OBJS) build/manic_main_wrapper.o build/jetset_main_wrapper.o build/unified_main.o

TARGET = unified-game-launcher

.PHONY: all clean

all: $(TARGET)

# Create build directory
build:
	mkdir -p build

# Compile Manic Miner sources
build/manic_%.o: $(MANIC_SRC_DIR)/%.c | build
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -DMANIC_MINER -DMANIC_PREFIX=manic_ -c $< -o $@

# Special rule for manic loader (needs BUILD definition)
build/manic_loader.o: $(MANIC_SRC_DIR)/loader.c | build
	$(CC) $(CFLAGS) $(SDL_CFLAGS) $(BUILD) -DMANIC_MINER -c $< -o $@

# Compile Jet Set Willy sources  
build/jetset_%.o: $(JETSET_SRC_DIR)/%.c | build
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -DJET_SET_WILLY -c $< -o $@

# Special rule for jetset loader (needs BUILD definition)
build/jetset_loader.o: $(JETSET_SRC_DIR)/loader.c | build
	$(CC) $(CFLAGS) $(SDL_CFLAGS) $(BUILD) -DJET_SET_WILLY -c $< -o $@

# Compile main wrappers
build/manic_main_wrapper.o: manic_main_wrapper.c | build
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -I$(MANIC_SRC_DIR) -DMANIC_MINER -c $< -o $@

build/jetset_main_wrapper.o: jetset_main_wrapper.c | build
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -I$(JETSET_SRC_DIR) -DJET_SET_WILLY -c $< -o $@

# Compile unified main
build/unified_main.o: main.cpp | build
	$(CXX) $(CXXFLAGS) $(GTK_CFLAGS) -I$(MANIC_SRC_DIR) -I$(JETSET_SRC_DIR) -c $< -o $@

# Link everything together
$(TARGET): $(ALL_OBJS)
	$(CXX) $(ALL_OBJS) -o $@ $(GTK_LIBS) $(SDL_LIBS) -lm

clean:
	rm -rf build $(TARGET)
