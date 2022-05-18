#if !defined(IML_TYPES_H)
#define IML_TYPES_H

#if defined(IML_NAMESPACE)
namespace IML_NAMESPACE {
#  if 0
}
#  endif
#endif
//
//~


#include "iml_base_defines.h"


//
// @note: Intrinsics
//
#if COMPILER_MSVC
#  include <intrin.h>
#  include <emmintrin.h>
#elif COMPILER_CLANG
#  include <x86intrin.h>
#elif COMPILER_GCC
#  include <immintrin.h>
#elif COMPILER_WEB // @todo Different name, WEB is the platform? Maybe EMSCRIPTEN?
//  @todo Compiler not supported at the moment.
#  include <emscripten/emscripten.h>
#else
#  error "SSE/NEON optimizations are not available for this compiler yet!!!!"
#endif


//
// @note: Includes :includes
//
#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

// @todo Move to OS??
#if OS_LINUX || OS_MAC
#  include <csignal>  // raise(SIGTRAP)
#  include <cstring>  // memcpy
#  include <unistd.h>
#endif


#include <stdint.h> // uint32_t, ...
#include <stddef.h> // ptrdiff_t
#include <float.h> // FLT_MIN/MAX, DBL_MIN/MAX

#if 0
#include <ctime>   // time, difftime, mktime
#include <cstdarg> // va_list, ...
#include <cstdlib> // for malloc/free on linux
#include <cstdio>  // printf, vfprintf
#include <cstring> // memcpy, memset, ...
//#include <stdio.h>
//#include <string.h> // strlen
#include "math.h"
#endif


//
// @note: Compiler compatability
//

// @note: Inlining
// MSVC has no guarantee that a function marked as inline is inlined, even when using __forceinline.
// VS2019 has an additional option for more aggressive inlining: /Ob3
// But still no guarantee!  "Because that would be so hard to implement in a compiler!!!" XD  _NOT_

// @note inline
#if COMPILER_MSVC && _MSC_VER <= 1800
#  define inline  __inline
#elif COMPILER_CLANG && !defined(__STDC_VERSION__)
#  define inline  __inline__
#else
#  if !defined(inline)
#    define inline  //static
#  endif
#endif

// @note force_inline
#if COMPILER_MSVC
#  if _MSC_VER < 1300
#    define force_inline  inline
#  else
#    define force_inline  __forceinline
#  endif
#else
#  define force_inline  __attribute__((__always_inline__))
#endif

// @note no_inline
#if COMPILER_MSVC
#  define no_inline  __declspec(noinline)
#else
#  define no_inline  __attribute__((noinline))
#endif

// @note thread_local
#if COMPILER_MSVC && _MSC_VER >= 1300
#  define thread_local  __declspec(thread)
#elif COMPILER_GNUC
#  define thread_local  __thread
#endif

// @note unused
#if COMPILER_MSVC
#  define unused(x)  (__pragma(warning(suppress : 4100))(x))
#elif COMPILER_GNUC
#  define unused(x)  __attribute__((__unused__))(x)
#else
#  define unused(x)  ((void)(sizeof(x)))
#endif

// @note maybe_unused
#if COMPILER_MSVC
#  if _HAS_CXX17
#    define maybe_unused(x)  [[maybe_unused]] x
#  else
#    define maybe_unused(x)
#  endif
#else
#  define maybe_unused(x)
#endif

// @note Debug break
#if COMPILER_MSVC
#  if _MSC_VER < 1300
#    define debug_break()  __asm int 3 /* Trap to debugger! */
#  else
#    define debug_break()  __debugbreak()
#  endif
#else // __GNUC__ or __clang__
#  if defined(__has_builtin)
#    if __has_builtin(__builtin_debugtrap)
#       define debug_break()  __builtin_trap()
#    else
#       define debug_break()  raise(SIGTRAP)
#    endif
#  else
#    define debug_break()  raise(SIGTRAP)
#  endif
#endif

#if COMPILER_MSVC
#  if _MSC_VER <= 1800
#    define snprintf  _snprintf
#  endif

#  if _MSC_VER <= 1900
#    define __VA_OPT__(x)
#  endif
#endif


//
// @note: alloca
//
#if !defined(alloca)
#  if defined(__GLIBC__) || defined(__sun) || defined(__APPLE__) || defined(__NEWLIB__)
#    include <alloca.h>  // (glibc uses <alloca.h>. Note that Cygwin may have _WIN32 defined, so the order matters here)
#  elif defined(_WIN32)
#    include <malloc.h>
#    if !defined(alloca)
#      define alloca _alloca
#    endif
#  else
#    include <stdlib.h>
#  endif
#endif


