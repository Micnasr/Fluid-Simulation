#include "Simulation.h"

#include "raylib.h"

Simulation::Simulation(int screenWidth, int screenHeight)
    : worldWidth(screenWidth / pixelsPerMeter),
      worldHeight(screenHeight / pixelsPerMeter)
{
    Reset();
}

// Initialize the simulation with particles in a grid pattern.
void Simulation::Reset()
{
    particles.clear();

    constexpr int columns = 20;
    constexpr int rows = 15;

    const float blockWidth = (columns - 1) * particleSpacing;
    const float startX = (worldWidth - blockWidth) * 0.5f;
    const float startY = 0.8f;

    particles.reserve(columns * rows);

    for (int row = 0; row < rows; ++row)
    {
        for (int column = 0; column < columns; ++column)
        {
            Particle particle;
            particle.position = {
                startX + column * particleSpacing,
                startY + row * particleSpacing
            };

            particles.push_back(particle);
        }
    }
}

// Draw particles as circles, converting from world units to screen pixels.
void Simulation::Draw() const
{
    for (const Particle& particle : particles)
    {
        const Vector2 screenPosition = {
            particle.position.x * pixelsPerMeter,
            particle.position.y * pixelsPerMeter
        };

        DrawCircleV(screenPosition, particleRadius * pixelsPerMeter, BLUE);
    }
}

void Simulation::Update(float deltaTime)
{
    for (Particle& particle : particles)
    {
        particle.force = { 0.0f, gravity };

        particle.velocity.x += particle.force.x * deltaTime;
        particle.velocity.y += particle.force.y * deltaTime;

        particle.position.x += particle.velocity.x * deltaTime;
        particle.position.y += particle.velocity.y * deltaTime;
    }
}