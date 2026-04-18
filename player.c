#include "player.h"
#include "maze.h"
#include <math.h>
#include <stdio.h>

/* ── Global ──────────────────────────────────────────────── */
Player player;

/* ── Init ────────────────────────────────────────────────── */
void PlayerInit(Vector3 startPos)
{
    player.yaw    = 0.0f;
    player.pitch  = 0.0f;
    player.health = PLAYER_MAX_HEALTH;
    player.alive  = true;
    player.iFrames   = 0.0f;
    player.sprinting = false;

    player.camera.position   = startPos;
    player.camera.target     = (Vector3){ startPos.x + 1.0f, startPos.y, startPos.z };
    player.camera.up         = (Vector3){ 0.0f, 1.0f, 0.0f };
    player.camera.fovy       = 75.0f;
    player.camera.projection = CAMERA_PERSPECTIVE;
}

/* ── Update ──────────────────────────────────────────────── */
void PlayerUpdate(float dt)
{
    if (!player.alive) return;

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    /* ── Mouse look (manual centre-reset, works reliably on Windows) ── */
    Vector2 mousePos = GetMousePosition();
    float mdx = mousePos.x - sw / 2.0f;
    float mdy = mousePos.y - sh / 2.0f;
    SetMousePosition(sw / 2, sh / 2);

    player.yaw   += mdx * MOUSE_SENSITIVITY;
    player.pitch -= mdy * MOUSE_SENSITIVITY;

    /* ── Arrow keys: look left/right/up/down ── */
    float arrowSpeed = 2.0f * dt;   /* radians per second */
    if (IsKeyDown(KEY_LEFT))  player.yaw   += arrowSpeed;
    if (IsKeyDown(KEY_RIGHT)) player.yaw   -= arrowSpeed;
    if (IsKeyDown(KEY_UP))    player.pitch += arrowSpeed;
    if (IsKeyDown(KEY_DOWN))  player.pitch -= arrowSpeed;

    if (player.pitch >  1.45f) player.pitch =  1.45f;
    if (player.pitch < -1.45f) player.pitch = -1.45f;

    float sy = sinf(player.yaw),  cy = cosf(player.yaw);
    float cp = cosf(player.pitch), sp = sinf(player.pitch);

    /* ── Keyboard movement ── */
    player.sprinting = IsKeyDown(KEY_LEFT_SHIFT);
    float speed = (player.sprinting ? PLAYER_SPRINT_SPEED : PLAYER_SPEED) * dt;

    /* Forward vector projected onto XZ plane */
    float fwdX = sy, fwdZ = cy;
    /* Right vector (perpendicular to forward in XZ) */
    float rgtX = cy, rgtZ = -sy;

    float moveX = 0.0f, moveZ = 0.0f;
    if (IsKeyDown(KEY_W)) { moveX += fwdX; moveZ += fwdZ; }
    if (IsKeyDown(KEY_S)) { moveX -= fwdX; moveZ -= fwdZ; }
    /* A/D swapped to match expected left/right strafe */
    if (IsKeyDown(KEY_A)) { moveX += rgtX; moveZ += rgtZ; }
    if (IsKeyDown(KEY_D)) { moveX -= rgtX; moveZ -= rgtZ; }

    /* Normalize diagonal movement */
    float len = sqrtf(moveX*moveX + moveZ*moveZ);
    if (len > 0.001f) {
        moveX = (moveX / len) * speed;
        moveZ = (moveZ / len) * speed;
    }

    /* ── Slide collision ── */
    Vector3 p = player.camera.position;

    p.x += moveX;
    if (CheckMazeWallCollision(p, PLAYER_RADIUS)) p.x -= moveX;

    p.z += moveZ;
    if (CheckMazeWallCollision(p, PLAYER_RADIUS)) p.z -= moveZ;

    player.camera.position = p;

    /* ── Update camera target ── */
    player.camera.target = (Vector3){
        p.x + sy * cp,
        p.y + sp,
        p.z + cy * cp
    };

    /* ── Invincibility cooldown ── */
    if (player.iFrames > 0.0f) player.iFrames -= dt;
}

