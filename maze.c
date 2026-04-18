#include "maze.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ── Globals ─────────────────────────────────────────────── */
int  grid[GRID_H][GRID_W];
Coin coins[MAX_COINS];
int  coinCount = 0;

static bool visited[MAZE_SIZE][MAZE_SIZE];

/* ── Maze generation (recursive backtracking DFS) ────────── */
/*
 * Room cells live at odd grid positions: (1+2r, 1+2c).
 * The wall between room (r,c) and room (r+dr, c+dc) is at
 *   grid position (1+2r+dr, 1+2c+dc).
 */
static void carve(int row, int col)
{
    visited[row][col] = true;

    /* directions: N S E W */
    int dirs[4][2] = { {-1,0}, {1,0}, {0,1}, {0,-1} };

    /* Fisher-Yates shuffle */
    for (int i = 3; i > 0; i--) {
        int j = rand() % (i + 1);
        int t0 = dirs[i][0], t1 = dirs[i][1];
        dirs[i][0] = dirs[j][0];  dirs[i][1] = dirs[j][1];
        dirs[j][0] = t0;          dirs[j][1] = t1;
    }

    for (int d = 0; d < 4; d++) {
        int nr = row + dirs[d][0];
        int nc = col + dirs[d][1];
        if (nr < 0 || nr >= MAZE_SIZE || nc < 0 || nc >= MAZE_SIZE) continue;
        if (visited[nr][nc]) continue;

        /* Knock down the wall between current cell and neighbor */
        int wr = 1 + 2*row + dirs[d][0];
        int wc = 1 + 2*col + dirs[d][1];
        grid[wr][wc] = 0;

        carve(nr, nc);
    }
}

void GenerateMaze(void)
{
    /* Fill everything as wall */
    for (int r = 0; r < GRID_H; r++)
        for (int c = 0; c < GRID_W; c++)
            grid[r][c] = 1;

    /* Open room cells */
    for (int r = 0; r < MAZE_SIZE; r++)
        for (int c = 0; c < MAZE_SIZE; c++)
            grid[1 + 2*r][1 + 2*c] = 0;

    memset(visited, 0, sizeof(visited));
    carve(0, 0);
}

/* ── Drawing ─────────────────────────────────────────────── */
void DrawMaze(void)
{
    /* Floor plane */
    float cx = (GRID_W * CELL_SIZE) / 2.0f;
    float cz = (GRID_H * CELL_SIZE) / 2.0f;
    DrawPlane((Vector3){ cx, 0.0f, cz },
              (Vector2){ GRID_W * CELL_SIZE, GRID_H * CELL_SIZE },
              (Color){ 45, 38, 32, 255 });

    /* Wall cubes */
    for (int r = 0; r < GRID_H; r++) {
        for (int c = 0; c < GRID_W; c++) {
            if (grid[r][c] != 1) continue;
            Vector3 pos = {
                c * CELL_SIZE + CELL_SIZE * 0.5f,
                WALL_HEIGHT  * 0.5f,
                r * CELL_SIZE + CELL_SIZE * 0.5f
            };
            /* Slightly vary color per block for a hand-placed look */
            unsigned char v = (unsigned char)(55 + ((r * 7 + c * 3) % 20));
            Color wallCol = { v, (unsigned char)(v - 10), (unsigned char)(v - 20), 255 };
            DrawCube(pos, CELL_SIZE, WALL_HEIGHT, CELL_SIZE, wallCol);
            DrawCubeWires(pos, CELL_SIZE, WALL_HEIGHT, CELL_SIZE,
                          (Color){ 30, 20, 15, 180 });
        }
    }
}

/* ── Collision (circle vs AABB per wall cell) ────────────── */
bool CheckMazeWallCollision(Vector3 pos, float radius)
{
    int c0 = (int)((pos.x - radius) / CELL_SIZE);
    int c1 = (int)((pos.x + radius) / CELL_SIZE);
    int r0 = (int)((pos.z - radius) / CELL_SIZE);
    int r1 = (int)((pos.z + radius) / CELL_SIZE);

    if (c0 < 0)        c0 = 0;
    if (c1 >= GRID_W)  c1 = GRID_W - 1;
    if (r0 < 0)        r0 = 0;
    if (r1 >= GRID_H)  r1 = GRID_H - 1;

    for (int r = r0; r <= r1; r++) {
        for (int c = c0; c <= c1; c++) {
            if (grid[r][c] != 1) continue;

            /* Closest point in AABB to circle centre */
            float bx0 = c * CELL_SIZE,  bx1 = bx0 + CELL_SIZE;
            float bz0 = r * CELL_SIZE,  bz1 = bz0 + CELL_SIZE;

            float nearX = pos.x < bx0 ? bx0 : (pos.x > bx1 ? bx1 : pos.x);
            float nearZ = pos.z < bz0 ? bz0 : (pos.z > bz1 ? bz1 : pos.z);

            float dx = pos.x - nearX;
            float dz = pos.z - nearZ;
            if (dx*dx + dz*dz < radius*radius) return true;
        }
    }
    return false;
}

