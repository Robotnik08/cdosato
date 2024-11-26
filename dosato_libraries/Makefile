# Makefile for building a DLL
CC = gcc
CFLAGS = -shared -I$(HEADER_PATH)
LDFLAGS = -L$(DOSATO_LIB_PATH) -llibdosato
BUILD_DIR = build
TMP_DIR = temp
LIB_NAME = $(LIB_NAME)
OBJ_FILE = $(TMP_DIR)/$(LIB_NAME).o
DLL_FILE = $(BUILD_DIR)/$(LIB_NAME).dll

# Detect operating system
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    MKDIR = mkdir -p
    RM = rm -rf
else
    MKDIR = if not exist $1 mkdir $1
    RM = del /Q
endif

# Targets
all: $(DLL_FILE)

$(DLL_FILE): $(OBJ_FILE) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(OBJ_FILE): $(LIB_NAME).c $(LIB_NAME).h | $(TMP_DIR)
	$(CC) -c -o $@ -I$(HEADER_PATH) $(LIB_NAME).c

$(BUILD_DIR) $(TMP_DIR):
	$(call MKDIR,$(BUILD_DIR))
	$(call MKDIR,$(TMP_DIR))

clean:
	$(RM) $(TMP_DIR)

.PHONY: clean
