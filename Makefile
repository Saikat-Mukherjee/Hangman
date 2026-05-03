# ─────────────────────────────────────────────────────────────────────────
# Hangman — Makefile  (MinGW-W64 GCC on Windows)
#
# Prerequisites: see SDL2_SETUP.txt
#   SDL2 headers  → include/SDL2/
#   SDL2 libs     → lib/  (libSDL2.a, libSDL2main.a, libSDL2_ttf.a)
#   SDL2 DLLs     → project root (SDL2.dll, SDL2_ttf.dll)
#   Font          → assets/fonts/Roboto-Regular.ttf
# ─────────────────────────────────────────────────────────────────────────

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -O2 -I./src -I./include
LDFLAGS = -L./lib -lmingw32 -lSDL3 -lSDL3_ttf -mwindows

SRCDIR  = src
OBJDIR  = obj
TARGET  = hangman.exe

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

# ── Targets ───────────────────────────────────────────────────────────────

all: $(OBJDIR) $(TARGET)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo ""
	@echo "  Build successful: $(TARGET)"
	@echo "  Make sure SDL2.dll and SDL2_ttf.dll are in the project root."
	@echo ""

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)

run: all
	./$(TARGET)

.PHONY: all clean run