/* ── Coins ───────────────────────────────────────────────── */
void PlaceCoins(void)
{
    coinCount = 0;

    /* Collect candidate rooms (skip start and exit rooms) */
    int open[MAZE_SIZE * MAZE_SIZE][2];
    int openN = 0;
    for (int r = 0; r < MAZE_SIZE; r++) {
        for (int c = 0; c < MAZE_SIZE; c++) {
            if (r == 0 && c == 0)                           continue; /* start */
            if (r == MAZE_SIZE-1 && c == MAZE_SIZE-1)       continue; /* exit  */
            open[openN][0] = r;
            open[openN][1] = c;
            openN++;
        }
    }

    /* Shuffle candidates */
    for (int i = openN - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tr = open[i][0], tc = open[i][1];
        open[i][0] = open[j][0];  open[i][1] = open[j][1];
        open[j][0] = tr;          open[j][1] = tc;
    }

    coinCount = (MAX_COINS < openN) ? MAX_COINS : openN;
    for (int i = 0; i < coinCount; i++) {
        int gr = 1 + 2 * open[i][0];
        int gc = 1 + 2 * open[i][1];
        coins[i].pos = (Vector3){
            gc * CELL_SIZE + CELL_SIZE * 0.5f,
            1.05f,
            gr * CELL_SIZE + CELL_SIZE * 0.5f
        };
        coins[i].collected = false;
    }
}

void DrawCoins(void)
{
    float t = (float)GetTime();
    for (int i = 0; i < coinCount; i++) {
        if (coins[i].collected) continue;

        /* Bobbing animation */
        float bob = sinf(t * 3.0f + (float)i) * 0.12f;
        Vector3 p = { coins[i].pos.x, coins[i].pos.y + bob, coins[i].pos.z };

        DrawSphere(p, 0.22f, GOLD);
        DrawSphereWires(p, 0.22f, 6, 6, (Color){ 180, 120, 0, 180 });

        /* Small glow ring */
        DrawCircle3D(p, 0.35f, (Vector3){1,0,0}, 90.0f,
                     (Color){ 255, 200, 50, 80 });
    }
}

int CollectCoins(Vector3 playerPos)
{
    int n = 0;
    for (int i = 0; i < coinCount; i++) {
        if (coins[i].collected) continue;
        float dx = playerPos.x - coins[i].pos.x;
        float dz = playerPos.z - coins[i].pos.z;
        if (dx*dx + dz*dz < 1.4f * 1.4f) {
            coins[i].collected = true;
            n++;
        }
    }
    return n;
}

/* ── Key world positions ─────────────────────────────────── */
Vector3 MazeStartPos(void)
{
    /* Room (0,0) → grid cell (1,1) */
    return (Vector3){
        1 * CELL_SIZE + CELL_SIZE * 0.5f,
        1.7f,   /* eye height */
        1 * CELL_SIZE + CELL_SIZE * 0.5f
    };
}

Vector3 MazeExitPos(void)
{
    int gr = 1 + 2 * (MAZE_SIZE - 1);
    int gc = 1 + 2 * (MAZE_SIZE - 1);
    return (Vector3){
        gc * CELL_SIZE + CELL_SIZE * 0.5f,
        WALL_HEIGHT * 0.5f,
        gr * CELL_SIZE + CELL_SIZE * 0.5f
    };
}

/* Enemy spawns in the opposite corner from the player */
Vector3 MazeEnemyStartPos(void)
{
    int gr = 1 + 2 * (MAZE_SIZE - 1);  /* far row  */
    int gc = 1;                          /* near col */
    return (Vector3){
        gc * CELL_SIZE + CELL_SIZE * 0.5f,
        0.5f,
        gr * CELL_SIZE + CELL_SIZE * 0.5f
    };
}

bool CheckExitReached(Vector3 playerPos)
{
    Vector3 e = MazeExitPos();
    float dx = playerPos.x - e.x;
    float dz = playerPos.z - e.z;
    return dx*dx + dz*dz < 2.2f * 2.2f;
}
