#include "Simulation.h"

#include "raylib.h"

#include <cmath>

namespace
{
    // Cells that can contain particles within one smoothing radius.
    constexpr SpatialCell NeighbourCellOffsets[] = {
        { -1, -1 }, { 0, -1 }, { 1, -1 },
        { -1,  0 }, { 0,  0 }, { 1,  0 },
        { -1,  1 }, { 0,  1 }, { 1,  1 }
    };
}

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

    constexpr int columns = 50;
    constexpr int rows = 50;

    const float blockWidth = (columns - 1) * particleSpacing;
    const float startX = (worldWidth - blockWidth) * 0.5f;
    const float startY = 0.8f;

    particles.reserve(columns * rows);

    for (int row = 0; row < rows; ++row)
    {
        for (int column = 0; column < columns; ++column)
        {
            Particle particle;

            const float jitterX = spawnJitter * GetRandomValue(-1000, 1000) / 1000.0f;
            const float jitterY = spawnJitter * GetRandomValue(-1000, 1000) / 1000.0f;

            // Introduce small position variations per particle
            particle.position = {
                startX + column * particleSpacing + jitterX,
                startY + row * particleSpacing + jitterY
            };

            particles.push_back(particle);
        }
    }

    BuildSpatialGrid();
    CalculateDensitiesAndPressures();
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

        const Color particleColor = GetPressureColor(particle.pressure);

        DrawCircleV(screenPosition, particleRadius * pixelsPerMeter, particleColor);
    }
}

void Simulation::Update(float deltaTime)
{
    constexpr int substeps = 1;
    const float stepTime = deltaTime / substeps;

    for (int step = 0; step < substeps; ++step)
    {
        BuildSpatialGrid();

        CalculateDensitiesAndPressures();

        ClearAccelerations();
        ApplyGravity();
        ApplyMousePush();
        ApplyPressureAccelerations();
		ApplyViscosityAccelerations();

        IntegrateParticles(stepTime);
        ResolveBoundaryCollisions();
    }
}

void Simulation::ClearAccelerations()
{
    for (Particle& particle : particles)
    {
        particle.acceleration = { 0.0f, 0.0f };
    }
}

