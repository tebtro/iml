#if !defined(IML_BASE_DEFINES_H)
#define IML_BASE_DEFINES_H


//
// @note: Compiler flags:
//
/*
BUILD_DEBUG:
- 0 Build for public release
- 1 Build for developer only
BUILD_RELEASE:

BUILD_SLOW:
- 0 Not slow code allowed!
- 1 Slow code welcome.
*/


//
// @note: Build defines
//
#if !defined(BUILD_DEBUG)
#  define BUILD_DEBUG  0
#endif
#if !defined(BUILD_RELEASE)
#  define BUILD_RELEASE  0
#endif

// Default to BUILD_DEBUG
#if !BUILD_DEBUG && !BUILD_RELEASE
#  undef  BUILD_DEBUG
#  define BUILD_DEBUG  1
#endif


#if !defined(BUILD_SLOW)
#  define BUILD_SLOW  BUILD_DEBUG
#endif

#if !defined(BUILD_WITH_ASSERTS)
#  define BUILD_WITH_ASSERTS  BUILD_SLOW
#endif
#if !defined(BUILD_WITH_PROFILING)
#  define BUILD_WITH_PROFILING  BUILD_SLOW
#endif
#if !defined(BUILD_TESTS)
#  define BUILD_TESTS  BUILD_SLOW
#endif


#if BUILD_RELEASE && !defined(NDEBUG)
#  define NDEBUG
#endif


//
// @note: Compilers, OS's (platforms), and architectures
//
// Info about predefined macros for compiler, platform, architecture, ... :
//     https://blog.kowalczyk.info/article/j/guide-to-predefined-macros-in-c-compilers-gcc-clang-msvc-etc..html
//

// @todo MinGW, Emscripten, ...
// @todo Android, AsmJs, ...

//~ MSVC
#if defined(_MSC_VER)
#  define COMPILER_MSVC  1

#  if defined(_WIN32)  // _WIN64 implies _WIN32
#    define OS_WINDOWS  1
#  else
#    error "This compiler/platform combo is not supported yet!"
#  endif

#  if defined(_M_AMD64)
#    define ARCH_X64  1
#  elif defined(_M_IX86)
#    define ARCH_X86  1
#  elif defined(_M_ARM64)
#    define ARCH_ARM64  1
#  elif defined(_M_ARM)
#    define ARCH_ARM32  1
#  else
#    error "Architecture not supported yet!"
#  endif

//~ CLANG
#elif defined(__clang__)
#  define COMPILER_CLANG  1

#  if defined(__APPLE__) && defined(__MACH__)  // Darwin (Max OS X and iOS)
#    define OS_MAC  1
#  else
#    error "This compiler/platform combo is not supported yet!"
#  endif

#  if defined(__amd64) || defined(__amd64__) || defined(__x86_64) || defined(__x86_64__)
#    define ARCH_X64  1
#  elif defined(i386) || defined(__i386) || defined(__i386__)
#    define ARCH_X86  1
#  elif defined(__aarch64__)
#    define ARCH_ARM64  1
#  elif defined(__arm__)
#    define ARCH_ARM32  1
#  else
#    error "Architecture not supported yet!"
#  endif

//~ GCC
#elif defined(__GNUC__) || defined(__GNUG__)
#  define COMPILER_GCC  1

#  if defined(__gnu_linux__)
#    define OS_LINUX  1
#  else
#    error "This compiler/platform combo is not supported yet!"
#  endif

#  if defined(__amd64) || defined(__amd64__) || defined(__x86_64) || defined(__x86_64__)
#    define ARCH_X64  1
#  elif defined(i386) || defined(__i386) || defined(__i386__)
#    define ARCH_X86  1
#  elif defined(__aarch64__)
#    define ARCH_ARM64  1
#  elif defined(__arm__)
#    define ARCH_ARM32  1
#  else
#    error "Architecture not supported yet!"
#  endif

//~ Unknown compiler
#else
#  error "This compiler is not supported yet!"
#endif


//
// @note
//
#if defined(ARCH_X64)
#  define ARCH_64BIT  1
#elif defined(ARCH_X86)
#  define ARCH_32BIT  1
#endif


//
// @note: Zeroify
//
// Compilers
#if !defined(COMPILER_MSVC)
#  define COMPILER_MSVC  0
#endif
#if !defined(COMPILER_CLANG)
#  define COMPILER_CLANG  0
#endif
#if !defined(COMPILER_GCC)
#  define COMPILER_GCC  0
#endif

// Platforms/OS's
#if !defined(OS_WINDOWS)
#  define OS_WINDOWS  0
#endif
#if !defined(OS_LINUX)
#  define OS_LINUX  0
#endif
#if !defined(OS_MAC)
#  define OS_MAC  0
#endif

// Architectures
#if !defined(ARCH_32BIT)
#  define ARCH_32BIT  0
#endif
#if !defined(ARCH_64BIT)
#  define ARCH_64BIT  0
#endif

#if !defined(ARCH_X64)
#  define ARCH_X64  0
#endif
#if !defined(ARCH_X86)
#  define ARCH_X86  0
#endif

#if !defined(ARCH_ARM64)
#  define ARCH_ARM64  0
#endif
#if !defined(ARCH_ARM32)
#  define ARCH_ARM32  0
#endif


//
// @note: Calling convention
//
#if ARCH_32BIT && OS_WINDOWS
#  define CALL_CONVENTION  __stdcall
#else
#  define CALL_CONVENTION
#endif


//
// @note: Endianness
//
#if OS_WINDOWS
#  define LITTLE_ENDIAN  1
#else
#  if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#    define LITTLE_ENDIAN  1
#  else
#    define LITTLE_ENDIAN  0
#  endif
#endif


#endif // IML_BASE_DEFINES_H