/* ── Damage ──────────────────────────────────────────────── */
void PlayerTakeDamage(float amount)
{
    if (player.iFrames > 0.0f) return;
    player.health -= amount;
    player.iFrames = 1.5f;   /* 1.5 s grace period */
    if (player.health <= 0.0f) {
        player.health = 0.0f;
        player.alive  = false;
    }
}

/* ── HUD ─────────────────────────────────────────────────── */
void PlayerDrawHUD(int totalCoins, int collectedCoins, float timeLeft)
{
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    /* ── Vignette / flashlight effect ── */
    int vw = sw / 3;
    int vh = sh / 4;
    DrawRectangleGradientH(0,      0, vw, sh, Fade(BLACK, 0.75f), Fade(BLACK, 0.0f));
    DrawRectangleGradientH(sw-vw,  0, vw, sh, Fade(BLACK, 0.0f),  Fade(BLACK, 0.75f));
    DrawRectangleGradientV(0,      0, sw, vh, Fade(BLACK, 0.55f), Fade(BLACK, 0.0f));
    DrawRectangleGradientV(0, sh-vh, sw, vh, Fade(BLACK, 0.0f),  Fade(BLACK, 0.55f));

    /* ── Crosshair ── */
    int cx = sw / 2, cy = sh / 2;
    DrawLine(cx - 12, cy,      cx + 12, cy,      Fade(WHITE, 0.8f));
    DrawLine(cx,      cy - 12, cx,      cy + 12, Fade(WHITE, 0.8f));
    DrawCircleLines(cx, cy, 3, Fade(WHITE, 0.5f));

    /* ── Health bar (bottom-left) ── */
    int barW = 220, barH = 22;
    int barX = 20,  barY = sh - 50;
    float hpPct = player.health / PLAYER_MAX_HEALTH;
    Color barCol = (hpPct > 0.5f) ? GREEN : (hpPct > 0.25f) ? YELLOW : RED;

    DrawRectangle(barX, barY, barW, barH, (Color){ 20,20,20,200 });
    DrawRectangle(barX, barY, (int)(barW * hpPct), barH, barCol);
    DrawRectangleLines(barX, barY, barW, barH, WHITE);
    DrawText(TextFormat("HP  %.0f / %.0f", player.health, PLAYER_MAX_HEALTH),
             barX + 5, barY + 4, 14, WHITE);

    /* ── Sprint indicator ── */
    if (player.sprinting)
        DrawText("[ SPRINT ]", barX, barY - 24, 16, SKYBLUE);

    /* ── Coin counter (top-right) ── */
    const char *coinStr = TextFormat("Coins: %d / %d", collectedCoins, totalCoins);
    int coinW = MeasureText(coinStr, 24);
    DrawText(coinStr, sw - coinW - 20, 20, 24, GOLD);

    /* ── Timer (top-centre) ── */
    int mins = (int)timeLeft / 60;
    int secs = (int)timeLeft % 60;
    Color timerCol = (timeLeft > 30.0f) ? WHITE : RED;
    const char *timerStr = TextFormat("  %02d:%02d", mins, secs);
    DrawText(timerStr, sw/2 - MeasureText(timerStr, 30)/2, 16, 30, timerCol);

    /* ── Objective hint (below timer) ── */
    if (collectedCoins >= totalCoins) {
        const char *msg = "EXIT IS OPEN  ->  Reach the GREEN door!";
        DrawText(msg, sw/2 - MeasureText(msg, 20)/2, 58, 20, GREEN);
    } else {
        const char *msg = TextFormat("Collect %d more coin%s!",
                                     totalCoins - collectedCoins,
                                     (totalCoins - collectedCoins == 1) ? "" : "s");
        DrawText(msg, sw/2 - MeasureText(msg, 20)/2, 58, 20, YELLOW);
    }

    /* ── Damage flash ── */
    if (player.iFrames > 0.0f) {
        /* Flicker red overlay */
        if ((int)(player.iFrames * 8) % 2 == 0)
            DrawRectangle(0, 0, sw, sh, Fade(RED, 0.25f));
    }
}