std::size_t Simulation::GetParticleCount() const
{
    return particles.size();
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

float Simulation::DistanceBetween(Vector2 firstPosition, Vector2 secondPosition) const
{
    const float offsetX = firstPosition.x - secondPosition.x;
    const float offsetY = firstPosition.y - secondPosition.y;

    return sqrtf(offsetX * offsetX + offsetY * offsetY);
}

// Returns the spatial-hash cell containing a world-space position.
SpatialCell Simulation::GetSpatialCell(Vector2 position) const
{
    return {
        static_cast<int>(std::floor(position.x / smoothingRadius)),
        static_cast<int>(std::floor(position.y / smoothingRadius))
    };
}

// Rebuilds cell buckets from current particle positions.
void Simulation::BuildSpatialGrid()
{
    spatialGrid.clear();

    for (std::size_t particleIndex = 0; particleIndex < particles.size(); ++particleIndex)
    {
        const SpatialCell cell = GetSpatialCell(particles[particleIndex].position);

        spatialGrid[cell].push_back(particleIndex);
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
        const float distance = DistanceBetween(particle.position, mousePosition);

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

float Simulation::SmoothingKernel(float distance) const
{
    if (distance >= smoothingRadius)
    {
        return 0.0f;
    }

    const float h = smoothingRadius;
    const float value = h * h - distance * distance;
    const float scale = 4.0f / (PI * std::pow(h, 8.0f));

    return scale * value * value * value;
}

// Calculates local density from nearby cells, then derives pressure.
void Simulation::CalculateDensitiesAndPressures()
{
    for (Particle& particle : particles)
    {
        particle.density = 0.0f;

        const SpatialCell particleCell = GetSpatialCell(particle.position);

		// Iterate over the particle's cell and its 8 neighbours to find nearby particles that contribute to density.
        for (const SpatialCell& cellOffset : NeighbourCellOffsets)
        {
            const SpatialCell nearbyCell = {
                particleCell.x + cellOffset.x,
                particleCell.y + cellOffset.y
            };

            const auto bucket = spatialGrid.find(nearbyCell);

            if (bucket == spatialGrid.end())
            {
                continue;
            }

            for (const std::size_t neighbourIndex : bucket->second)
            {
                const Particle& neighbour = particles[neighbourIndex];

                const float distance = DistanceBetween(particle.position, neighbour.position);

                particle.density += particleMass * SmoothingKernel(distance);
            }
        }

        const float densityError = particle.density - targetDensity;
        particle.pressure = std::max(0.0f, pressureStiffness * densityError);
    }
}

Color Simulation::GetPressureColor(float pressure) const
{
    if (pressure < -1.0f)
    {
        return SKYBLUE;
    }

    if (pressure <= 1.0f)
    {
        return BLUE;
    }

    if (pressure < 30.0f)
    {
        return ORANGE;
    }

    return RED;
}

float Simulation::PressureKernelDerivative(float distance) const
{
    if (distance >= smoothingRadius)
    {
        return 0.0f;
    }

    const float h = smoothingRadius;
    const float value = h - distance;
    const float scale = -30.0f / (PI * std::pow(h, 5.0f));

    return scale * value * value;
}

void Simulation::ApplyPressureAccelerations()
{   
    // Calculate the total pressure acceleration acting on each particle.
    for (std::size_t i = 0; i < particles.size(); ++i)
    {
        Particle& particle = particles[i];
        Vector2 pressureAcceleration = { 0.0f, 0.0f };

        const SpatialCell particleCell = GetSpatialCell(particle.position);

        // Search only spatial-hash cells that can contain SPH neighbours.
        for (const SpatialCell& cellOffset : NeighbourCellOffsets)
        {
            const SpatialCell nearbyCell = {
                particleCell.x + cellOffset.x,
                particleCell.y + cellOffset.y
            };

            const auto bucket = spatialGrid.find(nearbyCell);

            if (bucket == spatialGrid.end())
            {
                continue;
            }

            // Accumulate pressure contributions from particles in this nearby cell
            for (const std::size_t j : bucket->second)
            {
                if (i == j)
                {
                    continue;
                }

                const Particle& neighbour = particles[j];

                const Vector2 offset = {
                    particle.position.x - neighbour.position.x,
                    particle.position.y - neighbour.position.y
                };

                float distance = DistanceBetween(particle.position, neighbour.position);

                if (distance >= smoothingRadius)
                {
                    continue;
                }

                Vector2 directionAwayFromNeighbour;

                if (distance < 0.0001f)
                {
                    // Overlapping particles have no direction between them,
                    // so assign an opposite fallback direction to separate them.
                    directionAwayFromNeighbour = {i < j ? -1.0f : 1.0f, 0.0f};

                    distance = 0.0001f;
                }
                else
                {
					// Calculate the normalized direction away from the neighbour particle.
                    directionAwayFromNeighbour = {
                        offset.x / distance,
                        offset.y / distance
                    };
                }

                const float sharedPressure = (particle.pressure + neighbour.pressure) * 0.5f;

                const float slope = PressureKernelDerivative(distance);

                const float densityProduct = particle.density * neighbour.density;

                if (densityProduct <= 0.0f)
                {
                    continue;
                }

                // Equation 10 from "Smoothed Particle Hydrodynamics for Fluid Simulation"
                const float accelerationMagnitude = -particleMass * sharedPressure * slope / densityProduct;

                pressureAcceleration.x += directionAwayFromNeighbour.x * accelerationMagnitude;

                pressureAcceleration.y += directionAwayFromNeighbour.y * accelerationMagnitude;
            }
        }

        // Apply the sum of all nearby pressure interactions this substep.
        particle.acceleration.x += pressureAcceleration.x;
        particle.acceleration.y += pressureAcceleration.y;
    }
}

float Simulation::ViscosityKernelLaplacian(float distance) const
{
    if (distance >= smoothingRadius)
    {
        return 0.0f;
    }

    const float h = smoothingRadius;
    const float scale = 40.0f / (PI * std::pow(h, 5.0f));

    return scale * (h - distance);
}

void Simulation::ApplyViscosityAccelerations()
{
    // Calculate the total viscosity acceleration acting on each particle.
    for (std::size_t i = 0; i < particles.size(); ++i)
    {
        Particle& particle = particles[i];
        Vector2 viscosityAcceleration = { 0.0f, 0.0f };

        const SpatialCell particleCell = GetSpatialCell(particle.position);

        // Search only spatial-hash cells that can contain SPH neighbours.
        for (const SpatialCell& cellOffset : NeighbourCellOffsets)
        {
            const SpatialCell nearbyCell = {
                particleCell.x + cellOffset.x,
                particleCell.y + cellOffset.y
            };

            const auto bucket = spatialGrid.find(nearbyCell);

            if (bucket == spatialGrid.end())
            {
                continue;
            }

            // Accumulate viscosity contributions from particles in this cell.
            for (const std::size_t neighbourIndex : bucket->second)
            {
                if (i == neighbourIndex)
                {
                    continue;
                }

                const Particle& neighbour = particles[neighbourIndex];

                const float distance = DistanceBetween(particle.position, neighbour.position);

                if (distance >= smoothingRadius)
                {
                    continue;
                }

                const float densityProduct = particle.density * neighbour.density;

                if (densityProduct <= 0.0f)
                {
                    continue;
                }

                // Viscosity accelerates this particle toward the velocities of its neighbours, reducing local velocity differences.
                const Vector2 velocityDifference = {
                    neighbour.velocity.x - particle.velocity.x,
                    neighbour.velocity.y - particle.velocity.y
                };

                const float laplacian = ViscosityKernelLaplacian(distance);

                // Equation 14 from "Smoothed Particle Hydrodynamics for Fluid Simulation"
                const float influence = viscosityStrength * particleMass * laplacian / densityProduct;

                viscosityAcceleration.x += velocityDifference.x * influence;

                viscosityAcceleration.y += velocityDifference.y * influence;
            }
        }

        // Apply the sum of all nearby viscosity interactions this substep.
        particle.acceleration.x += viscosityAcceleration.x;
        particle.acceleration.y += viscosityAcceleration.y;
    }
}