//
// @note: Types
//

#define function       static
#define api(x)

#if !defined(internal)
#  define internal     static
#endif
#define external       extern "C"

#define local_persist  static
#define global         static
#define local_const    static const
#define global_const   static const


#if !defined(null)
#  define null  0
#endif
#if !defined(nullptr)
#  define nullptr  0
#endif

// :includes #include <stdint.h>

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

typedef s8   b8;
typedef s16  b16;
typedef s32  b32;
typedef s64  b64;

typedef float   f32;
typedef double  f64;

typedef uintptr_t  uintptr;
typedef intptr_t   intptr;
typedef uintptr_t  umm;  // e.g., used for offsets into a memory block, memory size/counts, ...
typedef intptr_t   smm;  // e.g., used as results for pointer divisions  @todo same as signed_memory_index

typedef size_t     memory_index;
typedef ptrdiff_t  signed_memory_index; // @note cppreference.com: ptrdiff_t is used for pointer arithmetic and array indexing, if negative values are possible.

#define flag8(Type)   u8
#define flag16(Type)  u16
#define flag32(Type)  u32
#define flag64(Type)  u64

#define enum8(Type)   u8
#define enum16(Type)  u16
#define enum32(Type)  u32
#define enum64(Type)  u64

//~ @note: enum
#define enum_T(Name, T)     \
enum Name : T;                                                            \
constexpr Name operator + (const Name &a, const int &b) {             \
return static_cast<Name>(static_cast<T>(a) + static_cast<T>(b));  \
}                                                                     \
constexpr Name operator - (const Name &a, const int &b) {             \
return static_cast<Name>(static_cast<T>(a) - static_cast<T>(b));  \
}                                                                     \
constexpr Name operator += (Name &a, const int &b) {                  \
a = a + b;                                                        \
return a;                                                         \
}                                                                     \
constexpr Name operator -= (Name &a, const int &b) {                  \
a = a - b;                                                        \
return a;                                                         \
}                                                                     \
enum Name : T

#define enum_s8(Name)   enum_T(Name, s8)
#define enum_s16(Name)  enum_T(Name, s16)
#define enum_s32(Name)  enum_T(Name, s32)
#define enum_s64(Name)  enum_T(Name, s64)

#define enum_u8(Name)   enum_T(Name, u8)
#define enum_u16(Name)  enum_T(Name, u16)
#define enum_u32(Name)  enum_T(Name, u32)
#define enum_u64(Name)  enum_T(Name, u64)

//~ @note: enum_flags
#define enum_flags_T(Name, T)                                             \
enum Name;                                                            \
inline Name operator & (const Name &a, const Name &b) {               \
return static_cast<Name>(static_cast<T>(a) & static_cast<T>(b));  \
}                                                                     \
inline Name operator | (const Name &a, const Name &b) {               \
return static_cast<Name>(static_cast<T>(a) | static_cast<T>(b));  \
}                                                                     \
inline Name operator ^ (const Name &a, const Name &b) {               \
return static_cast<Name>(static_cast<T>(a) ^ static_cast<T>(b));  \
}                                                                     \
inline Name operator ~ (const Name &a) {                              \
return static_cast<Name>(~(static_cast<T>(a)));                   \
}                                                                     \
inline Name operator << (const Name &a, const T &b) {                 \
return static_cast<Name>(static_cast<T>(a) << b);                 \
}                                                                     \
inline Name operator >> (const Name &a, const T &b) {                 \
return static_cast<Name>(static_cast<T>(a) >> b);                 \
}                                                                     \
enum Name : T

#define enum_flags8(Name)   enum_flags_T(Name, u8)
#define enum_flags16(Name)  enum_flags_T(Name, u16)
#define enum_flags32(Name)  enum_flags_T(Name, u32)
#define enum_flags64(Name)  enum_flags_T(Name, u64)


//
// @note: Cast
//
#define cast(Type)  (Type)


//
// @note: Limits
//
// :includes #include <limits.h>

global_const s8  S8_MIN  = cast(s8)  cast(u8)  0x80;
global_const s16 S16_MIN = cast(s16) cast(u16) 0x8000;
global_const s32 S32_MIN = cast(s32) cast(u32) 0x80000000;
global_const s64 S64_MIN = cast(s64) cast(u64) 0x8000000000000000;

