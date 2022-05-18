typedef void *HANDLE;
typedef int BOOL;
typedef int DWORD;

typedef unsigned      int UINT;
typedef unsigned long int ULONG;

#ifdef _WIN64
typedef unsigned __int64  SIZE_T;
#else
typedef _W64 unsigned int SIZE_T;
#endif


void ExitProcess(UINT uExitCode);

HANDLE GetProcessHeap(void);
void *HeapAlloc(HANDLE heap, DWORD flags, SIZE_T bytes);
BOOL HeapFree(HANDLE heap, DWORD flags, void *memory);



//
// @note: Floating point
//

int _fltused = 0x9875;


//
// @note: Thread-Local-Storage
//
ULONG _tls_index = 0;


//
// @note Agner Fog asmlib
//
//#pragma comment(lib, "../src/asmlib/libacof64.lib")
//#pragma comment(lib, "../src/asmlib/libacof64o.lib") // @note Override standard library
#include "asmlib/asmlib.h"

// -Oi- to make sure the asmlib functions are called.

char * strstr(char * const haystack, char const * const needle) {
    return A_strstr((char*)haystack, needle);} // Overload A_strstr with const char * version

#pragma warning(disable : 4164)  // 'strcmp': intrinsic function not declared
#pragma function(strcmp)
int strcmp(const char * a, const char * b) {
    return A_strcmp(a, b);
}


//~
// Compiler flag -Oi is required.
// -Oi (Generate Intrinsic Functions)
#if 0
typedef unsigned char  u8;

#pragma function(memset)
void *memset(void *dest, int value, size_t count) {
    u8 c = *(u8 *)&value;
    u8 *at = (u8 *)dest;
    
    while (count--) {
        *at++ = c;
    }
    
    return dest;
}
#endif


//
// @note: Static initializers
//
typedef void (__cdecl *_PVFV)();

#if !defined(IML_RUN_STATIC_CONSTRUCTORS_AND_DESTRUCTORS)
int atexit(_PVFV func) { return 0; }
void __run_static_constructors(void) {}
void __run_static_destructors(void) {}
#else


#pragma section(".CRT$XCA", read)
#pragma data_seg(".CRT$XCA")		// start of ctor section
_PVFV __xc_a[] = {0};

#pragma section(".CRT$XCZ", read)
#pragma data_seg(".CRT$XCZ")		// end of ctor section
_PVFV __xc_z[] = {0};

#pragma data_seg()
#pragma comment(linker, "/merge:.CRT=.rdata")


void __initterm(_PVFV *pfbegin, _PVFV *pfend) {
	// walk the table of function pointers from the bottom up, until
	// the end is encountered.  Do not skip the first entry.  The initial
	// value of pfbegin points to the first valid entry.  Do not try to
	// execute what pfend points to.  Only entries before pfend are valid.
    for (; pfbegin < pfend; ++pfbegin) {
		if (*pfbegin)  (**pfbegin)();
	}
}


#if 0  //~ @note: Statically allocated number of destructors.
static _PVFV atexitlist[256];
static const unsigned int max_atexit_entries     = sizeof(atexitlist) / sizeof(atexitlist[0]);
static       unsigned int current_atexit_entries = 0;

int atexit(_PVFV func) {
	if (current_atexit_entries < max_atexit_entries) {
		atexitlist[current_atexit_entries] = func;
        current_atexit_entries += 1;
		return 0;
	}
    
	return -1;
}

#else  //~ @note: Statically allocated number of destructors.
static _PVFV *atexitlist = 0;
static unsigned int max_atexit_entries     = 0;
static unsigned int current_atexit_entries = 0;

