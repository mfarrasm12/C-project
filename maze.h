#ifndef MAZE_H
#define MAZE_H

#include "raylib.h"
#include <stdbool.h>

/* ── Maze dimensions ─────────────────────────────────────── */
#define MAZE_SIZE   10                   /* rooms per axis      */
#define GRID_W     (2 * MAZE_SIZE + 1)   /* full grid width     */
#define GRID_H     (2 * MAZE_SIZE + 1)   /* full grid height    */
#define CELL_SIZE   3.5f                 /* world units / cell  */
#define WALL_HEIGHT 3.2f                 /* wall cube height    */
#define MAX_COINS   15

/* ── Coin ────────────────────────────────────────────────── */
typedef struct {
    Vector3 pos;
    bool    collected;
} Coin;

/* ── Globals (defined in maze.c) ─────────────────────────── */
extern int  grid[GRID_H][GRID_W];   /* 1 = wall, 0 = floor */
extern Coin coins[MAX_COINS];
extern int  coinCount;

/* ── API ─────────────────────────────────────────────────── */
void    GenerateMaze(void);
void    DrawMaze(void);
bool    CheckMazeWallCollision(Vector3 pos, float radius);

void    PlaceCoins(void);
void    DrawCoins(void);
int     CollectCoins(Vector3 playerPos);   /* returns # newly collected */

Vector3 MazeStartPos(void);
Vector3 MazeExitPos(void);
Vector3 MazeEnemyStartPos(void);
bool    CheckExitReached(Vector3 playerPos);

#endif /* MAZE_H */
