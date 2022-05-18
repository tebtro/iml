#if !defined(IML_CRASH_HANDLER_CPP)
#define IML_CRASH_HANDLER_CPP


// @todo Move this crash handler to os_win32
#include "os/os.h"


#include "iml_types.h"
#include "iml_terminal.h"
#include "iml_array.h"
#include "iml_memory/temporary_storage.h"


//
// @note: Setup
//
typedef void Print_Stack_Tracy_Function(void);

global Print_Stack_Tracy_Function *global_print_stack_trace_function = null;

no_inline void iml_print_stack_trace(void) {
    if (global_print_stack_trace_function)  global_print_stack_trace_function();
}

inline void enable_crash_handler(bool interactive = true) {
    void win32_print_stack_trace(void);
    global_print_stack_trace_function = win32_print_stack_trace;
    
    
    // @todo interactive debug_break
#if 0
    int msg_id = MessageBox(NULL,
                            "Hit a debug break!",
                            "Hit Debug Breakpoint",
                            MB_ICONERROR | MB_ABORTRETRYIGNORE);
    switch (msg_id) {
        case IDABORT: { printf("Abort\n"); } break;
        case IDRETRY: { printf("Retry\n"); } break;
        case IDIGNORE: { printf("Ignore\n"); } break;
    }
#endif
}

inline void disable_crash_handler(void) {
    global_print_stack_trace_function = null;
}


//
// @note: Stack Trace
//
#if OS_WINDOWS
struct Stack_Frame {
    DWORD64 address;
    char *name;
    //std::string module;
    DWORD64 line;
    char *file;
};

inline void win32_get_stack_trace(Array<Stack_Frame> *frames) {
    auto arena = get_temporary_storage();
    
#if _WIN64
    DWORD machine = IMAGE_FILE_MACHINE_AMD64;
#else
    DWORD machine = IMAGE_FILE_MACHINE_I386;
#endif
    
    HANDLE process = GetCurrentProcess();
    HANDLE thread  = GetCurrentThread();
    
    if (SymInitialize(process, NULL, TRUE) == FALSE) {
        fprintf(stderr, "*** Error in stack_trace: Failed to call SymInitialize.\n");
        return;
    }
    
    SymSetOptions(SYMOPT_LOAD_LINES);
    
    CONTEXT context = {};
    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&context);
    
#if _WIN64
    STACKFRAME frame = {};
    frame.AddrPC.Offset    = context.Rip;
    frame.AddrPC.Mode      = AddrModeFlat;
    frame.AddrFrame.Offset = context.Rbp;
    frame.AddrFrame.Mode   = AddrModeFlat;
    frame.AddrStack.Offset = context.Rsp;
    frame.AddrStack.Mode   = AddrModeFlat;
#else
    STACKFRAME frame = {};
    frame.AddrPC.Offset    = context.Eip;
    frame.AddrPC.Mode      = AddrModeFlat;
    frame.AddrFrame.Offset = context.Ebp;
    frame.AddrFrame.Mode   = AddrModeFlat;
    frame.AddrStack.Offset = context.Esp;
    frame.AddrStack.Mode   = AddrModeFlat;
#endif
    
    for (int i = 0;
         StackWalk(machine, process, thread, &frame, &context, NULL, SymFunctionTableAccess, SymGetModuleBase, NULL);
         ++i) {
        Stack_Frame f = {0};
        f.address = frame.AddrPC.Offset;
        
        IMAGEHLP_LINE line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE);
        
        // @note Get function name
#if _WIN64
        DWORD64 offset = 0;
#else
        DWORD offset = 0;
#endif
        char symbol_buffer[sizeof(IMAGEHLP_SYMBOL) + 255];
        PIMAGEHLP_SYMBOL symbol = (PIMAGEHLP_SYMBOL)symbol_buffer;
        symbol->SizeOfStruct = (sizeof IMAGEHLP_SYMBOL) + 255;
        symbol->MaxNameLength = 254;
        if (SymGetSymFromAddr(process, frame.AddrPC.Offset, &offset, symbol)) {
            f.name = push_cstring_copy(arena, symbol->Name);
        }
        else {
            f.name = push_cstring_copy(arena, "unknown");
            // @todo Error
        }
        
        // @note Get filename and line number
        DWORD offset_ln = 0;
        if (SymGetLineFromAddr(process, frame.AddrPC.Offset, &offset_ln, &line)) {
            f.file = push_cstring_copy(arena, line.FileName);
            f.line = line.LineNumber;
        }
        else {
            f.file = push_cstring_copy(arena, "unknown");
            f.line = frame.AddrPC.Offset;
            // @todo Error
        }
        
        // @note: Skip a few functions related to asserts and this stack tracer.
        if (i > 2) {
            frames->add(f);
        }
    }
    
    SymCleanup(process);
}
#endif

void win32_print_stack_trace(void) {
#if OS_WINDOWS
    // Prevent recursion.
    disable_crash_handler();
    defer { enable_crash_handler(); };
    
    auto colors = get_terminal_colors();
    fprintf(stderr, "%sStack trace:%s \n", colors.red, colors.reset);
    
    Array<Stack_Frame> stack_frames;
    win32_get_stack_trace(&stack_frames);
    
    For (stack_frames) {
        fprintf(stderr, "  %s(%lld) : %s\n", it.file, it.line, it.name);
    }
    
    fprintf(stderr, "\n");
#endif
}


#endif
