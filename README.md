# 3D Survival Maze

A first-person 3D survival game built with **raylib** in C.

---

## Gameplay

1. **Explore** a randomly generated maze.
2. **Collect all coins** (gold spheres that bob and glow).
3. **Reach the EXIT door** (turns green once all coins are collected).
4. **Avoid the red enemy cube** — it hunts you down.
5. Beat the **3-minute timer** to survive.

### Controls

| Key / Input    | Action              |
|----------------|---------------------|
| `W A S D`      | Move                |
| `Left Shift`   | Sprint              |
| Mouse          | Look around         |
| `ESC`          | Quit (in-game)      |
| `Enter`        | Confirm / Restart   |

---

## Features

| Feature               | Implementation                              |
|-----------------------|---------------------------------------------|
| First-person camera   | Manual yaw/pitch with mouse delta           |
| Procedural maze       | Recursive-backtracking DFS on a grid        |
| Sliding collision     | Circle vs AABB per wall cell                |
| Enemy AI              | Direct-chase with wall sliding              |
| Health + i-frames     | 25 HP/hit, 1.5 s grace period               |
| Sprint mechanic       | Speed ×1.9 while Left Shift held            |
| Countdown timer       | 3 minutes, turns red at 30 s                |
| Vignette effect       | Gradient overlays simulate a flashlight     |
| Multiple game states  | Menu → Playing → Win / Game Over            |

---

## Building

### Prerequisites

Install **raylib** for your platform:

- **Linux (Ubuntu/Debian)**
  ```bash
  sudo apt install libraylib-dev
  # or build from source: https://github.com/raysan5/raylib
  ```

- **macOS (Homebrew)**
  ```bash
  brew install raylib
  ```

- **Windows**  
  Download the pre-built binaries from https://github.com/raysan5/raylib/releases  
  and add them to your compiler path.

### Compile

```bash
make          # auto-detects Linux / macOS / Windows
make run      # compile + launch immediately
make clean    # remove build artefacts
```

**Manual compile (Linux):**
```bash
gcc -Wall -O2 -std=c99 main.c maze.c player.c enemy.c \
    -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 \
    -o survival_maze
```

**Manual compile (macOS):**
```bash
gcc -Wall -O2 -std=c99 main.c maze.c player.c enemy.c \
    -lraylib -framework OpenGL -framework Cocoa \
    -framework IOKit -framework CoreAudio -framework CoreVideo \
    -o survival_maze
```

**Manual compile (Windows / MinGW):**
```bash
gcc -Wall -O2 -std=c99 main.c maze.c player.c enemy.c \
    -lraylib -lopengl32 -lgdi32 -lwinmm \
    -o survival_maze.exe
```

---

## File Structure

```
survival_maze/
├── main.c      – Game loop, states (Menu/Playing/Win/GameOver)
├── maze.c/h    – Maze generation, drawing, coin placement, collision
├── player.c/h  – First-person camera, movement, health, HUD
├── enemy.c/h   – Enemy AI, attack, 3-D drawing
├── Makefile    – Cross-platform build rules
└── README.md   – This file
```

---

## Extending the Project

| Idea                  | Where to start                             |
|-----------------------|--------------------------------------------|
| Multiple enemy types  | Add a second `Enemy` struct with new speed |
| Sound effects         | `InitAudioDevice()`, `LoadSound()` in main |
| Textures on walls     | `LoadTexture()` + `DrawCubeTexture()`      |
| Minimap               | Read `grid[][]` and draw coloured rects    |
| Levels / difficulty   | Increase `MAZE_SIZE` or `ENEMY_SPEED`      |
| Shader flashlight     | Load a GLSL shader and use `SetShaderValue`|
| Pathfinding enemy     | Implement BFS/A* over the grid             |
