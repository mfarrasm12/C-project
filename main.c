/*
 * 3D Survival Maze — main.c
 * --------------------------
 * Controls:
 *   WASD        – move
 *   Left Shift  – sprint
 *   Mouse       – look
 *   ESC         – quit (in-game)
 *   Enter       – confirm menu/restart
 */

#include "raylib.h"
#include "maze.h"
#include "player.h"
#include "enemy.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

/* ── Screen & game settings ──────────────────────────────── */
#define SCREEN_W   1280
#define SCREEN_H   720
#define GAME_TIME  180.0f   /* 3 minutes */
#define TARGET_FPS 60

/* ── Game states ─────────────────────────────────────────── */
typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_WIN,
    STATE_GAMEOVER
} GameState;

/* ── Forward decls ───────────────────────────────────────── */
static void ResetGame(void);
static void DrawMenu(void);
static void DrawWin(int collected, int total, float elapsed);
static void DrawGameOver(bool outOfTime);
static void DrawExitDoor(bool open);

/* ── Main ────────────────────────────────────────────────── */
int main(void)
{
    srand((unsigned)time(NULL));

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(SCREEN_W, SCREEN_H, "3D Survival Maze");
    SetTargetFPS(TARGET_FPS);

    /* We manage the cursor ourselves */
    DisableCursor();

    /* ── Initial game setup ── */
    GenerateMaze();
    PlaceCoins();
    PlayerInit(MazeStartPos());
    EnemyInit(MazeEnemyStartPos());

    GameState state          = STATE_MENU;
    float     timeLeft       = GAME_TIME;
    int       collectedCoins = 0;
    float     elapsed        = 0.0f;    /* used for win screen */

    /* ── Main loop ── */
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        if (dt > 0.05f) dt = 0.05f;    /* cap delta to avoid tunnelling */

        /* ════════════════════════════════════════════════════
         * STATE LOGIC
         * ════════════════════════════════════════════════════ */
        switch (state)
        {
            /* ── Menu ── */
            case STATE_MENU:
                EnableCursor();
                if (IsKeyPressed(KEY_ENTER)) {
                    DisableCursor();
                    ResetGame();
                    collectedCoins = 0;
                    timeLeft       = GAME_TIME;
                    elapsed        = 0.0f;
                    state          = STATE_PLAYING;
                }
                break;

            /* ── Playing ── */
            case STATE_PLAYING:
                DisableCursor();
                timeLeft -= dt;
                elapsed  += dt;

                PlayerUpdate(dt);
                EnemyUpdate(dt, player.camera.position);

                /* Coin collection */
                collectedCoins += CollectCoins(player.camera.position);

                /* Win condition: all coins AND reach exit */
                if (collectedCoins >= coinCount &&
                    CheckExitReached(player.camera.position))
                {
                    state = STATE_WIN;
                    EnableCursor();
                }

                /* Lose conditions */
                if (!player.alive) {
                    state = STATE_GAMEOVER;
                    EnableCursor();
                }
                if (timeLeft <= 0.0f) {
                    timeLeft = 0.0f;
                    state    = STATE_GAMEOVER;
                    EnableCursor();
                }
                break;

            /* ── Win / Game Over ── */
            case STATE_WIN:
            case STATE_GAMEOVER:
                if (IsKeyPressed(KEY_ENTER)) {
                    state = STATE_MENU;
                }
                break;
        }

        /* ════════════════════════════════════════════════════
         * RENDERING
         * ════════════════════════════════════════════════════ */
        BeginDrawing();
        ClearBackground((Color){ 8, 5, 12, 255 });   /* near-black sky */

        /* ── 3D scene (only while playing) ── */
        if (state == STATE_PLAYING)
        {
            BeginMode3D(player.camera);

                /* Draw world */
                DrawMaze();
                DrawCoins();

                /* Exit door — glows green when unlocked */
                DrawExitDoor(collectedCoins >= coinCount);

                /* Enemy */
                EnemyDraw();

                /* Tiny flashlight sphere near player's hand */
                Vector3 handPos = {
                    player.camera.position.x + sinf(player.yaw) * 0.6f,
                    player.camera.position.y - 0.35f,
                    player.camera.position.z + cosf(player.yaw) * 0.6f
                };
                DrawSphere(handPos, 0.06f, (Color){ 255, 240, 180, 220 });

            EndMode3D();

            /* 2-D HUD on top */
            PlayerDrawHUD(coinCount, collectedCoins, timeLeft);
        }

        /* ── 2-D overlays ── */
        if (state == STATE_MENU)
            DrawMenu();

        if (state == STATE_WIN)
            DrawWin(collectedCoins, coinCount, elapsed);

        if (state == STATE_GAMEOVER)
            DrawGameOver(timeLeft <= 0.0f && player.alive);

        /* FPS counter (small, top-left corner) */
        if (state == STATE_PLAYING)
            DrawFPS(8, 8);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

/* ── Helpers ─────────────────────────────────────────────── */
static void ResetGame(void)
{
    GenerateMaze();
    PlaceCoins();
    PlayerInit(MazeStartPos());
    EnemyInit(MazeEnemyStartPos());
}

/* Draw the exit door — locked (red) or open (green) */
static void DrawExitDoor(bool open)
{
    Vector3 ep = MazeExitPos();

    Color doorCol  = open ? (Color){ 30, 220, 60,  255 } : (Color){ 180, 30, 30, 255 };
    Color wireCol  = open ? (Color){ 0,  140, 30,  255 } : (Color){ 100,  0,  0, 255 };
    Color glowCol  = open ? (Color){ 80, 255, 100,  90 } : (Color){ 200, 50, 50,  80 };

    /* Door frame */
    DrawCube(ep, 1.6f, WALL_HEIGHT, 0.25f, doorCol);
    DrawCubeWires(ep, 1.6f, WALL_HEIGHT, 0.25f, wireCol);

    /* Pulsing glow sphere if open */
    if (open) {
        float pulse = 0.18f + sinf((float)GetTime() * 4.0f) * 0.06f;
        DrawSphere(ep, pulse, glowCol);
    }

    /* Small "EXIT" label floated above door */
    /* (DrawText in 3-D world is not natively supported in raylib;
       we use a 2-D DrawText in world-to-screen space instead)    */
    Vector2 screenPos = GetWorldToScreen(
        (Vector3){ ep.x, ep.y + WALL_HEIGHT * 0.5f + 0.4f, ep.z },
        player.camera);
    if (screenPos.x > 0 && screenPos.x < GetScreenWidth() &&
        screenPos.y > 0 && screenPos.y < GetScreenHeight())
    {
        const char *label = open ? "EXIT" : "LOCKED";
        Color labelCol    = open ? GREEN : RED;
        int fs = 20;
        DrawText(label,
                 (int)screenPos.x - MeasureText(label, fs) / 2,
                 (int)screenPos.y,
                 fs, labelCol);
    }
}

/* Animated title flicker helper */
static bool titleFlicker(void)
{
    return ((int)(GetTime() * 2.0) % 2 == 0);
}

static void DrawMenu(void)
{
    int sw = GetScreenWidth(), sh = GetScreenHeight();

    /* Dark overlay */
    DrawRectangle(0, 0, sw, sh, (Color){ 0, 0, 0, 210 });

    /* Title */
    const char *title = "3D SURVIVAL MAZE";
    int titleSize = 64;
    Color titleCol = titleFlicker() ? DARKGREEN : GREEN;
    DrawText(title, sw/2 - MeasureText(title, titleSize)/2, 140, titleSize, titleCol);

    /* Subtitle */
    const char *sub = "Collect all coins  ~  Reach the exit  ~  Survive";
    DrawText(sub, sw/2 - MeasureText(sub, 22)/2, 225, 22, LIGHTGRAY);

    /* Controls box */
    int bx = sw/2 - 260, by = 290, bw = 520, bh = 170;
    DrawRectangle(bx, by, bw, bh, (Color){ 20, 20, 20, 200 });
    DrawRectangleLines(bx, by, bw, bh, DARKGRAY);

    int fy = by + 14;
    DrawText("WASD              Move",            bx+20, fy, 20, LIGHTGRAY); fy += 28;
    DrawText("Arrow Keys        Look around",     bx+20, fy, 20, LIGHTGRAY); fy += 28;
    DrawText("Mouse             Look around",     bx+20, fy, 20, LIGHTGRAY); fy += 28;
    DrawText("Left Shift        Sprint",          bx+20, fy, 20, LIGHTGRAY); fy += 28;
    DrawText("Avoid the RED enemy cube!",         bx+20, fy, 20, RED);

    /* Press Enter prompt */
    if (titleFlicker()) {
        const char *prompt = "Press  ENTER  to Start";
        DrawText(prompt, sw/2 - MeasureText(prompt, 32)/2, 510, 32, WHITE);
    }

    /* Tip */
    DrawText("Tip: Collect ALL coins before the exit opens.",
             sw/2 - MeasureText("Tip: Collect ALL coins before the exit opens.", 18)/2,
             580, 18, GRAY);
}

static void DrawWin(int collected, int total, float elapsed)
{
    int sw = GetScreenWidth(), sh = GetScreenHeight();

    DrawRectangle(0, 0, sw, sh, (Color){ 0, 30, 10, 200 });

    const char *t1 = "YOU ESCAPED!";
    DrawText(t1, sw/2 - MeasureText(t1, 80)/2, 180, 80, YELLOW);

    DrawText(TextFormat("Coins collected : %d / %d", collected, total),
             sw/2 - 160, 300, 28, WHITE);
    DrawText(TextFormat("Time taken      : %02d:%02d",
                        (int)elapsed / 60, (int)elapsed % 60),
             sw/2 - 160, 340, 28, WHITE);

    const char *prompt = "Press ENTER to play again";
    DrawText(prompt, sw/2 - MeasureText(prompt, 28)/2, 440, 28, LIGHTGRAY);
}

static void DrawGameOver(bool outOfTime)
{
    int sw = GetScreenWidth(), sh = GetScreenHeight();

    DrawRectangle(0, 0, sw, sh, (Color){ 40, 0, 0, 200 });

    const char *t1 = "GAME  OVER";
    DrawText(t1, sw/2 - MeasureText(t1, 80)/2, 180, 80, RED);

    const char *reason = outOfTime ? "The timer ran out!"
                                   : "The enemy got you!";
    DrawText(reason, sw/2 - MeasureText(reason, 30)/2, 300, 30, LIGHTGRAY);

    const char *prompt = "Press ENTER to try again";
    DrawText(prompt, sw/2 - MeasureText(prompt, 28)/2, 400, 28, WHITE);
}