int atexit(_PVFV func) {
    if (!atexitlist) {
        max_atexit_entries = 256;
        atexitlist = (_PVFV *) HeapAlloc(GetProcessHeap(), 0, max_atexit_entries * sizeof(_PVFV));
        if (!atexitlist)  return -1;
    }
    else if (current_atexit_entries >= max_atexit_entries) {
        _PVFV *old_data = atexitlist;
        auto old_size = max_atexit_entries;
        auto new_size = max_atexit_entries = max_atexit_entries * 2;
        
        HANDLE heap = GetProcessHeap();
        
        _PVFV *new_data = (_PVFV *) HeapAlloc(heap, 0, max_atexit_entries * sizeof(_PVFV));
        if (!new_data)  return -1;
        
        memcpy(new_data, old_data, old_size * sizeof(_PVFV));
        
        HeapFree(heap, 0, old_data);
    }
    
    atexitlist[current_atexit_entries] = func;
    current_atexit_entries += 1;
    
    return 0;
}
#endif


void __run_static_constructors(void) {
    __initterm(__xc_a, __xc_z);
}
void __run_static_destructors(void) {
    if (current_atexit_entries) {
        __initterm(atexitlist, atexitlist + current_atexit_entries);
    }
}
#endif


//
// @note: I don't know why this is needed.
//
// Has something todo with exception handling.
// So we shouldn't need that.
//
int _purecall(void) {
	ExitProcess(-1);
    return -1;
}

void exit(const int status) {
    __run_static_destructors();
    ExitProcess(status);
}

void abort(void) {
    __debugbreak();
    exit(-1);
}

__declspec(noreturn) void __cdecl __std_terminate(void) {
    abort();
}


//
// @note: Stuff to get defer working
//
#if 1

// @study What should these function really do, return???

__cdecl
__CxxFrameHandler3(void *pExcept,
                   void *pRN,
                   void *pContext,
                   void *pDC) {
    return 0;
}

#include <excpt.h>

EXCEPTION_DISPOSITION
__CxxFrameHandler4(void *ExceptionRecord,
                   void *EstablisherFrame,
                   void *ContextRecord,
                   void *DispatcherContext) {
    return ExceptionContinueExecution;  // @hack
}

#else

//
// __CxxFrameHandler3 - Real entry point to the runtime
//                                              __CxxFrameHandler2 is an alias for __CxxFrameHandler3
//                                              since they are compatible in VC version of CRT
//                      These function should be separated out if a change makes
//                                              __CxxFrameHandler3 incompatible with __CxxFrameHandler2
//
extern "C" _VCRTIMP __declspec(naked) DECLSPEC_GUARD_SUPPRESS EXCEPTION_DISPOSITION __cdecl
__CxxFrameHandler3(
                   /*
                       EAX=FuncInfo   *pFuncInfo,          // Static information for this frame
                   */
                   EHExceptionRecord  *pExcept,        // Information for this exception
                   EHRegistrationNode *pRN,            // Dynamic information for this frame
                   void               *pContext,       // Context info (we don't care what's in it)
                   DispatcherContext  *pDC             // More dynamic info for this frame (ignored on Intel)
                   ) {
    FuncInfo   *pFuncInfo;
    EXCEPTION_DISPOSITION result;
    
    __asm {
        //
        // Standard function prolog
        //
        push    ebp
            mov     ebp, esp
            sub     esp, __LOCAL_SIZE
            push    ebx
            push    esi
            push    edi
            cld             // A bit of paranoia -- Our code-gen assumes this
        
            //
            // Save the extra parameter
            //
            mov     pFuncInfo, eax
    }
    
    EHTRACE_ENTER_FMT1("pRN = 0x%p", pRN);
    
    result = __InternalCxxFrameHandler<RENAME_EH_EXTERN(__FrameHandler3)>( pExcept, pRN, (PCONTEXT)pContext, pDC, pFuncInfo, 0, nullptr, FALSE );
    
    EHTRACE_HANDLER_EXIT(result);
    
    __asm {
        pop     edi
            pop     esi
            pop     ebx
            mov     eax, result
            mov     esp, ebp
            pop     ebp
            ret     0
    }
}
#endif

