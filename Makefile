CC = gcc
CFLAGS = -Iinclude -Werror
SRCDIR = src
INCDIR = include
BUILDDIR = build
TARGET = main

SOURCES := $(wildcard $(SRCDIR)/*.c) main.c
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))

all: $(BUILDDIR) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILDDIR):
ifeq ($(OS),Windows_NT)
	if not exist $(BUILDDIR) mkdir $(BUILDDIR)
else
	mkdir -p $(BUILDDIR)
endif

clean:
ifeq ($(OS),Windows_NT)
	rmdir /s /q $(BUILDDIR)
else
	rm -rf $(BUILDDIR)
endif

run:
	./$(TARGET)

.PHONY: all clean run
