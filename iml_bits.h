#if !defined(IML_BITS_H)
#define IML_BITS_H

#if defined(IML_NAMESPACE)
namespace IML_NAMESPACE {
#  if 0
}
#  endif
#endif
//
//~


#if defined(ASMLIB_H)
#  define USE_ASMLIB  1
#else
#  define USE_ASMLIB  0
#endif


//
// @note: Set/Clear/... bits macros
//
// w = word to modify
#define BIT_SET_WITH_MASK(w, mask)     ((w) |=  (mask))
#define BIT_CLEAR_WITH_MASK(w, mask)   ((w) &= ~(mask))
#define BIT_TOGGLE_WITH_MASK(w, mask)  ((w) ^=  (mask))
#define BIT_CHECK_WITH_MASK(w, mask)   ((w) &   (mask))
#define BIT_SET_VALUE_WITH_MASK(w, mask, value)  ((w) = (((w) & ~(mask)) | ((-(u64)(!!(value))) & (mask))))

#define BIT_SET_AT_INDEX(w, index)     BIT_SET_WITH_MASK((w),    ((u64)1 << (index)))
#define BIT_CLEAR_AT_INDEX(w, index)   BIT_CLEAR_WITH_MASK((w),  ((u64)1 << (index)))
#define BIT_TOGGLE_AT_INDEX(w, index)  BIT_TOGGLE_WITH_MASK((w), ((u64)1 << (index)))
#define BIT_CHECK_AT_INDEX(w, index)   BIT_CHECK_WITH_MASK((w),  ((u64)1 << (index)))
#define BIT_SET_VALUE_AT_INDEX(w, index, value)  BIT_SET_VALUE_WITH_MASK((w), ((u64)1 << (index)), (value))

inline u64 set_bits_with_mask(u64 w, u64 mask, bool value) {
    BIT_SET_VALUE_WITH_MASK(w, mask, value);
    return w;
}
inline u64 set_bit_at_index(u64 w, u64 index, bool value) {
    BIT_SET_VALUE_AT_INDEX(w, index, value);
    return w;
}

// @todo BITMASK_*...


//
// @note: Count bits set / popcount
//
// A_popcount is about 2x faster.
//
u32 count_bits_set_dumb(u32 v) {
    u32 num_bits = 0;
    while (v) {
        num_bits += 1;
        v &= v - 1;
    }
    return num_bits;
}

u32 count_bits_set_fast(u32 v) {
    v = v - ((v >> 1) & 0x55555555);                    // reuse input as temporary
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333);     // temp
    u32 c = ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24; // count
    return c;
}

template <typename T>
T count_bits_set_fast_template(T v) {
    v = v - ((v >> 1) & (T)~(T)0/3);
    v = (v & (T)~(T)0/15*3) + ((v >> 2) & (T)~(T)0/15*3);
    v = (v + (v >> 4)) & (T)~(T)0/255*15;
    T c = (T)(v * ((T)~(T)0/255)) >> (sizeof(T) - 1) * CHAR_BIT;
    return c;
}

constexpr u64 constexpr_count_bits_set(u64 v) {
    v = v - ((v >> 1) & (u64)~(u64)0/3);
    v = (v & (u64)~(u64)0/15*3) + ((v >> 2) & (u64)~(u64)0/15*3);
    v = (v + (v >> 4)) & (u64)~(u64)0/255*15;
    u64 c = (u64)(v * ((u64)~(u64)0/255)) >> (sizeof(u64) - 1) * CHAR_BIT;
    return c;
}


inline u8 count_bits_set(u8 v) {
#if USE_ASMLIB
    return A_popcount(v);
#else
    return count_bits_set_fast_template<u8>(v);
#endif
}

inline u16 count_bits_set(u16 v) {
#if USE_ASMLIB
    return A_popcount(v);
#else
    return count_bits_set_fast_template<u16>(v);
#endif
}

inline u32 count_bits_set(u32 v) {
#if USE_ASMLIB
    return A_popcount(v);
#else
    return count_bits_set_fast(v);
#endif
}

inline u64 count_bits_set(u64 v) {
    return count_bits_set_fast_template<u64>(v);
}


//
// @note: Bit permutations
//
// Stack overflow: https://stackoverflow.com/questions/35115478/specific-binary-permutation-generating-function
// See also: Hamming Weight
inline u64 get_last_set_bit(u64 ui) {
    return ui & -ui;
}

// next_with_same_popcount(ui) finds the next larger integer with the same
// number of 1-bits as ui. If there isn't one (within the range
// of the unsigned type), it returns 0.
inline u64 next_bit_permutation_with_same_popcount(u64 ui) {
    u64 lo = get_last_set_bit(ui);
    u64 next = ui + lo;
    if (next) {
        u64 hi = get_last_set_bit(next);
        next += (hi >> __builtin_ffsll(lo)) - 1;
    }
    return next;
}

inline u64 next_bit_permutation_with_popcount_k(u64 ui, u64 k) {
    u64 count;
#if 0
    while ((count = __builtin_popcountll(ui)) > k) {
        ui += get_last_set_bit(ui);
    }
#else
    while ((count = __builtin_popcountll(ui)) > k) {
        u64 lo = get_last_set_bit(ui);
        ui += get_last_set_bit(ui - lo) - lo;
    }
#endif
    for (u64 i = count; i < k; ++i) {
        ui += get_last_set_bit(~ui);
    }
    return ui;
}



//
// @note: Tests
//
#include "iml_test_suit.h"

global_unit("iml_bits") {
    test("@todo") {};  // @todo
    
    test("set_bits_with_mask(w, mask, value)") {
        u64 base, mask, expected, result;
        
        base     = 0x10011001;
        mask     = 0x00101010;
        expected = 0x10111011;
        result = set_bits_with_mask(base, mask, true);
        check(result == expected);
        
        base     = 0x10111011;
        mask     = 0x00101010;
        expected = 0x10010001;
        result = set_bits_with_mask(base, mask, false);
        check(result == expected);
    };
    
    test("set_bit_at_index(w, index, value)") {
        // Set to true.
        for (u64 i = 0; i < 64; i++) {
            u64 expected = (cast(u64) 1 << i);
            u64 result = set_bit_at_index(0, i, true);
            check(result == expected);
        }
        
        // Set to false.
        for (u64 i = 0; i < 64; i++) {
            u64 expected = U64_MAX ^ ((cast(u64) 1 << i));
            u64 result = set_bit_at_index(U64_MAX, i, false);
            check(result == expected);
        }
    };
};



//
//~
#ifdef IML_NAMESPACE
#if 0
{
#endif
}
#endif

#endif//IML_BITS_H