global_const s8  S8_MAX  = cast(s8)  0x7F;
global_const s16 S16_MAX = cast(s16) 0x7FFF;
global_const s32 S32_MAX = cast(s32) 0x7FFFFFFF;
global_const s64 S64_MAX = cast(s64) 0x7FFFFFFFFFFFFFFF;


global_const u8  U8_MIN  = 0;
global_const u16 U16_MIN = 0;
global_const u32 U32_MIN = 0;
global_const u64 U64_MIN = 0;

global_const u8  U8_MAX  = cast(u8)  -1;
global_const u16 U16_MAX = cast(u16) -1;
global_const u32 U32_MAX = cast(u32) -1;
global_const u64 U64_MAX = cast(u64) -1;


// :includes #include <float.h>

global_const f32 F32_MIN = -FLT_MAX;
global_const f32 F32_MAX =  FLT_MAX;

global_const f64 F64_MIN = -DBL_MAX;
global_const f64 F64_MAX =  DBL_MAX;


//
// @note: Other types and helpers
//

const f32 PI32  = 3.14159265359f;
const f32 TAU32 = 6.28318530717958647692f;


#define kilobytes(value)  ((value) * 1024LL)
#define megabytes(value)  (kilobytes(value) * 1024LL)
#define gigabytes(value)  (megabytes(value) * 1024LL)
#define terabytes(value)  (gigabytes(value) * 1024LL)
#define KB(x)  (kilobytes(x))
#define MB(x)  (megabytes(x))
#define GB(x)  (gigabytes(x))
#define TB(x)  (terabytes(x))


#define umm_from_pointer(pointer)     (cast(umm)(pointer))
#define pointer_from_umm(Type, value)  cast(Type *)(value)

#define u32_from_pointer(pointer)     (cast(u32)cast(memory_index)(pointer))
#define pointer_from_u32(Type, value)  cast(Type *)(cast(memory_index)(value))


#define CHAR_BIT  8
#define offset_of(Type, member)  (cast(umm) &((cast(Type *) 0)->member))
#define bit_size_of(Type)  (sizeof(Type) * CHAR_BIT)


// @note: S = Struct, m = member, ptr = pointer
#define ptr_dif(a,b) ((u8*)(a) - (u8*)(b))
//#define ptr_as_int(a) PtrDif(a,0)
//#define HandleAsU64(a) (u64)(PtrAsInt(a))
//#define Member(S,m) (((S*)0)->m)
//#define NullMember(S,m) (&Member(S,m))
//#define offset_of_member(S,m) PtrAsInt(&Member(S,m))
//#define OffsetOfMemberStruct(s,m) PtrDif(&(s)->m, (s))
//#define SizeAfterMember(S,m) (sizeof(S) - OffsetOfMember(S,m))
#define cast_from_member(S, m, ptr)  (S *)((u8 *)(ptr) - offset_of(S, m))
//#define IntAsPtr(a) (void*)(((u8*)0) + a)


//#define min(a, b)  (((a) < (b)) ? (a) : (b))
//#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define minimum(a, b)  (((a) < (b)) ? (a) : (b))
#define maximum(a, b)  (((a) > (b)) ? (a) : (b))

#define clamp_top(a, b)  minimum(a, b)
#define clamp_bot(a, b)  maximum(a, b)
#define clamp_(a, x, b)  ((a > x) ? a : ((b < x) ? b : x))
#define clamp(a, x, b)  clamp_((a), (x), (b))

/*
#define swap(a, b) do { \
auto tmp = (b); \
(b) = (a);      \
(a) = tmp;      \
} while (0)
*/


// inclusive range
#define in_range(a, x, b)      ((unsigned)((x) - (a)) <= (unsigned)((b) - (a)))
#define not_in_range(a, x, b)  ((unsigned)((x) - (a)) >  (unsigned)((b) - (a)))

#define is_space(c)  ((c) == ' ' || in_range('\t', (c), '\r')) // '\t' '\v' '\n' '\r'
#define is_digit(c)  in_range('0', (c), '9')
#define is_alpha(c)  in_range('a', ((c) | 32), 'z')


//
// @note: Array
//
#define array_count(a)  ((sizeof(a)) / (sizeof(*a)))
#define array_wrap(a, i)  ((a)[(i) % array_count(a)])
#define expand_array(a)    (a), (array_count(a))
// #define fix_size(s) struct{ u8 __size_fixer__[s]; }


