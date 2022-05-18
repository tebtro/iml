#if !defined(IML_MATH_H)
#define IML_MATH_H

#if defined(IML_NAMESPACE)
namespace IML_NAMESPACE {
#  if 0
}
#  endif
#endif
//
//~


#if BUILD_WITH_NO_CRT


#define PI  3.14159265359f
#define TAU 6.28318530717958647692f


// @todo Vectors, ...

// @todo f16
// _mm_sqrt_sh  sqrt of single half-precision floating point value.

inline f64 sqrt(f64 real) {
    f64 result = _mm_cvtsd_f64(_mm_sqrt_sd(_mm_set_sd(0), _mm_set_sd(real)));
    return result;
}

inline f32 sqrt(f32 real) {
    f32 result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(real)));
    return result;
}

#endif


//
// @note: Tests
//
#include "iml_test_suit.h"

global_unit("Math") {
    test("@todo") {};  // @todo
};


//
//~
#ifdef IML_NAMESPACE
#if 0
{
#endif
}
#endif

#endif
