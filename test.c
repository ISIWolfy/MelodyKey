#include "raylib.h"

int main(void) {
    InitWindow(800, 450, "Raylib Test");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Raylib is working!", 250, 200, 20, BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}