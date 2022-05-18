#if !defined(IML_NO_CRT_STARTUP_CPP)
#define IML_NO_CRT_STARTUP_CPP


//
// @note: Defines
//
#if !defined(BUILD_WITH_NO_CRT)
#  define BUILD_WITH_NO_CRT  0
#endif

#if !defined(CUSTOM_CRT_STARTUP)
#  define CUSTOM_CRT_STARTUP  0
#endif


#if !defined(OUTPUT_TYPE_EXECUTABLE)
#  define OUTPUT_TYPE_EXECUTABLE  1
#endif

#if !defined(OUTPUT_TYPE_DLL)
#  define OUTPUT_TYPE_DLL  1
#endif


#if !defined(SUBSYSTEM_CONSOLE)
#define SUBSYSTEM_CONSOLE  1
#endif

#if !defined(SUBSYSTEM_WINDOWS)
#define SUBSYSTEM_WINDOWS  1
#endif


//
// @note Custom startup code
//
#if !BUILD_WITH_NO_CRT
static struct CustomCRTStartup {
    CustomCRTStartup() {
        void __custom_crt_startup(void);
        __custom_crt_startup();
    }
} __global_custom_crt_startup__;
#endif


#include "iml_base_defines.h"

#if OS_WINDOWS
extern "C" {
    void __run_static_constructors(void);
    void __run_static_destructors(void);
};
#else
extern "C" {
    void __run_static_constructors(void) {}
    void __run_static_destructors(void) {}
};
#endif


#include "iml_profiler.h"
#include "iml_crash_handler.cpp"

#define IML_OS_IMPLEMENTATION
#include "os/os.h"

#define IML_TYPES_IMPLEMENTATION
#include "iml/iml_types.h"

#define IML_COMMON_ARRAY_SYSTEM_IMPLEMENTATION
#include "iml_common_array_system.h"



static bool __custom_crt_startup_was_run = false;
void __custom_crt_startup(void) {
    if (__custom_crt_startup_was_run)  return;
    __custom_crt_startup_was_run = true;
    
    os_init();
    os_set_high_process_priority(true);
    
    enable_crash_handler(/*interactive=*/false);
    
    init_common_array_system();
}



// @note @important
//  If not using the default CRT entry points,
// C++ constructors for static objects are probably not executed.


#if CUSTOM_CRT_STARTUP
#if OUTPUT_TYPE_EXECUTABLE
// -ENTRY  default is WinMainCRTStartup
// There are a few that get looked up:
// mainCRTStartup || wmainCRTStartup  For an application using /SUBSYSTEM:CONSOLE  (DEFAULT)
// WinMainCRTStartup wWinMainCRTS     For an application using /SUBSYSTEM:WINDOWS

#if SUBSYSTEM_WINDOWS
extern "C" void __stdcall WinMainCRTStartup(void) {
    __custom_crt_startup();
    __run_static_constructors();
    const int result = main();
    __run_static_destructors();
    ExitProcess(result);  // Should not just return here, so let's exit the process instead.
}

#elif SUBSYSTEM_CONSOLE
// For /SUBSYSTEM:console
extern "C" void __stdcall mainCRTStartup(void) {
    __custom_crt_startup();
    __run_static_constructors();
    const int result = main();
    __run_static_destructors();
    ExitProcess(result);
}
#endif // SUBSYSTEM_CONSOLE

#elif OUTPUT_TYPE_DLL
// For /DLL
extern "C" void __stdcall _DllMainCRTStartup(void) {
    // @todo
}
#endif // OUTPUT_TYPE_DLL

#else // of #if CUSTOM_CRT_STARTUP
#  if 0
int main(int argc, char *argv[]) {
}
int CALLBACK
WinMain(HINSTANCE instance,
        HINSTANCE prev_instance,
        LPSTR command_line,
        int show_code) {
}
extern "C" inline int __stdcall _main_crt_startup(void) {
    __custom_crt_startup();
    __run_static_constructors();
    const int result = main();
    __run_static_destructors();
    return result;
}
#  endif
#endif
//
//~


#endif // IML_NO_CRT_STARTUP_CPP


