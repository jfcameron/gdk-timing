// © Joseph Cameron - MIT License

#include <gdk/timing/impl_sleep.h>

#include <chrono>
#include <thread>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif

using namespace gdk::timing;

namespace {
    class high_resolution_timer final {
    public:
        high_resolution_timer()
        : m_Handle(CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
            TIMER_ALL_ACCESS)) {}
        ~high_resolution_timer() { if (m_Handle) CloseHandle(m_Handle); }
        high_resolution_timer(const high_resolution_timer &) = delete;
        high_resolution_timer &operator=(const high_resolution_timer &) = delete;

        [[nodiscard]] bool wait(const time_type aSeconds) const {
            if (!m_Handle) return false;

            LARGE_INTEGER due;
            due.QuadPart = -static_cast<LONGLONG>(aSeconds * 1e7);

            return SetWaitableTimer(m_Handle, &due, 0, nullptr, nullptr, FALSE)
                && WaitForSingleObject(m_Handle, INFINITE) == WAIT_OBJECT_0;
        }

    private:
        HANDLE m_Handle;
    };
}

void gdk::timing::_sleep_for(const time_type aSeconds) {
    static thread_local const high_resolution_timer timer;

    if (timer.wait(aSeconds)) return;

    std::this_thread::sleep_for(std::chrono::duration<time_type>(aSeconds));
}