#define loop    for(;;)
#define For(x)  for (const auto &it: x)


//
// @note: Alignment
//
// @note: Added (value-value) here to force integral promotion to the size of value.
#define align_pow2(value, alignment)  ((value + ((alignment) - 1)) & ~((value - value) + (alignment) - 1))
#define align_4(value)   ((value + 3)  & ~3)
#define align_8(value)   ((value + 7)  & ~7)
#define align_16(value)  ((value + 15) & ~15)
#define align_32(value)  ((value + 31) & ~31)
#define align_64(value)  ((value + 63) & ~63)

inline b32 is_pow2(u32 value) {
    //b32 result = ((value & ~(value - 1)) == value);  // This version thinks 0 is a pow2
    b32 result = value && !(value & (value - 1));
    return result;
}


//
// @note: Macro stuff
//
#define STRINGIFY_(x)  #x
#define STRINGIFY(x)  STRINGIFY_(x)

#define CONCAT_INTERNAL(x, y)  x ## y
#define CONCAT(x, y)  CONCAT_INTERNAL(x,y)


// @note: Macro block{} to regular statement statement
//        https://stackoverflow.com/questions/1067226/c-multi-line-macro-do-while0-vs-scope-block
// For example:
//   if (1)  assert(false);
//   else    assert(true);  // error C2181: illegal else without matching if
#define STMNT(s) do { s } while(0)



// @note: Filename and line number
#define LINE_NUMBER_STRING    STRINGIFY(__LINE__)
#define FILE_AND_LINE_STRING  __FILE__ ":" LINE_NUMBER_STRING ": "


// @note: Workaround, because msvc compiler is broken!!!
#define MACRO_EXPAND(x)  x
#define MACRO_GET_ARG1(_1, ...)  _1


//
// @note: defer
//
#if !IML_NO_DEFER_IMPLEMENTATION
template<typename T>
struct Exit_Scope {
    T lambda;
    force_inline Exit_Scope(T lambda): lambda(lambda) {}
    force_inline ~Exit_Scope() {
        lambda();
    }
    force_inline Exit_Scope(const Exit_Scope&);
    private:
    force_inline Exit_Scope& operator=(const Exit_Scope&);
};

struct Exit_Scope_Help {
    template<typename T> force_inline Exit_Scope<T> operator+(T t) {
        return t;
    }
};
#undef IML_NO_DEFER_IMPLEMENTATION
#endif

#if COMPILER_MSVC
#  define defer  const auto& CONCAT(defer__, __LINE__) = Exit_Scope_Help() + [&]()
// @note Disable compiler warning: local variable is initialized but not referenced
#  pragma warning(disable : 4189)
#else // __GNUC__ or __clang__
#  define defer  const auto& __attribute__((unused)) CONCAT(defer__, __LINE__) = Exit_Scope_Help() + [&]()
#endif


//
// @note: Assert
//
#if BUILD_WITH_ASSERTS
// @note Debug:
#  define assert(c, ...)  assert_always(c, ##__VA_ARGS__)
#  define assert_message(m, ...)   assert_message_always(m, ##__VA_ARGS__)
#  if !_HAS_CXX17 && COMPILER_MSVC
#    define static_assert(c, ...)  static_assert_always(c, ##__VA_ARGS__)
#  endif
#else
// @note Release:
#  define assert(...)
#  define assert_message(...)
#  undef  static_assert
#  define static_assert(...)
#endif

#define if_assert(c)          assert(c); if (!(static_cast<bool>(c)))
#define assert_implies(a, b)  assert(!(a) || (b))
#define not_implemented  assert_message("Not implemented!")
#define invalid_path     assert_message("Invalid path!")
#define invalid_default_case  default: { invalid_path; } break
#define dont_compile  static_assert_always(!"No seriously don't compile!!!'") // __NoSeriouslyDontCompile__


//#define assert_break  (*((volatile s32 *)0) = 0xA11E)
#define assert_break  debug_break()

//#define assert_always(c, ...)  STMNT( if (!(c)) { assert_break; } )
#define assert_always(c, ...)  assert_log_always(c, ##__VA_ARGS__)
#define assert_message_always(m, ...)  assert_log_always(false, m, ##__VA_ARGS__) // assert_break; // assert_always(!m)
#define static_assert_always(c, ...)   typedef char CONCAT(__static_assert,__LINE__)[(c) ? 1 : -1]


