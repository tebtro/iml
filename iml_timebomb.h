#if !defined(IML_TIMEBOMB_H)
#define IML_TIMEBOMB_H


//
// @important: Only working when compiling with CRT
//
#if BUILD_WITH_NO_CRT
#  define timebomb_assert(...)
#  error "Depracated: Timebomb assert only works when compiling with CRT!"
#else


#include <ctime>   // time, difftime, mktime


//
// @note: Timebomb
//
// Usage:  timebomb_assert(16,8,3024); // not yet reached this deadline
// Usage:  timebomb_assert( 1,5,2020, "Reached deadline, %d\n", x);
// :includes #include <ctime> // time, difftime, mktime
// :includes #include <cstdarg> // va_list, ...
#if BUILD_WITH_ASSERTS
#  define timebomb_assert(d, m, y, ...)  timebomb_assert_always(d, m, y, ##__VA_ARGS__)
#else
#  define timebomb_assert(...)
#endif

#define timebomb_assert_always(day, month, year, ...) \
static const Timebomb CONCAT(_iml_timebomb_,__LINE__)(day, month, year, __FILE__, __LINE__, ##__VA_ARGS__)

struct Timebomb {
#if 0
    Timebomb(int day, int month, int year, const char *file, int line, const char *format = nullptr, ...) {
        time_t timenow = time(0);
        struct tm *deadline = localtime(&timenow);
        deadline->tm_mday = day;
        deadline->tm_mon  = month - 1;
        deadline->tm_year = year  - 1900;
        if (difftime(timenow, mktime(deadline)) >= 0) {
            fprintf(stderr, "%s:%d: Deadline '%d.%d.%d' reached!", file, line, day, month, year);
            if (format) {
                fprintf(stderr, "  <-  ");
                va_list args;
                va_start(args, format);
                vfprintf(stderr, format, args);
                va_end(args);
            }
            fprintf(stderr, "\n");
            assert_break;
        }
    }
#else
    template<typename... T> Timebomb(int day, int month, int year, const char *file, int line, const char *format = nullptr, T... args) {
        time_t timenow = time(0);
        struct tm *deadline = localtime(&timenow);
        deadline->tm_mday = day;
        deadline->tm_mon  = month - 1;
        deadline->tm_year = year  - 1900;
        if (difftime(timenow, mktime(deadline)) >= 0) {
            fprintf(stderr, "%s:%d: Deadline '%d.%d.%d' reached!", file, line, day, month, year);
            if (format) {
                fprintf(stderr, "  <-  ");
                iml_fprintf(stderr, format, args...);
            }
            fprintf(stderr, "\n");
            assert_break;
        }
    }
#endif
};


#if 0
timebomb_assert(9,5,2020, "I am a global deadline!");

int main() {
    for (int i = 0; i < 5; ++i) {
        printf("loop at %d\n", i);
        timebomb_assert(9,5,2020, "You reached this deadline at %d", i);
    }
    
    timebomb_assert(1,5,2020, "You should have definately fixed xx %d till now!", x);
    
    return 0;
}
#endif

#endif // !BUILD_WITH_NO_CRT
#endif // IML_TIMEBOMB_H
