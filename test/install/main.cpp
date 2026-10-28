// © Joseph Cameron - MIT License

#include <gdk/timing/game_loop.h>

#include <cstdint>

int main() {
    using namespace gdk::timing;

    std::uint64_t steps = 0;

    game_loop loop(frames_per_second::uncapped(),
        [](const game_loop::frame) { return game_loop::SHOULD_CONTINUE; },
        steps_per_second{1000},
        [&steps](const game_loop::step aStep) {
            ++steps;
            return aStep.index + 1 >= 10 ? game_loop::SHOULD_CLOSE : game_loop::SHOULD_CONTINUE;
        });

    loop.run();

    return steps == 10 ? 0 : 1;
}
