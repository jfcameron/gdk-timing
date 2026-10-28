# gdk-timing
Timing library for games: a game loop that presents frames at a capped rate and, beside them, can run a simulation at a fixed step.

API documentation: https://jfcameron.github.io/gdk-timing/

## Getting Started
Install cmake 3.21 or greater, and a c++20 compatible compiler
```
# initialize the tree
git submodule update --init --recursive

# generate the script to build the library and its tests on linux using gcc
cmake --preset linux-gcc

# build the library and tests
cmake --build --preset linux-gcc

# run the test suite
ctest --preset linux-gcc
```
See `CMakePresets.json` for the full list of supported platforms and compilers. See `.github/workflows/ci.yml` to see exactly how CI runners build it on different platforms.

To use it from another project, add it with `add_subdirectory` and link `gdk::timing`, or install it and `find_package(gdk-timing)`.

## Usage Example
```cpp
#include <gdk/timing/game_loop.h>

using namespace gdk::timing;

game_loop(frames_per_second{60},
    [&](const game_loop::frame aFrame) {
        render(aFrame.interpolation); // User rendering update
        return game_loop::SHOULD_CONTINUE;
    },
    steps_per_second{60},
    [&](const game_loop::step aStep) {
        world.integrate(aStep.delta); // User simulation update
        return game_loop::SHOULD_CONTINUE;
    }).run();
```

## Overview
### Frames and steps
The frame behaviour runs once per frame, with the time the last frame actually took. The fixed behaviour, if there is one, runs as many times as that time covers, each step the same length, so the simulation comes out the same whatever the frame rate. `interpolation` says how far a frame sits between the last step and the next, for drawing between them.

A frame's delta is clamped to a ceiling, a quarter of a second unless one is given, so a stall such as a window being dragged cannot make one frame run an unbounded number of steps: at most the ceiling times the step rate.

### Time scale
`set_time_scale` speeds game time up or slows it down against the clock: `game_loop::PAUSED` stops the simulation while frames keep coming, and the clock's own delta is still there as `unscaled_delta` for whatever should not pause. A step is the same length at any scale; the scale changes how many run.

### Stopping
A behaviour returns `game_loop::SHOULD_CLOSE` to end the loop, or `request_stop` ends it from anywhere. Either way the loop returns at once, without waiting out the rest of the frame. A finished loop stays finished until `reset`.

### A loop of your own, and the browser
`run` is `tick` until a behaviour asks to stop. A program with a loop of its own calls `tick` once per iteration instead. A browser needs that: its page cannot block, so `run` cannot be used there, and a frame rate cap would make the main thread spin. Call `tick` from `emscripten_set_main_loop`, with the loop uncapped, and let the browser pace the frames.

### Threads
`request_stop`, the frame rate cap and the time scale can be set from any thread, while the loop is running. One thread ticks at a time: `tick`, `run` and `reset` throw if another is in progress.

