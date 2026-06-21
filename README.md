# ASCII Terminal Raycaster

This is a 3D raycasting engine that runs entirely inside your terminal. No graphics libraries, no game engines - just C++, some trigonometry, and a lot of ASCII characters pretending to be walls.

The idea is basically the same trick Wolfenstein 3D used in 1992. You stand in a 2D grid, shoot a ray out for every column of pixels on the screen, measure how far it travels before hitting a wall, and draw a vertical slice based on that distance. Do that 120 times per frame and you get something that looks surprisingly like a 3D world.

<img width="1442" height="925" alt="image" src="https://github.com/user-attachments/assets/d0653d99-1d9b-4de7-8b57-30a6f7309f91" />

## Controls

| Key | Action |
|-----|--------|
| `W` / `S` | Move forward / back |
| `A` / `D` | Turn left / right |
| `Q` / `E` | Strafe left / right |
| Arrow keys | Same as WASD |
| `X` or `Esc` | Quit |


> Make sure your terminal is at least **120 columns × 40 rows** before launching, otherwise the frame wraps and looks broken.

## How it works

### Raycasting & the DDA algorithm

For each of the 120 screen columns, we calculate a ray angle spread evenly across the player's 60° field of view. Then we need to figure out how far that ray travels before it hits a wall.

The naive approach - stepping along the ray in tiny fixed increments - is slow and can miss thin walls entirely. Instead we use **DDA (Digital Differential Analyzer)**, which jumps directly from one grid cell boundary to the next. It's much faster and never skips anything.

The rough idea:
1. Work out how much ray length is consumed each time you cross a vertical vs. horizontal grid line
2. Always step toward whichever boundary is closer
3. After each step, check if the new cell is a wall - if it is, you're done

The result is an exact hit with the minimum number of steps.

### Fisheye correction

There's a subtle problem with raw ray distances: rays shot toward the edges of the screen are longer than rays shot straight ahead, even if they're hitting the same flat wall. If you draw wall slices based on raw distance, the walls bow outward in a fisheye curve.

The fix is one line - multiply the distance by the cosine of the angle between the ray and the player's forward direction. That projects the ray length onto the view plane and gives you the true perpendicular distance, which keeps walls flat.

```
correctedDistance = rayLength * cos(rayAngle - playerAngle)
```

### Shading

Distance maps to an ASCII shading ramp: `@ % # * + = ~ - : .` from close to far. Walls hit on their north/south face get bumped one step lighter than east/west faces, which gives a cheap sense of directional lighting and makes corners actually readable.

### No flicker

The whole frame is built in a `std::string` buffer in memory, then written to stdout in a single `fwrite`. Rather than clearing the screen between frames (which causes flicker), we just send the ANSI escape `\x1b[H` to move the cursor back to the top-left and overwrite the previous frame. At a normal framerate the transitions are invisible.

### Frame-rate independence

Movement and rotation speeds are expressed in units-per-second and multiplied by delta-time each frame, so the player moves at the same real-world speed regardless of how fast the loop is running. Large frame spikes (e.g. dragging the window) are clamped so you don't teleport through walls.


Each module is independent enough that you could swap out the renderer or the input layer without touching the raycaster math.
