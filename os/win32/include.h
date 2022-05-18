#if !defined(IML_OS_WIN32_INCLUDE_H)
#define IML_OS_WIN32_INCLUDE_H



#if defined(_WINDOWS_) || defined(_INC_WINDOWS)
#  error "windows.h should not be included before os.h!"
#endif

#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#define _ALLOW_KEYWORD_MACROS
#define NOMINMAX
#define STRICT
#define WIN32_MEAN_AND_LEAN
#define WIN32_EXTRA_LEAN
#define VC_EXTRALEAN
//#undef _MSC_EXTENSIONS

#if IML_PROF_ENABLED
#  include <winsock2.h>
#endif

#include <windows.h>
#include <shellapi.h>
#include <dbghelp.h>

#pragma comment(lib, "dbghelp.lib")



#endif // IML_OS_WIN32_INCLUDE_H
