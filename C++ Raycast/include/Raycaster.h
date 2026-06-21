#pragma once
//
// Raycaster.h - Module 3: The Math Layer
// --------------------------------------
// Casts a single ray per screen column using the DDA grid-traversal algorithm,
// then applies fisheye correction. Produces a small POD describing the hit.
//
#include "GameState.h"

struct RayHit {
    double distance  = 0.0;   // fisheye-corrected perpendicular distance to wall
    bool   hit       = false; // was a wall found within range?
    bool   sideY     = false; // true if a North/South face (Y-side) was hit
    int    cellValue = 0;     // wall type at the hit cell (for future texturing)
};

class Raycaster {
public:
    // Cast the ray for screen column `column` out of `columns` total columns.
    // `maxDistance` bounds the DDA walk so empty horizons terminate.
    RayHit cast(const World& world, const Player& player,
                int column, int columns, double maxDistance = 64.0) const;
};
