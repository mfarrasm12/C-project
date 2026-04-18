#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include <stdbool.h>

/* ── Tuning ──────────────────────────────────────────────── */
#define ENEMY_SPEED          2.4f
#define ENEMY_RADIUS         0.45f
#define ENEMY_DAMAGE         25.0f
#define ENEMY_ATTACK_RANGE   1.3f
#define ENEMY_ATTACK_CD      1.5f   /* seconds between attacks  */
#define ENEMY_WIDTH          0.90f
#define ENEMY_HEIGHT         1.40f

/* ── Data ────────────────────────────────────────────────── */
typedef struct {
    Vector3 pos;
    bool    active;
    float   attackCooldown;
    float   angle;          /* facing direction for cosmetics */
} Enemy;

extern Enemy enemy;

/* ── API ─────────────────────────────────────────────────── */
void EnemyInit(Vector3 startPos);
void EnemyUpdate(float dt, Vector3 playerPos);
void EnemyDraw(void);

#endif /* ENEMY_H */
