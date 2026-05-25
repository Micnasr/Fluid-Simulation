#include "raylib.h"

#include "Simulation.h"

int main()
{
    const int screenWidth = 1000;
    const int screenHeight = 700;

    InitWindow(screenWidth, screenHeight, "Fluid Simulation");
    SetTargetFPS(60);

    Simulation simulation(screenWidth, screenHeight);

    while (!WindowShouldClose())
    {
        const float deltaTime = GetFrameTime();

        simulation.Update(deltaTime);

        BeginDrawing();

        ClearBackground(BLACK);

        simulation.Draw();

		// Draw FPS and particle count for debugging.
        DrawFPS(10, 10);

        DrawText(
            TextFormat("Particles: %i", static_cast<int>(simulation.GetParticleCount())),
            10,
            35,
            20,
            WHITE
        );

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
