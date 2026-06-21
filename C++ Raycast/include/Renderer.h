#pragma once
//
// Renderer.h - Module 4: The Presentation Layer
// ---------------------------------------------
// Owns an in-memory character framebuffer. Each frame it fills the buffer with
// ceiling / wall / floor characters, then flushes the whole thing at once with
// the cursor homed via an ANSI escape - no clear, no flicker.
//
#include <string>

#include "GameState.h"
#include "Raycaster.h"

class Renderer {
public:
    Renderer(int width, int height);

    // Rasterize one full frame into the internal framebuffer.
    void renderFrame(const World& world, const Player& player,
                     const Raycaster& caster);

    // Overlay a heads-up display (stats + controls) on top of the frame.
    void drawHud(const Player& player, double fps);

    // Flush the framebuffer to the terminal in a single write.
    void present();

    int width()  const { return width_; }
    int height() const { return height_; }

private:
    // Pick a shading glyph: closer walls = denser glyph. Y-side faces are
    // nudged one step lighter to fake directional lighting.
    char shadeForDistance(double distance, bool sideY) const;

    // Blit a string into the framebuffer at (x, y), clipping to bounds.
    void putText(int x, int y, const std::string& text);

    int width_;
    int height_;
    std::string screen_;    // width_ * height_ characters (the framebuffer)
    std::string present_;   // reused output buffer (ANSI home + rows + newlines)
};