//
// @note: Assert with logging and format string
//
#define assert_log_always(c, ...)  do { \
if (!(static_cast<bool>(c))) { \
do_assert_print(#c, __FILE__, __LINE__, __FUNCTION__); \
iml_maybe_print_message_and_args("    Message: ", ##__VA_ARGS__); \
iml_print_stack_trace(); \
assert_break; \
} \
} while (0)

no_inline void do_assert_print(const char *condition, const char *filename, int line_number, const char *function_name);
void iml_print_stack_trace(void);

#if !IML_ASSERT_INCLUDE_STACK_TRACE
void iml_print_stack_trace(void) {}
#endif

//~ @note: Logging stuff
// Usage: iml_maybe_print_message_and_args("\nError: ", ##__VA_ARGS__);
template<typename... T>
no_inline void iml_maybe_print_message_and_args(const char *prefix = nullptr, const char *format = nullptr, T... args);


//
// @note notepad_log
//
template<typename... T>
void notepad_log(const char *format, T... args);


//
// @note: Zeroing
//
#define zero_struct(instance)       zero_size(sizeof(instance),             &(instance))
#define zero_array(count, pointer)  zero_size(count * sizeof((pointer)[0]), pointer)

internal void
zero_size(memory_index size, void *data) {
    u8 *byte = cast(u8 *)data;
    while (size--) {
        *byte++ = 0;
    }
}


//
// @note: Debug initialize test
//
#if BUILD_DEBUG
#  define DEBUG_INITIALIZER_DATA    b32 _debug_is_initialized = false;
#  define DEBUG_INITIALIZE          _debug_is_initialized = true;
#  define DEBUG_ASSERT_INITIALIZED  assert(_debug_is_initialized, "Data-Structure was not initialized!")
#else
#  define DEBUG_INIT_DATA
#  define DEBUG_INITIALIZE
#  define DEBUG_ASSERT_INITIALIZED
#endif


//
// @note: Data-Structures
//
struct Node{
    Node *next;
    Node *prev;
};
union SNode{
    SNode *next;
    SNode *prev;
};

//~ @note: Doubly-Linked-List
#define dll_sentinel_initializer_NP_(s,next,prev)  { .next=s, .prev=s }
#define dll_init_sentinel_NP_(s,next,prev)  s->next=s, s->prev=s
#define dll_insert_NP_(p,n1,n2,next,prev)   n2->next=p->next, n1->prev=p, p->next->prev=n2, p->next=n1
#define dll_remove_NP_(n1,n2,next,prev)     n2->next->prev=n1->prev, n1->prev->next=n2->next, n2->next=n1->prev=0

#define dll_sentinel_initializer_(s)        dll_sentinel_initializer_NP_(s,next,prev)
#define dll_init_sentinel_(s)               dll_init_sentinel_NP_(s,next,prev)
#define dll_insert_(p,n)                    dll_insert_NP_(p,n,n,next,prev)
#define dll_insert_multiple_(p,n1,n2)       dll_insert_NP_(p,n1,n2,next,prev)
#define dll_insert_back_(p,n)               dll_insert_NP_(p,n,n,prev,next)
#define dll_insert_multiple_back_(p,n1,n2)  dll_insert_NP_(p,n2,n1,prev,next)
#define dll_remove_(n)                      dll_remove_NP_(n,n,next,prev)
#define dll_remove_multiple_(n1,n2)         dll_remove_NP_(n1,n2,next,prev)

#define dll_sentinel_initializer(s)         dll_sentinel_initializer_((s))
#define dll_init_sentinel(s)               (dll_init_sentinel_((s)))
#define dll_insert(p,n)                    (dll_insert_((p),(n)))
#define dll_insert_multiple(p,n1,n2)       (dll_insert_multiple_((p),(n1),(n2)))
#define dll_insert_back(p,n)               (dll_insert_back_((p),(n)))
#define dll_insert_multiple_back(p,n1,n2)  (dll_insert_multiple_back_((p),(n1),(n2)))
#define dll_remove(n)                      (dll_remove_((n)))
#define dll_remove_multiple(n1,n2)         (dll_remove_multiple_((n1),(n2)))

//~ @note: Single-Linked-List Stack/Queue
#define sll_stack_push_(h,n) n->next=h,h=n
#define sll_stack_pop_(h) h=h=h->next
#define sll_queue_push_multiple_(f,l,ff,ll) if(ll){if(f){l->next=ff;}else{f=ff;}l=ll;l->next=0;}
#define sll_queue_push_(f,l,n) sll_queue_push_multiple_(f,l,n,n)
#define sll_queue_pop_(f,l) if (f==l) { f=l=0; } else { f=f->next; }

#define sll_stack_push(h,n) (sll_stack_push_((h),(n)))
#define sll_stack_pop(h) (sll_stack_pop_((h)))
#define sll_queue_push_multiple(f,l,ff,ll) STMNT( sll_queue_push_multiple_((f),(l),(ff),(ll)) )
#define sll_queue_push(f,l,n) STMNT( sll_queue_push_((f),(l),(n)) )
#define sll_queue_pop(f,l) STMNT( sll_queue_pop_((f),(l)) )

//~
#define zdll_push_back_NP_(f,l,n,next,prev) ((f==0) ? (n->next=n->prev=0,f=l=n) : (n->prev=l,n->next=0,l->next=n,l=n))
#define zdll_remove_back_NP_(f,l,next,prev) ((f==l) ? (f=l=0) : (l->prev->next=0,l=l->prev))
#define zdll_remove_NP_(f,l,n,next,prev)              \
((l==n) ? (zdll_remove_back_NP_(f,l,next,prev))   \
: (f==n) ? (zdll_remove_back_NP_(l,f,prev,next)) \
:          (dll_remove_NP_(n,n,next,prev)))

#define zdll_push_back(f,l,n) zdll_push_back_NP_((f),(l),(n),next,prev)
#define zdll_push_front(f,l,n) zdll_push_back_NP_((l),(f),(n),prev,next)
#define zdll_remove_back(f,l) zdll_remove_back_NP_((f),(l),next,prev)
#define zdll_remove_front(f,l) zdll_remove_back_NP_((l),(f),prev,next)
#define zdll_remove(f,l,n) zdll_remove_NP_((f),(l),(n),next,prev)

#define zdll_assert_good(T,f) do { \
if (f != 0) { \
assert(f->prev == 0); \
for(T *p_ = f; p_ != 0; p_ = p_->next) { \
assert(p_->prev == 0 || p_->prev->next == p_); \
assert(p_->next == 0 || p_->next->prev == p_); \
} \
} \
} while (0)



