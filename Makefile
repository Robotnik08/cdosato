CC = gcc
CFLAGS = -Iinclude -Iinclude/standard_libraries -Werror
SRCDIR = src
INCDIR = include
BUILDDIR = build
TEMPDIR = temp
TARGET = $(BUILDDIR)/dosato
LIB_TARGET = $(BUILDDIR)/dosato_lib

SOURCES := $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/standard_libraries/*.c) main.c
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(TEMPDIR)/%.o,$(patsubst $(SRCDIR)/standard_libraries/%.c,$(TEMPDIR)/standard_libraries/%.o,$(SOURCES:main.c=$(TEMPDIR)/main.o)))

all: $(BUILDDIR) $(TEMPDIR) $(TARGET) lib

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

lib: $(LIB_TARGET)

ifeq ($(OS),Windows_NT)
$(LIB_TARGET): $(OBJECTS)
	$(CC) -shared -o $(LIB_TARGET).dll $^
else
$(LIB_TARGET): $(OBJECTS)
	$(CC) -shared -o $(LIB_TARGET).so $^
endif

$(TEMPDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TEMPDIR)/standard_libraries/%.o: $(SRCDIR)/standard_libraries/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TEMPDIR)/main.o: main.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILDDIR):
ifeq ($(OS),Windows_NT)
	if not exist $(BUILDDIR) mkdir $(BUILDDIR)
else
	mkdir -p $(BUILDDIR)
endif

$(TEMPDIR):
ifeq ($(OS),Windows_NT)
	if not exist $(TEMPDIR) mkdir $(TEMPDIR)
	if not exist $(TEMPDIR)\standard_libraries mkdir $(TEMPDIR)\standard_libraries
else
	mkdir -p $(TEMPDIR)
	mkdir -p $(TEMPDIR)/standard_libraries
endif

clean:
ifeq ($(OS),Windows_NT)
	rmdir /s /q $(TEMPDIR)
else
	rm -rf $(TEMPDIR)
endif

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run lib
