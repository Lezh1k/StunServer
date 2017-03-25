CC = gcc

LIBS = 
DEFS = -DNDEBUG 

# Directories
INCLUDE_DIR = include
BUILD_DIR = build
SRC_DIR = src
BIN_DIR = bin

#device and program
PRG = sedi_stun 
OPTIMIZE = -Os
INCLUDES = -Iinclude 

override CFLAGS = $(INCLUDES) $(OPTIMIZE) $(DEFS) -Wall -W 
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS	= $(patsubst %,$(BUILD_DIR)/%.o, $(subst src/,,$(subst .c,,$(SOURCES))))

all: directories $(PRG) 
$(PRG): $(BIN_DIR)/$(PRG).elf

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -Wall $(CFLAGS) -c $< -o $@

$(BIN_DIR)/$(PRG).elf: $(OBJECTS)
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

