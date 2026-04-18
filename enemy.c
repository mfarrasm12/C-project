#include "enemy.h"
#include "player.h"
#include "maze.h"
#include <math.h>

/* ── Global ──────────────────────────────────────────────── */
Enemy enemy;

/* ── Init ────────────────────────────────────────────────── */
void EnemyInit(Vector3 startPos)
{
    enemy.pos            = startPos;
    enemy.pos.y          = 0.0f;   /* sit on the floor */
    enemy.active         = true;
    enemy.attackCooldown = 0.0f;
    enemy.angle          = 0.0f;
}

/* ── Update ──────────────────────────────────────────────── */
void EnemyUpdate(float dt, Vector3 playerPos)
{
    if (!enemy.active) return;

    /* Vector toward player (XZ plane only) */
    float dx = playerPos.x - enemy.pos.x;
    float dz = playerPos.z - enemy.pos.z;
    float dist = sqrtf(dx*dx + dz*dz);

    if (dist < 0.05f) return;

    /* Smooth-chase the facing angle for cosmetics */
    float targetAngle = atan2f(dx, dz);
    enemy.angle = targetAngle;   /* snap; you can lerp if preferred */

    /* ── Movement with sliding wall collision ── */
    float speed = ENEMY_SPEED * dt;
    float nx = (dx / dist) * speed;
    float nz = (dz / dist) * speed;

    Vector3 p = enemy.pos;

    p.x += nx;
    if (CheckMazeWallCollision(p, ENEMY_RADIUS)) {
        p.x -= nx;
        /* Try sliding along Z only */
        p.z += nz * 1.5f;
        if (CheckMazeWallCollision(p, ENEMY_RADIUS)) p.z -= nz * 1.5f;
    } else {
        p.z += nz;
        if (CheckMazeWallCollision(p, ENEMY_RADIUS)) p.z -= nz;
    }

    enemy.pos = p;

    /* ── Attack ── */
    if (enemy.attackCooldown > 0.0f) enemy.attackCooldown -= dt;

    if (dist < ENEMY_ATTACK_RANGE && enemy.attackCooldown <= 0.0f) {
        PlayerTakeDamage(ENEMY_DAMAGE);
        enemy.attackCooldown = ENEMY_ATTACK_CD;
    }
}

/* ── Draw ────────────────────────────────────────────────── */
void EnemyDraw(void)
{
    if (!enemy.active) return;

    float t = (float)GetTime();

    /* Body — red cube, centred vertically */
    Vector3 bodyPos = {
        enemy.pos.x,
        enemy.pos.y + ENEMY_HEIGHT * 0.5f,
        enemy.pos.z
    };
    DrawCube(bodyPos, ENEMY_WIDTH, ENEMY_HEIGHT, ENEMY_WIDTH,
             (Color){ 200, 30, 30, 255 });
    DrawCubeWires(bodyPos, ENEMY_WIDTH, ENEMY_HEIGHT, ENEMY_WIDTH,
                  (Color){ 120, 0, 0, 255 });

    /* Head — slightly smaller dark-red cube on top */
    float headY = enemy.pos.y + ENEMY_HEIGHT + 0.25f;
    Vector3 headPos = { enemy.pos.x, headY, enemy.pos.z };
    DrawCube(headPos, 0.60f, 0.50f, 0.60f, (Color){ 160, 20, 20, 255 });

    /* Glowing eyes — face in direction of travel */
    float sinA = sinf(enemy.angle), cosA = cosf(enemy.angle);
    float eyeOffX = 0.12f, eyeOffZ = -0.28f;

    Vector3 eyeL = {
        headPos.x + (-eyeOffX * cosA - eyeOffZ * sinA),
        headPos.y + 0.05f,
        headPos.z + (-eyeOffX * (-sinA) + eyeOffZ * cosA)
    };
    Vector3 eyeR = {
        headPos.x + ( eyeOffX * cosA - eyeOffZ * sinA),
        headPos.y + 0.05f,
        headPos.z + ( eyeOffX * (-sinA) + eyeOffZ * cosA)
    };

    /* Pulsing glow */
    unsigned char glow = (unsigned char)(180 + (int)(sinf(t * 8.0f) * 60.0f));
    Color eyeCol = { glow, glow, 0, 255 };
    DrawSphere(eyeL, 0.07f, eyeCol);
    DrawSphere(eyeR, 0.07f, eyeCol);

    /* Ground shadow ring */
    DrawCircle3D(enemy.pos, ENEMY_WIDTH * 0.7f,
                 (Vector3){1,0,0}, 90.0f,
                 (Color){ 0, 0, 0, 120 });
}
