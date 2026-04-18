#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include <stdbool.h>

/* ── Tuning ──────────────────────────────────────────────── */
#define PLAYER_SPEED         5.0f
#define PLAYER_SPRINT_SPEED  9.5f
#define PLAYER_RADIUS        0.4f
#define PLAYER_HEIGHT        1.7f   /* eye level above ground */
#define PLAYER_MAX_HEALTH  100.0f
#define MOUSE_SENSITIVITY    0.003f

/* ── Data ────────────────────────────────────────────────── */
typedef struct {
    Camera3D camera;
    float    health;
    bool     alive;
    float    iFrames;    /* invincibility timer after a hit   */
    float    yaw;        /* horizontal look angle (radians)   */
    float    pitch;      /* vertical look angle (radians)     */
    bool     sprinting;
} Player;

extern Player player;

/* ── API ─────────────────────────────────────────────────── */
void PlayerInit(Vector3 startPos);
void PlayerUpdate(float dt);
void PlayerTakeDamage(float amount);
void PlayerDrawHUD(int totalCoins, int collectedCoins, float timeLeft);

#endif /* PLAYER_H */
