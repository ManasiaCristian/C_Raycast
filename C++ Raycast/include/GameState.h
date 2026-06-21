#pragma once
//
// GameState.h - Module 1: The World
// -----------------------------------
// Holds the static map grid and the dynamic Player state. This module knows
// nothing about rendering or input - it is pure data plus map queries.
//
#include <vector>
#include <cstddef>

// Single source of truth for PI so every module agrees on angle units.
inline constexpr double kPi = 3.14159265358979323846;

// The player is a point in continuous 2D map space with a facing angle.
// Movement/turn speeds are expressed *per second* so the game loop can
// multiply them by delta-time for frame-rate independence.
struct Player {
    double x = 3.5;             // position in map units (X)
    double y = 6.5;             // position in map units (Y)
    double angle = 0.0;         // viewing direction in radians; 0 faces +X
    double fov = kPi / 3.0;     // field of view (60 degrees)
    double moveSpeed = 4.0;     // map units travelled per second
    double turnSpeed = 2.4;     // radians rotated per second
};

// The map: a flat (1D) grid of integers. 0 == empty, >0 == wall.
// Stored row-major: index = y * width + x.
class World {
public:
    World(std::vector<int> cells, int width, int height);

    // Factory for a hand-authored demo level.
    static World makeDefault();

    int width()  const { return w_; }
    int height() const { return h_; }

    // Cell value. Anything outside the grid is treated as solid wall so rays
    // (and the player) can never escape the level.
    int  at(int cx, int cy) const;

    bool isWall(int cx, int cy) const;       // grid-cell test
    bool isWall(double fx, double fy) const; // continuous-position test

private:
    std::vector<int> cells_;
    int w_ = 0;
    int h_ = 0;
};
