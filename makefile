# =============================================================================
# MAKEFILE FOR TINY SSL SERVER
# =============================================================================

.PHONY: all clean

# Compiler and paths
CC = cc
OPENSSL_PATH = /opt/homebrew/opt/openssl@3
CFLAGS = -I$(OPENSSL_PATH)/include
LDFLAGS = -L$(OPENSSL_PATH)/lib -lssl -lcrypto

# Directories
BIN_DIR = .bin
OBJ_DIR = .obj

# Files
TARGET = $(BIN_DIR)/tinyserver
SRC = src/tinyserver.c
OBJ = $(OBJ_DIR)/tinyserver.o

# Default target
all: $(TARGET)

# Link executable
$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CC) -o $(TARGET) $(OBJ) $(LDFLAGS)

# Compile object file
$(OBJ): $(SRC) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

# Create directories
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean build artifacts
clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)