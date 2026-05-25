#pragma once

#include "raylib.h"

// State stored for a single simulated fluid particle.
struct Particle
{
    Vector2 position{};
    Vector2 velocity{};
    Vector2 acceleration{};

    float density = 0.0f;
    float pressure = 0.0f;
};
