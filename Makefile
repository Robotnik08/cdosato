CC = gcc
CFLAGS = -Iinclude -Iinclude/standard_libraries -Werror -Wno-format -fPIC
SRCDIR = src
INCDIR = include
BUILDDIR = build
TEMPDIR = temp
TARGET = $(BUILDDIR)/dosato
LIB_TARGET = $(BUILDDIR)/libdosato

ifeq ($(OS),Windows_NT)
CURRENT_DATE := $(shell powershell -Command "Get-Date -Format 'dd/MM/yyyy'")
else
CURRENT_DATE := $(shell date +"%d/%m/%Y")
endif
CFLAGS += -DDOSATO_DATE="\"$(CURRENT_DATE)\""

SOURCES := $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/standard_libraries/*.c) main.c
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(TEMPDIR)/%.o,$(patsubst $(SRCDIR)/standard_libraries/%.c,$(TEMPDIR)/standard_libraries/%.o,$(SOURCES:main.c=$(TEMPDIR)/main.o)))

all: $(BUILDDIR) $(TEMPDIR) $(TARGET) lib

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

lib: $(LIB_TARGET)

ifeq ($(OS),Windows_NT)
$(LIB_TARGET): $(OBJECTS)
	$(CC) -shared -o $(LIB_TARGET).dll $^ -lm
else
$(LIB_TARGET): $(OBJECTS)
	$(CC) -shared -o $(LIB_TARGET).so $^ -lm
endif

$(TEMPDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $< -lm

$(TEMPDIR)/standard_libraries/%.o: $(SRCDIR)/standard_libraries/%.c
	$(CC) $(CFLAGS) -c -o $@ $< -lm

$(TEMPDIR)/main.o: main.c
	$(CC) $(CFLAGS) -c -o $@ $< -lm

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
