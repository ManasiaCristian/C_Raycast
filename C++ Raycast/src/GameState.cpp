#include "GameState.h"

#include <string>
#include <utility>
#include <cmath>

World::World(std::vector<int> cells, int width, int height)
    : cells_(std::move(cells)), w_(width), h_(height) {}

int World::at(int cx, int cy) const {
    // Out of bounds -> solid wall. This keeps the DDA loop and the collision
    // checks bounded without any special-casing at the call sites.
    if (cx < 0 || cy < 0 || cx >= w_ || cy >= h_) return 1;
    return cells_[static_cast<std::size_t>(cy) * w_ + cx];
}

bool World::isWall(int cx, int cy) const {
    return at(cx, cy) > 0;
}

bool World::isWall(double fx, double fy) const {
    return isWall(static_cast<int>(std::floor(fx)),
                  static_cast<int>(std::floor(fy)));
}

World World::makeDefault() {
    // Authored as ASCII for readability; '#' becomes a wall, anything else empty.
    const std::vector<std::string> rows = {
        "################",
        "#..............#",
        "#..##......##..#",
        "#..#........#..#",
        "#..#...##...#..#",
        "#......##......#",
        "#..............#",
        "#...####...#...#",
        "#......#...#...#",
        "#......#...#...#",
        "#..##..........#",
        "#..##.....###..#",
        "#.........#....#",
        "#.........#....#",
        "#..............#",
        "################",
    };

    const int h = static_cast<int>(rows.size());
    const int w = static_cast<int>(rows[0].size());

    std::vector<int> cells(static_cast<std::size_t>(w) * h, 0);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            cells[static_cast<std::size_t>(y) * w + x] =
                (rows[y][x] == '#') ? 1 : 0;
        }
    }
    return World(std::move(cells), w, h);
}