//
// @note: Safe truncate helpers
//

//~ @note: Signed
inline s32 safe_truncate_to_s32(s64 value) {
    assert(value <= S32_MAX);
    return static_cast<s32>(value);
}

inline s16 safe_truncate_to_s16(s64 value) {
    assert(value <= S16_MAX);
    return static_cast<s16>(value);
}
inline s16 safe_truncate_to_s16(s32 value) {
    assert(value <= S16_MAX);
    return static_cast<s16>(value);
}

inline s8 safe_truncate_to_s8(s64 value) {
    assert(value <= S8_MAX);
    return static_cast<s8>(value);
}
inline s8 safe_truncate_to_s8(s32 value) {
    assert(value <= S8_MAX);
    return static_cast<s8>(value);
}
inline s8 safe_truncate_to_s8(s16 value) {
    assert(value <= S8_MAX);
    return static_cast<s8>(value);
}

//~ @note: Unsigned
inline u32 safe_truncate_to_u32(u64 value) {
    assert(value <= U32_MAX);
    return static_cast<u32>(value);
}

inline u16 safe_truncate_to_u16(u64 value) {
    assert(value <= U16_MAX);
    return static_cast<u16>(value);
}
inline u16 safe_truncate_to_u16(u32 value) {
    assert(value <= U16_MAX);
    return static_cast<u16>(value);
}

inline u8 safe_truncate_to_u8(u64 value) {
    assert(value <= U8_MAX);
    return static_cast<u8>(value);
}
inline u8 safe_truncate_to_u8(u32 value) {
    assert(value <= U8_MAX);
    return static_cast<u8>(value);
}
inline u8 safe_truncate_to_u8(u16 value) {
    assert(value <= U8_MAX);
    return static_cast<u8>(value);
}


