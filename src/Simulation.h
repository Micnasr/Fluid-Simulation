#pragma once

#include <cstddef>
#include <functional>
#include <unordered_map>
#include <vector>

#include "Particle.h"

struct SpatialCell
{
    int x;
    int y;

    bool operator==(const SpatialCell&) const = default;
};

struct SpatialCellHash
{
    std::size_t operator()(const SpatialCell& cell) const
    {
        const std::size_t hashX = std::hash<int>{}(cell.x);
        const std::size_t hashY = std::hash<int>{}(cell.y);

        return hashX ^ (hashY << 1);
    }
};

class Simulation
{
public:
    Simulation(int screenWidth, int screenHeight);

    void Reset();
    void Draw() const;
    void Update(float deltaTime);

    std::size_t GetParticleCount() const;

private:
    void ClearAccelerations();
    void ApplyGravity();
    void ApplyMousePush();
    void IntegrateParticles(float deltaTime);
    void ResolveBoundaryCollisions();
    float DistanceBetween(Vector2 firstPosition, Vector2 secondPosition) const;

    SpatialCell GetSpatialCell(Vector2 position) const;
    void BuildSpatialGrid();
    
    void CalculateDensitiesAndPressures();
    float SmoothingKernel(float distance) const;
    
    void ApplyPressureAccelerations();
    float PressureKernelDerivative(float distance) const;

    void ApplyViscosityAccelerations();
    float ViscosityKernelLaplacian(float distance) const;

    Color GetPressureColor(float pressure) const;

    void PredictPositions();

    std::vector<Particle> particles;
    std::unordered_map<SpatialCell, std::vector<std::size_t>, SpatialCellHash> spatialGrid;

    float pixelsPerMeter = 100.0f;
    float worldWidth;
    float worldHeight;

    float particleRadius = 0.04f;
    float particleSpacing = 0.10f;
    float spawnJitter = 0.01f;
    
    float particleMass = 0.01f;
    float targetDensity = 0.8f;
    float pressureStiffness = 30.0f;
    float viscosityStrength = 0.01f;
    
    float gravity = 9.81f;

    float collisionDamping = 0.5f;

    float mouseRadius = 1.0f;
    float mouseStrength = 25.0f;

    float smoothingRadius = 0.25f;

	float predictionTime = 1.0f / 120.0f; // Predict 1/120 second into the future
};
