#include <stdio.h>
#include <raylib.h>
#include <string.h>
#include <math.h>

#define WIDTH 900
#define HEIGHT 600
#define MAX_ENTRIES 10

Color colors[] = {
    DARKBLUE, RED, GREEN, YELLOW,
    LIGHTGRAY, BROWN, ORANGE, PURPLE,
    PINK, SKYBLUE
};

typedef struct {
    int value;
    char label[100];
} ChartEntry;

ChartEntry data[MAX_ENTRIES];

int main() {
    int totalEntries;

    printf("How many categories do you want? (Max %d): ", MAX_ENTRIES);
    scanf("%d", &totalEntries);

    while (totalEntries <= 0 || totalEntries > MAX_ENTRIES) {
        printf("Invalid amount. Enter between 1 and %d: ", MAX_ENTRIES);
        scanf("%d", &totalEntries);
    }

    int remainingPercentage = 100;

    for (int i = 0; i < totalEntries; i++) {
        printf("\nCategory %d\n", i + 1);

        printf("Enter label: ");
        scanf(" %[^\n]", data[i].label);

        if (i == totalEntries - 1) {
            data[i].value = remainingPercentage;
            printf("%s automatically gets the remaining %d%%\n", data[i].label, data[i].value);
        } else {
            printf("Remaining percentage available: %d%%\n", remainingPercentage);
            printf("Enter percentage for %s: ", data[i].label);
            scanf("%d", &data[i].value);

            while (data[i].value <= 0 || data[i].value > remainingPercentage) {
                printf("Invalid percentage. You only have %d%% left. Enter again: ", remainingPercentage);
                scanf("%d", &data[i].value);
            }

            remainingPercentage -= data[i].value;
        }
    }

    InitWindow(WIDTH, HEIGHT, "Pie Chart Animal");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        float current_angle = 0.0f;
        Vector2 center = { WIDTH / 2.0f, HEIGHT / 2.0f };

        int mouseX = GetMouseX();
        int mouseY = GetMouseY();

        float deltaX = mouseX - center.x;
        float deltaY = mouseY - center.y;

        float mouseAngleRad = atan2f(deltaY, deltaX);
        float mouseAngleDeg = mouseAngleRad * RAD2DEG;

        if (mouseAngleDeg < 0) {
            mouseAngleDeg += 360;
        }

        for (int i = 0; i < totalEntries; i++) {
            float percentage = data[i].value / 100.0f;
            float end_angle = current_angle + percentage * 360.0f;

            float radius = HEIGHT / 2.0f * 0.8f;

            if (mouseAngleDeg >= current_angle && mouseAngleDeg < end_angle) {
                radius *= 1.1f;

                char infoText[200];
                sprintf(infoText, "%s : %d%%", data[i].label, data[i].value);
                DrawText(infoText, 30, 30, 30, WHITE);
            }

            DrawCircleSector(
                center,
                radius,
                current_angle,
                end_angle,
                100,
                colors[i % 10]
            );

            current_angle = end_angle;
        }

        DrawText("Move mouse over the pie chart", 20, HEIGHT - 40, 20, WHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}