//
// @note: Rounding
//
inline s8
round_up_s8(s8 x, s8 b){
    x += b - 1;
    x -= x%b;
    return(x);
}
inline u8
round_up_u8(u8 x, u8 b){
    x += b - 1;
    x -= x%b;
    return(x);
}
inline s16
round_up_s16(s16 x, s16 b){
    x += b - 1;
    x -= x%b;
    return(x);
}
inline u16
round_up_u16(u16 x, u16 b){
    x += b - 1;
    x -= x%b;
    return(x);
}
inline s32
round_up_s32(s32 x, s32 b){
    x += b - 1;
    x -= x%b;
    return(x);
}
inline u32
round_up_u32(u32 x, u32 b){
    x += b - 1;
    x -= x%b;
    return(x);
}
inline s64
round_up_s64(s64 x, s64 b){
    x += b - 1;
    x -= x%b;
    return(x);
}
inline u64
round_up_u64(u64 x, u64 b){
    x += b - 1;
    x -= x%b;
    return(x);
}

inline s8
round_down_s8(s8 x, s8 b){
    x -= x%b;
    return(x);
}
inline u8
round_down_u8(u8 x, u8 b){
    x -= x%b;
    return(x);
}
inline s16
round_down_s16(s16 x, s16 b){
    x -= x%b;
    return(x);
}
inline u16
round_down_u16(u16 x, u16 b){
    x -= x%b;
    return(x);
}
inline s32
round_down_s32(s32 x, s32 b){
    x -= x%b;
    return(x);
}
inline u32
round_down_u32(u32 x, u32 b){
    x -= x%b;
    return(x);
}
inline s64
round_down_s64(s64 x, s64 b){
    x -= x%b;
    return(x);
}
inline u64
round_down_u64(u64 x, u64 b){
    x -= x%b;
    return(x);
}


//
// @note: Find
//
template <typename T>
T find_lowest(T *array, u64 count) {
    T lowest = array[0];
    
    for (u64 i = 0; i < count; ++i) {
        auto it = array[i];
        if (it < lowest) {
            lowest = it;
        }
    }
    
    return lowest;
}

template <typename T>
T find_highest(T *array, u64 count) {
    T highest = array[0];
    
    for (u64 i = 0; i < count; ++i) {
        auto it = array[i];
        if (it > highest) {
            highest = it;
        }
    }
    
    return highest;
}

template <typename T>
void find_lowest_highest(T *lowest_out, T *highest_out, T *array, u64 count) {
    T lowest  = array[0];
    T highest = array[0];
    
    for (u64 i = 0; i < count; ++i) {
        auto it = array[i];
        if (it < lowest)   lowest  = it;
        if (it > highest)  highest = it;
    }
    
    *lowest_out  = lowest;
    *highest_out = highest;
}



//
// @note: Size checks
//
static_assert(sizeof(u64) == 8);
static_assert(sizeof(s64) == 8);
static_assert(sizeof(f64) == 8);
static_assert(sizeof(u32) == 4);
static_assert(sizeof(s32) == 4);
static_assert(sizeof(f32) == 4);

#if 1
typedef bool __check_int____size32__[sizeof(int)    == 4  ? 1  : -1];
typedef bool __check_bool___size8___[sizeof(bool)   == 1  ? 1  : -1];
typedef bool __check_float__size32__[sizeof(float)  == 4  ? 1  : -1];
typedef bool __check_double_size64__[sizeof(double) == 8  ? 1  : -1];
#else
static_assert(sizeof(size_t) == 8, "Compiling for 32-bit not supported!");
#endif


#if 0
// :includes #include <limits.h>

static_assert(U8_MAX  == UCHAR_MAX);
static_assert(U16_MAX == USHRT_MAX);
static_assert(U32_MAX == ULONG_MAX);
static_assert(U64_MAX == ULLONG_MAX);


static_assert(S8_MIN == SCHAR_MIN);
static_assert(S8_MAX == SCHAR_MAX);

static_assert(S16_MIN == SHRT_MIN);
static_assert(S16_MAX == SHRT_MAX);

static_assert(S32_MIN == LONG_MIN);
static_assert(S32_MAX == LONG_MAX);

static_assert(S64_MIN == LLONG_MIN);
static_assert(S64_MAX == LLONG_MAX);


// static_assert(F32_MIN == FLT_MIN);
static_assert(F32_MIN == (-FLT_MAX));
static_assert(F32_MAX == FLT_MAX);

//static_assert(F64_MIN == DBL_MIN);
static_assert(F64_MIN == (-DBL_MAX));
static_assert(F64_MAX == DBL_MAX);
#endif


#ifdef IML_NAMESPACE
#if 0
{
#endif
}
#endif

#endif // IML_TYPES_H


