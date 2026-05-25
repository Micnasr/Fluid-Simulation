#pragma once

#include <vector>

#include "Particle.h"

class Simulation
{
public:
    Simulation(int screenWidth, int screenHeight);

    void Reset();
    void Draw() const;
    void Update(float deltaTime);

private:
    void ClearAccelerations();
    void ApplyGravity();
    void ApplyMousePush();
    void IntegrateParticles(float deltaTime);
    void ResolveBoundaryCollisions();
    
    void CalculateDensities();
    float SmoothingKernel(float distance) const;
    
    void CalculatePressures();
    Color GetPressureColor(float pressure) const;

    std::vector<Particle> particles;

    float pixelsPerMeter = 100.0f;
    float worldWidth;
    float worldHeight;

    float particleRadius = 0.04f;
    float particleSpacing = 0.10f;
    
    float particleMass = 0.01f;
    float targetDensity = 1.0f;
    float pressureStiffness = 20.0f;
    
    float gravity = 9.81f;

    float collisionDamping = 0.5f;

    float mouseRadius = 1.0f;
    float mouseStrength = 25.0f;

    float smoothingRadius = 0.25f;
};
