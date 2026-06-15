#include "../timer.h"
#include <windows.h>

static LARGE_INTEGER s_freq;
static LARGE_INTEGER s_start;

void timer_init(void) {
    QueryPerformanceFrequency(&s_freq);
    QueryPerformanceCounter(&s_start);
}

uint64_t timer_us(void) {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (uint64_t)((now.QuadPart - s_start.QuadPart) * 1000000LL
                      / s_freq.QuadPart);
}

void timer_usleep(uint32_t us) {
    uint64_t end = timer_us() + us;
    if (us > 3000)
        Sleep((us - 2000) / 1000);
    while (timer_us() < end);
}
