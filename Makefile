# ─────────────────────────────────────────────────────────────────
#  3D Survival Maze – Makefile
#  Targets: linux (default) | macos | windows
# ─────────────────────────────────────────────────────────────────

TARGET  = survival_maze
SRCS    = main.c maze.c player.c enemy.c
OBJS    = $(SRCS:.c=.o)
CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c99

# ── Detect OS automatically ───────────────────────────────────────
UNAME := $(shell uname -s 2>/dev/null || echo Windows)

ifeq ($(UNAME), Linux)
    LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

ifeq ($(UNAME), Darwin)          # macOS
    LDFLAGS = -lraylib -framework OpenGL -framework Cocoa \
              -framework IOKit -framework CoreAudio -framework CoreVideo
endif

ifeq ($(UNAME), Windows)
    TARGET  = survival_maze.exe
    CC      = gcc
    LDFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm
endif

# ── Rules ─────────────────────────────────────────────────────────
.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).exe
