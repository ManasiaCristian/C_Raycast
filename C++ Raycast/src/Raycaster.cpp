#include "Raycaster.h"

#include <cmath>

RayHit Raycaster::cast(const World& world, const Player& player,
                       int column, int columns, double maxDistance) const {
    // --- 1. Map this screen column to a ray angle inside the FOV cone -------
    // t spreads linearly from 0 (leftmost column) to 1 (rightmost column).
    const double t = (columns > 1)
                   ? static_cast<double>(column) / (columns - 1)
                   : 0.5;
    const double rayAngle = player.angle - player.fov * 0.5 + t * player.fov;

    // Unit direction vector for this ray.
    const double rayDirX = std::cos(rayAngle);
    const double rayDirY = std::sin(rayAngle);

    // --- 2. DDA setup ------------------------------------------------------
    // Which grid cell we start in.
    int mapX = static_cast<int>(std::floor(player.x));
    int mapY = static_cast<int>(std::floor(player.y));

    // Ray length accrued when crossing one full unit in X (resp. Y).
    // Because rayDir is a unit vector, these accumulate true Euclidean length.
    const double deltaDistX = (rayDirX == 0.0) ? 1e30 : std::abs(1.0 / rayDirX);
    const double deltaDistY = (rayDirY == 0.0) ? 1e30 : std::abs(1.0 / rayDirY);

    int    stepX, stepY;          // +1 or -1: direction we march through cells
    double sideDistX, sideDistY;  // ray length from origin to the next gridline

    if (rayDirX < 0.0) { stepX = -1; sideDistX = (player.x - mapX) * deltaDistX; }
    else               { stepX =  1; sideDistX = (mapX + 1.0 - player.x) * deltaDistX; }
    if (rayDirY < 0.0) { stepY = -1; sideDistY = (player.y - mapY) * deltaDistY; }
    else               { stepY =  1; sideDistY = (mapY + 1.0 - player.y) * deltaDistY; }

    // --- 3. March the grid until we hit a wall or run out of range ---------
    bool   hit       = false;
    bool   sideY     = false;
    double rayLength = maxDistance;  // Euclidean distance travelled along the ray
    int    cellValue = 0;

    while (true) {
        // Always advance toward whichever gridline (X or Y) is nearer. The
        // distance to *that* boundary is the ray length where we enter the
        // next cell, so we record it before stepping over the line.
        if (sideDistX < sideDistY) {
            rayLength  = sideDistX;
            sideDistX += deltaDistX;
            mapX      += stepX;
            sideY      = false;          // we crossed a vertical (X) gridline
        } else {
            rayLength  = sideDistY;
            sideDistY += deltaDistY;
            mapY      += stepY;
            sideY      = true;           // we crossed a horizontal (Y) gridline
        }

        if (rayLength > maxDistance) break;   // nothing solid in view

        cellValue = world.at(mapX, mapY);
        if (cellValue > 0) { hit = true; break; }
    }

    // --- 4. Fisheye correction --------------------------------------------
    // Straight Euclidean distance would bow walls outward at the screen edges.
    // Projecting the ray length onto the player's view axis (multiplying by the
    // cosine of the angle between the ray and the view direction) yields the
    // perpendicular distance, which is what gives flat, undistorted walls.
    RayHit result;
    result.hit       = hit;
    result.sideY     = sideY;
    result.cellValue = cellValue;
    result.distance  = rayLength * std::cos(rayAngle - player.angle);
    return result;
}
