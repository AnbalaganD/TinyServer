# =============================================================================
# MAKEFILE BASICS FOR BEGINNERS
# =============================================================================
# A Makefile automates building your program. It has:
# 1. VARIABLES (store values like CC = cc)
# 2. TARGETS (things to build like 'all' or 'clean')
# 3. RULES (how to build targets: target: dependencies + commands)
# 4. COMMANDS (shell commands that do the actual work)
#
# When you type 'make', it runs the first target (usually 'all')
# When you type 'make clean', it runs the 'clean' target
# =============================================================================

# PHONY TARGETS: Tell make these don't create actual files
# Without this, if you had a file named 'all' or 'clean', make would get confused
.PHONY: all clean

# =============================================================================
# VARIABLES SECTION
# =============================================================================
# Variables store values you can reuse. Use $(VARIABLE_NAME) to access them

# CC = Compiler Command (what program compiles your code)
CC = cc

# Where OpenSSL is installed on your Mac (needed for HTTPS/SSL)
OPENSSL_PATH = /opt/homebrew/opt/openssl@3

# CFLAGS = Compiler Flags (options for the compiler)
# -I tells compiler where to find header files (.h files)
CFLAGS = -I$(OPENSSL_PATH)/include

# LDFLAGS = Linker Flags (options for linking libraries)
# -L tells linker where to find library files
# -lssl -lcrypto tells linker to use OpenSSL libraries
LDFLAGS = -L$(OPENSSL_PATH)/lib -lssl -lcrypto

# Directory names (using . makes them hidden folders)
BIN_DIR = .bin    # Where final executable goes
OBJ_DIR = .obj    # Where intermediate .o files go

# File paths using variables above
TARGET = $(BIN_DIR)/tinyserver    # Final program: .bin/tinyserver
SRC = src/tinyserver.c            # Your source code
OBJ = $(OBJ_DIR)/tinyserver.o     # Compiled object file

# =============================================================================
# TARGETS AND RULES SECTION
# =============================================================================
# Format: target: dependencies
#         [TAB]command
#         [TAB]command
# IMPORTANT: Commands MUST start with TAB, not spaces!

# DEFAULT TARGET: When you type just 'make', this runs
# This target depends on $(TARGET), so make will build that first
all: $(TARGET)

# RULE TO CREATE FINAL EXECUTABLE
# This says: "To make .bin/tinyserver, I need .obj/tinyserver.o and .bin folder"
# The | means "order-only prerequisite" - create .bin folder if needed, but
# don't rebuild executable just because folder timestamp changed
$(TARGET): $(OBJ) | $(BIN_DIR)
	# This command links the object file into final executable
	# -o means "output file name"
	$(CC) -o $(TARGET) $(OBJ) $(LDFLAGS)

# RULE TO CREATE OBJECT FILE
# This says: "To make .obj/tinyserver.o, I need src/tinyserver.c and .obj folder"
$(OBJ): $(SRC) | $(OBJ_DIR)
	# This command compiles source code into object file
	# -c means "compile only, don't link yet"
	# -o specifies output file name
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

# RULE TO CREATE BIN DIRECTORY
# This has no dependencies (nothing after :)
$(BIN_DIR):
	# mkdir -p creates directory and any missing parent directories
	# Won't error if directory already exists
	mkdir -p $(BIN_DIR)

# RULE TO CREATE OBJ DIRECTORY
$(OBJ_DIR):
	# Same as above but for object files directory
	mkdir -p $(OBJ_DIR)

# CLEAN TARGET: Removes all built files
# Type 'make clean' to run this
clean:
	# rm -rf removes directories and all contents recursively
	# This deletes all build artifacts so you can start fresh
	rm -rf $(BIN_DIR) $(OBJ_DIR)

# =============================================================================
# HOW THIS WORKS WHEN YOU TYPE 'make':
# =============================================================================
# 1. Make looks at 'all' target (first target)
# 2. 'all' depends on $(TARGET) which is .bin/tinyserver
# 3. To make .bin/tinyserver, make needs .obj/tinyserver.o and .bin folder
# 4. To make .obj/tinyserver.o, make needs src/tinyserver.c and .obj folder
# 5. Make creates .obj folder (if needed)
# 6. Make compiles src/tinyserver.c → .obj/tinyserver.o
# 7. Make creates .bin folder (if needed)
# 8. Make links .obj/tinyserver.o → .bin/tinyserver
# 9. Done! Your program is ready in .bin/tinyserver
# =============================================================================