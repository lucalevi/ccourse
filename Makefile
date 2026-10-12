# Builds the ccourse tool. The exercises are not built here; ccourse compiles
# those itself while it runs.
#
#   make             build build/ccourse
#   make verify      check every exercise and solution (for course authors)
#   make install     copy ccourse to $(PREFIX)/bin   (default: ~/.local/bin)
#   make uninstall   remove it again
#   make clean       delete build/

CC ?= cc
CFLAGS ?= -O2 -g
PREFIX ?= $(HOME)/.local

# The tool is held to the same standard as the exercises: C17, and it must
# build without a single warning.
# _DEFAULT_SOURCE makes glibc declare the POSIX functions under -std=c17.
STRICT = -std=c17 -Wall -Wextra -Wpedantic -D_DEFAULT_SOURCE

BUILD = build
BIN = $(BUILD)/ccourse
SOURCES = $(wildcard src/*.c)
OBJECTS = $(SOURCES:src/%.c=$(BUILD)/%.o)

.PHONY: all verify install uninstall clean

all: $(BIN)

$(BIN): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

# -MMD -MP record which headers each file includes, so that changing a header
# rebuilds what depends on it.
$(BUILD)/%.o: src/%.c
	@mkdir -p $(BUILD)
	$(CC) $(STRICT) $(CFLAGS) -MMD -MP -c $< -o $@

verify: $(BIN)
	$(BIN) verify

install: $(BIN)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $(BIN) $(DESTDIR)$(PREFIX)/bin/ccourse

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/ccourse

clean:
	rm -rf $(BUILD)

-include $(OBJECTS:.o=.d)
