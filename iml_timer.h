#if !defined(IML_TIMER_H)
#define IML_TIMER_H

#if defined(IML_NAMESPACE)
namespace IML_NAMESPACE {
#  if 0
}
#  endif
#endif
//
//~


#if !defined(IML_TIMER_ENABLED)
#  define IML_TIMER_ENABLED  0
#endif


#if !IML_TIMER_ENABLED
#define Scoped_Timer(...)
struct Timer {};
inline void start(Timer t) {}
inline void stop(Timer t) {}
inline const float get_duration(Timer t) { return 0.0f; }
#else


#include "os/os.h"
#include "iml_types.h"


#define Scoped_Timer(timer) \
start(timer);           \
defer { stop(timer); };



#if COMPILER_MSVC

struct Timer {
	f64 ticks_to_ms;
	s64 ticks_accumulated;
	s64 tick_start;
    
	force_inline Timer() {
        ticks_to_ms = 0.0f;
        ticks_accumulated = 0;
        tick_start = 0;
        
		s64 freq;
		QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
		ticks_to_ms = 1000.0f / f64(freq);
	}
};

force_inline void operator+=(Timer &lhs, const Timer &rhs) {
    lhs.ticks_accumulated += rhs.ticks_accumulated;
}

force_inline Timer operator+(const Timer &lhs, const Timer &rhs) {
    Timer result = lhs;
    result += rhs;
    return result;
}

// @note: I don't know why we use references, so we don't have to put & on the timer when calling these??

force_inline void start(Timer &timer) {
    QueryPerformanceCounter((LARGE_INTEGER *)&timer.tick_start);
}

force_inline void stop(Timer &timer) {
    s64 tick_end;
    QueryPerformanceCounter((LARGE_INTEGER *)&tick_end);
    timer.ticks_accumulated += (tick_end - timer.tick_start);
}

inline f64 get_duration(Timer timer) {
    f64 ms_accumulated = f64(timer.ticks_accumulated) * timer.ticks_to_ms;
    return ms_accumulated;
}

inline f64 get_duration_seconds(Timer timer) {
    return get_duration(timer) / 1000.0f;
}

#else // elif defined(__linux__)

#include <time.h>
#include <cstdint>

#include "iml_types.h"

struct Timer {
    s64 nsec_accumulated;
	timespec ts_start;
    
	force_inline Timer() {
        nsec_accumulated = 0;
        ts_start = {};
	}
};

force_inline void operator+=(Timer &lhs, const Timer &rhs) {
    lhs.nsec_accumulated += rhs.nsec_accumulated;
}

force_inline Timer operator+(const Timer &lhs, const Timer &rhs) {
    Timer result = lhs;
    result += rhs;
    return result;
}

force_inline void start(Timer &timer) {
    clock_gettime(CLOCK_REALTIME, &timer.ts_start);
}

force_inline void stop(Timer &timer) {
    timespec ts_end;
    clock_gettime(CLOCK_REALTIME, &ts_end);
    timer.nsec_accumulated += 1000000000 * (ts_end.tv_sec - timer.ts_start.tv_sec) + (ts_end.tv_nsec - timer.ts_start.tv_nsec);
}

inline f64 get_duration(Timer timer) {
    f64 ms_accumulated = cast(f64)(timer.nsec_accumulated) * 1e-6f;
    return ms_accumulated;
}

inline f64 get_duration_seconds(Timer timer) {
    return get_duration(timer) / 1000.0f;
}

#endif

#endif//else IML_TIMER_DISABLED


//
//~
#ifdef IML_NAMESPACE
#  if 0
{
#  endif
}
#endif

#endif