//
// @note: Implementation
//
#ifdef IML_TYPES_IMPLEMENTATION
#undef IML_TYPES_IMPLEMENTATION
#if !defined(IML_TYPES_CPP)
#define IML_TYPES_CPP


#include "iml_printf.h"
#include "iml_terminal.h"


//
// @note: Assert with logging and format string
//
no_inline void do_assert_print(const char *condition, const char *filename, int line_number, const char *function_name) {
    auto colors = get_terminal_colors();
    fprintf(stderr,
            "\n\n"
            "%s*** Assertion failed:%s '%s'\n"
            "    %sOn file:%s %s\n"
            "    %sOn line:%s %d\n",
            colors.red,   colors.reset, condition,
            colors.green, colors.reset, filename,
            colors.green, colors.reset, line_number);
}


//
// @note: Logging stuff
//
// Usage iml_maybe_print_message_and_args(##__VA_ARGS__);
// :includes #include <cstdarg> // va_list, ...
// :includes #include <cstdio>  // vfprintf
template<typename... T>
no_inline void iml_maybe_print_message_and_args(const char *prefix, const char *format, T... args) {
    if (format && string_length(format) > 0) {
        if (prefix) {
            auto colors = get_terminal_colors();
            fprintf(stderr, "%s%s%s", colors.green, prefix, colors.reset);
        }
        // TODO iml_fprintf(stderr, format, args...);
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}

#if 0
no_inline void iml_maybe_print_message_and_args(const char *prefix = nullptr, const char *format = nullptr, ...) {
    if (!format || string_length(format) <= 1)  return;
    if (prefix) {
        fprintf(stderr, "%s", prefix);
    }
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}
#endif


//
// @note notepad_log
//
template<typename... T>
void notepad_log(const char *format, T... args) {
#if COMPILER_MSVC && defined(_WINDOWS_)
    local_persist u8 buffer[4096]; // @thread_safety
    Buffer output = { array_count(buffer), buffer };
    iml_snprintf(&output, format, args...);
    snprint(&output, '\0');
    
    HWND notepad = FindWindow(NULL, "Untitled - Notepad");
    if (!notepad) {
        notepad  = FindWindow(NULL, "*Untitled - Notepad");
    }
    if (!notepad)  return;
    
    HWND edit = FindWindowEx(notepad, NULL, "EDIT", NULL);
    if (!edit)  return;
    
    // @note Set cursor to end-of-file
    {
        //int text_length = GetWindowTextLengthA(edit);
        SendMessage(edit, EM_SETSEL, S32_MAX, S32_MAX);
    }
    
    SendMessage(edit, EM_REPLACESEL, TRUE, cast(LPARAM)buffer);
#endif
}



//
// @note: Tests
//
#include "iml_test_suit.h"

global_unit("iml_types") {
    unit("Struct/Pointer helpers") {
        test("@todo") {};  // @todo
    };
    
    unit("Base Data-Structures") {
        test("@todo") {};  // @todo
        
#if 0
        unit("dll") {
            struct Free_Node {
                union {
                    Free_Node *next;
                    Node node;
                };
                
                Data data;
            };
            Node free_list = dll_sentinel_initializer(&free_list);
            
            // @note: The optimized dissasembly of those 2 ways to go through the list look exactly the same.
            void search() {
                for (Node *node = free_list.next;
                     node != &free_list;
                     node = node->next) {
                    assert(node);
                    Free_Node *free_node = cast_from_member(Free_Node, node, node);
                    dll_remove(free_node->node)
                }
            }
            void search2() {
                for (Free_Node *free_node = cast_from_member(Free_Node, node, free_list.next);
                     &free_node->node != &free_list;
                     free_node = free_node->next) {
                    assert(free_node);
                    dll_remove(free_node);
                }
            }
            
            //
            //
            //
            
            struct Free_Node {
                Free_Node *next;
                Free_Node *prev;
                Data data;
            };
            Free_Node free_list = {
                .next = &free_list,
                .prev = &free_list,
                .data = { .data = null, .size = 0 },
            };
            
            dll_insert_back(&free_list, node);
            
            void search() {
                for (Free_Node *free_node = free_list.next;
                     free_node != &free_list;
                     free_node = free_node->next) {
                    assert(free_node);
                    dll_remove(free_node);
                }
            }
        };
#endif
    };
};


#endif // IML_TYPES_CPP
#endif // IML_TYPES_IMPLEMENTATION
