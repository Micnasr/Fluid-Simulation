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
    std::vector<Particle> particles;

    float pixelsPerMeter = 100.0f;
    float worldWidth;
    float worldHeight;

    float particleRadius = 0.04f;
    float particleSpacing = 0.10f;
    float gravity = 9.81f;
};
