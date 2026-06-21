//
// main.cpp - Module 2: The Game Loop & Timing (Core Engine)
// ---------------------------------------------------------
// Wires the modules together and runs the loop: measure delta-time, gather
// input, integrate movement (scaled by dt for frame-rate independence), cast
// the world, and present the frame.
//
#include "GameState.h"
#include "Raycaster.h"
#include "Renderer.h"
#include "Input.h"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <thread>

int main() {
    constexpr int SCREEN_W = 120;   // terminal columns (one ray each)
    constexpr int SCREEN_H = 40;    // terminal rows

    World     world  = World::makeDefault();
    Player    player;               // starts in an open cell (see defaults)
    Raycaster caster;
    Renderer  renderer(SCREEN_W, SCREEN_H);
    Input     input;                // configures terminal; restores on exit

    using Clock = std::chrono::steady_clock;
    auto previous = Clock::now();

    bool   running = true;
    double fps     = 0.0;

    while (running) {
        // ---- Timing: precise per-frame delta-time -------------------------
        const auto now = Clock::now();
        const std::chrono::duration<double> frame = now - previous;
        previous = now;

        double dt = frame.count();
        if (dt > 0.05) dt = 0.05;   // clamp big stalls (window drag, breakpoints)

        const double instFps = (dt > 0.0) ? 1.0 / dt : 0.0;
        fps = (fps == 0.0) ? instFps : fps * 0.9 + instFps * 0.1; // smooth

        // ---- Input --------------------------------------------------------
        input.poll();
        if (input.quit()) { running = false; break; }

        // Rotation - scaled by dt so turn speed is identical at any frame rate.
        if (input.turnLeft())  player.angle -= player.turnSpeed * dt;
        if (input.turnRight()) player.angle += player.turnSpeed * dt;

        // Build a movement vector from forward/back + strafe intents.
        const double dirX  = std::cos(player.angle);
        const double dirY  = std::sin(player.angle);
        const double perpX = std::cos(player.angle + kPi * 0.5);
        const double perpY = std::sin(player.angle + kPi * 0.5);

        double mvX = 0.0, mvY = 0.0;
        if (input.forward())     { mvX += dirX;  mvY += dirY;  }
        if (input.backward())    { mvX -= dirX;  mvY -= dirY;  }
        if (input.strafeRight()) { mvX += perpX; mvY += perpY; }
        if (input.strafeLeft())  { mvX -= perpX; mvY -= perpY; }

        const double step = player.moveSpeed * dt;
        const double newX = player.x + mvX * step;
        const double newY = player.y + mvY * step;

        // Axis-separated collision so the player slides along walls.
        if (!world.isWall(newX, player.y)) player.x = newX;
        if (!world.isWall(player.x, newY)) player.y = newY;

        // Keep the heading in a tidy range.
        if (player.angle >  kPi) player.angle -= 2.0 * kPi;
        if (player.angle < -kPi) player.angle += 2.0 * kPi;

        // ---- Render -------------------------------------------------------
        renderer.renderFrame(world, player, caster);
        renderer.drawHud(player, fps);
        renderer.present();

        // Yield a sliver so we don't peg a core at 100%.
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    std::printf("\x1b[2J\x1b[H");   // tidy the terminal on the way out
    std::fflush(stdout);
    return 0;
}
