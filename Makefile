CC = gcc

DEFS = -DNDEBUG 

# Directories
INCLUDE_DIR = include
BUILD_DIR = build
SRC_DIR = src
BIN_DIR = bin
PREFIX = /usr/local/bin

#device and program
PRG = mad_stun 
OPTIMIZE = -Os
LIBS = -lpthread
INCLUDES = -Iinclude 

override CFLAGS = $(INCLUDES) $(OPTIMIZE) $(DEFS) $(LIBS) -Wall -W 
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS	= $(patsubst %,$(BUILD_DIR)/%.o, $(subst src/,,$(subst .c,,$(SOURCES))))

all: directories $(PRG) 
$(PRG): $(BIN_DIR)/$(PRG)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -Wall $(CFLAGS) -c $< -o $@

$(BIN_DIR)/$(PRG): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

clean:
	@rm -rf $(BUILD_DIR)/*
	@rm -rf $(BIN_DIR)/*

mrproper:
	@rm -rf $(BUILD_DIR) 
	@rm -rf $(BIN_DIR)

install:
	@cp $(BIN_DIR)/$(PRG) $(PREFIX)
