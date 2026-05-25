#include "raylib.h"

int main()
{
    const int screenWidth = 1000;
    const int screenHeight = 700;

    InitWindow(screenWidth, screenHeight, "Fluid Simulation");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(BLACK);

        DrawCircle(screenWidth / 2, screenHeight / 2, 60, BLUE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}