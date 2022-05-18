#if !defined(IML_PROFILER_H)
#define IML_PROFILER_H

#if defined(IML_NAMESPACE)
namespace IML_NAMESPACE {
#  if 0
}
#  endif
#endif
//
//~



//
// @note: Options
//
#if !defined(IML_PROF_ENABLED)
#  define IML_PROF_ENABLED  0
#endif

#if !defined(IML_TIMER_ENABLED)
#  define IML_TIMER_ENABLED  1
#endif


#if IML_PROF_ENABLED
#  define TRACY_ENABLED  1
#else
#  define TRACY_ENABLED  0
#endif


//#  define Profile_Zone_Base
#if TRACY_ENABLED
#  define Profile_Zone()            ZoneScoped
#  define Profile_Zone_Named(name)  ZoneScopedN(name)
#  define Profile_Plot(name, value_s64)  TracyPlot(name, value_s64)
#  define Profile_Memory_Alloc(ptr, size)  TracyAlloc(ptr, size)
#  define Profile_Memory_Free(ptr)         TracyFree(ptr)
#else
#  define Profile_Zone()
#  define Profile_Zone_Named(name)
#  define Profile_Plot(name, value_s64)
#  define Profile_Memory_Alloc(ptr, size)
#  define Profile_Memory_Free(ptr)
#endif


//
// @note: Profilers
//
//#pragma comment(lib, "Ws2_32.lib")
//#pragma comment(lib, "Sechost.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Shell32.lib")

//~ @note Tracy
#if TRACY_ENABLED
#  if BUILD_DEBUG
#    error "You shouldn't run Tracy with Debug mode!"
#  endif
#  define TRACY_ENABLE
//
#  define TRACY_NO_EXIT  1
#  include "os/win32/win32_include.h"
#  include "tracy/TracyClient.cpp"
#  include "tracy/Tracy.hpp"
#  undef assert
#  undef KB
#  undef MB
#  undef GB
#  undef TB
#endif

// @note Usage:
#if 0
int main() {
    ZoneScoped;
    {
        ZoneScopedN("do_some_work");
        do_some_work();
    }
}

void *operator new(std::size_t count) {
    auto ptr = malloc(count);
    TracyAlloc(ptr, count);
    return ptr;
}
void operator delete(void *ptr) noexcept {
    TracyFree(ptr);
    free(ptr);
}
#endif

//~ @note iml_timer
#include "iml_timer.h"

// @note Usage:
#if 0
int main() {
    Timer t;
    start(t);
    do_some_work();
    stop(t);
    {
        Scoped_Timer(t);
        do_some_work();
    }
    printf("Timer: %fms\n", get_duration(t));
}
#endif


//
//~
#ifdef IML_NAMESPACE
#if 0
{
#endif
}
#endif

#endif

