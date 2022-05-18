#if !defined(IML_RANDOM_H)
#define IML_RANDOM_H

#if defined(IML_NAMESPACE)
namespace IML_NAMESPACE {
#  if 0
}
#  endif
#endif
//
//~


// RNG = Random Number Generator


//
// @note Fast RNG
//

// George Marsaglia's XOR Shift random generator:
// https://en.wikipedia.org/wiki/Xorshift
u32 xorshift32() {
    //local_persist u32 state = __TIME__[7] | (__TIME__[6] << 4) | (__TIME__[4] << 8) | (__TIME__[3]);
    //local_persist u32 state = cast(u32) time(0);
    local_persist u32 state = 392607415;
    
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    
    return state;
}


//
// @note Good RNG
//

// @todo


//
//~
#if defined(IML_NAMESPACE)
#if 0
{
#endif
}
#endif

#endif//IML_RANDOM_H
