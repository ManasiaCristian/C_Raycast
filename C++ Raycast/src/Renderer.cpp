#include "Renderer.h"

#include <algorithm>
#include <cstdio>

Renderer::Renderer(int width, int height)
    : width_(width),
      height_(height),
      screen_(static_cast<std::size_t>(width) * height, ' ') {
    // Reserve once: width+1 per row leaves room for the '\n' separators.
    present_.reserve(static_cast<std::size_t>(width + 1) * height + 8);
}

char Renderer::shadeForDistance(double distance, bool sideY) const {
    // Dense (near) ----------------------------> light (far)
    static const std::string ramp = "@%#*+=~-:.` ";
    constexpr double maxDist = 16.0;

    double t = std::clamp(distance / maxDist, 0.0, 1.0);
    int idx = static_cast<int>(t * (ramp.size() - 1));

    // Differentiate the two wall orientations for a sense of solidity.
    if (sideY) idx = std::min<int>(idx + 1, static_cast<int>(ramp.size()) - 1);

    return ramp[static_cast<std::size_t>(idx)];
}

void Renderer::putText(int x, int y, const std::string& text) {
    if (y < 0 || y >= height_) return;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const int px = x + static_cast<int>(i);
        if (px < 0 || px >= width_) continue;
        screen_[static_cast<std::size_t>(y) * width_ + px] = text[i];
    }
}

void Renderer::renderFrame(const World& world, const Player& player,
                           const Raycaster& caster) {
    const int horizon = height_ / 2;

    for (int x = 0; x < width_; ++x) {
        const RayHit ray = caster.cast(world, player, x, width_);
        const double dist = ray.hit ? ray.distance : 1e9;

        // Perceived wall height is inversely proportional to distance.
        int lineHeight = ray.hit
            ? static_cast<int>(height_ / std::max(dist, 1e-4))
            : 0;
        if (lineHeight > height_) lineHeight = height_;

        const int  drawStart = (height_ - lineHeight) / 2;
        const int  drawEnd   = drawStart + lineHeight;
        const char wallCh    = shadeForDistance(dist, ray.sideY);

        for (int y = 0; y < height_; ++y) {
            char c;
            if (y < drawStart) {
                c = ' ';                              // ceiling / sky
            } else if (y < drawEnd) {
                c = wallCh;                           // vertical wall slice
            } else {
                // Floor: rows nearer the bottom are "closer", so denser.
                static const char fr[] = " .:-=+";
                const int fl = static_cast<int>(sizeof(fr)) - 2; // drop NUL
                const double depth = (static_cast<double>(y) - horizon) / horizon;
                int idx = std::clamp(static_cast<int>(depth * fl), 0, fl);
                c = fr[idx];
            }
            screen_[static_cast<std::size_t>(y) * width_ + x] = c;
        }
    }
}

void Renderer::drawHud(const Player& player, double fps) {
    char line[192];
    std::snprintf(line, sizeof(line),
        " ASCII Raycaster | pos (%.1f, %.1f) | heading %4.0f deg | fov %2.0f | %5.1f fps ",
        player.x, player.y,
        player.angle * 180.0 / kPi,
        player.fov   * 180.0 / kPi,
        fps);
    putText(1, 0, line);
    putText(1, height_ - 1,
            " [W/S] move  [A/D] turn  [Q/E] strafe  [X or ESC] quit ");
}

void Renderer::present() {
    present_.clear();
    present_ += "\x1b[H";   // ANSI: move cursor to top-left (no clear -> no flicker)
    for (int y = 0; y < height_; ++y) {
        present_.append(&screen_[static_cast<std::size_t>(y) * width_], width_);
        if (y != height_ - 1) present_ += '\n';
    }
    std::fwrite(present_.data(), 1, present_.size(), stdout);
    std::fflush(stdout);
}
