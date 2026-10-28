// © Joseph Cameron - MIT License

#include <gdk/timing/impl_sleep.h>

#include <chrono>
#include <thread>

void gdk::timing::_sleep_for(const time_type aSeconds) {
    std::this_thread::sleep_for(std::chrono::duration<time_type>(aSeconds));
}
