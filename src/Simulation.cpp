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
    ClearAccelerations();

    ApplyGravity();
    ApplyMousePush();

    IntegrateParticles(deltaTime);
    ResolveBoundaryCollisions();
}

void Simulation::ClearAccelerations()
{
    for (Particle& particle : particles)
    {
        particle.acceleration = { 0.0f, 0.0f };
    }
}

void Simulation::ApplyGravity()
{
    for (Particle& particle : particles)
    {
        particle.acceleration.y += gravity;
    }
}

void Simulation::IntegrateParticles(float deltaTime)
{
    for (Particle& particle : particles)
    {
        particle.velocity.x += particle.acceleration.x * deltaTime;
        particle.velocity.y += particle.acceleration.y * deltaTime;

        particle.position.x += particle.velocity.x * deltaTime;
        particle.position.y += particle.velocity.y * deltaTime;
    }
}

void Simulation::ResolveBoundaryCollisions()
{
    for (Particle& particle : particles)
    {
        if (particle.position.x - particleRadius < 0.0f)
        {
            particle.position.x = particleRadius;
            particle.velocity.x *= -collisionDamping;
        }

        if (particle.position.x + particleRadius > worldWidth)
        {
            particle.position.x = worldWidth - particleRadius;
            particle.velocity.x *= -collisionDamping;
        }

        if (particle.position.y - particleRadius < 0.0f)
        {
            particle.position.y = particleRadius;
            particle.velocity.y *= -collisionDamping;
        }

        if (particle.position.y + particleRadius > worldHeight)
        {
            particle.position.y = worldHeight - particleRadius;
            particle.velocity.y *= -collisionDamping;
        }
    }
}

void Simulation::ApplyMousePush()
{
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        return;
    }

    const Vector2 mouseScreenPosition = GetMousePosition();

	// Convert mouse position from screen pixels to world units.
    const Vector2 mousePosition = {
        mouseScreenPosition.x / pixelsPerMeter,
        mouseScreenPosition.y / pixelsPerMeter
    };

    for (Particle& particle : particles)
    {
        const Vector2 offset = {
            particle.position.x - mousePosition.x,
            particle.position.y - mousePosition.y
        };

		// Calculate the distance from the particle to the mouse position.
        const float distance = sqrtf(offset.x * offset.x + offset.y * offset.y);

		// If the particle is within the mouse radius, apply a force pushing it away from the mouse position.
        if (distance > 0.0f && distance < mouseRadius)
        {
            const Vector2 directionAwayFromMouse = {
                offset.x / distance,
                offset.y / distance
            };

            particle.acceleration.x += directionAwayFromMouse.x * mouseStrength;
            particle.acceleration.y += directionAwayFromMouse.y * mouseStrength;
        }
    }
}