#ifndef IML_OS_H
#define IML_OS_H


#include <stdio.h>
#include <cstdio>

//
// @note: sprintf, vsnprintf
//
#include "stb/stb_sprintf.h"

#if OS_WINDOWS
#  undef sprintf
#  undef vsnprintf
#  define sprintf    stbsp_sprintf
#  define vsnprintf  stbsp_vsnprintf
#endif


//
// @note: Include platform specific
//
#include "iml_base_defines.h"

#if OS_WINDOWS
#  include "os/win32/include.h"
#elif OS_LINUX
#  include "os/linux/include.h"
#else
#  error "OS not implemented!"
#endif

#include "iml_types.h"
#include "iml_memory/memory.h"


//
// @note: File API
//
struct File_Handle {
    b32 no_errors;
    void *os;
};

#define os_no_file_errors(file_handle)  ((file_handle)->no_errors)

//global File_Handle *iml_stdin;
//global File_Handle *iml_stdout;
//global File_Handle *iml_stderr;

enum Open_File_Mode_Flags : u32 {
    OPEN_FILE_read = 0x1,
    OPEN_FILE_write = 0x2,
};

inline bool os_file_exists(const char *path);

inline File_Handle os_open_file(const char *file_name, u32 mode_flags);
inline void os_close_file(File_Handle *file);

inline u64 os_get_file_size(File_Handle *file_handle);

inline u32 os_read_entire_file(File_Handle *file_handle, u64 size, void *destination);
inline u32 os_write_file(File_Handle *file, u64 size, const void *source);
inline u32 os_read_data_from_file(File_Handle *file, u64 offset, u64 size, void *destination);
inline u32 os_write_data_to_file(File_Handle *file, u64 offset, u64 size, void *source);


//
// @note: Directory stuff
//
inline bool os_directory_exists(const char *path);
inline bool os_change_directory(const char *path);
inline bool os_create_directory(const char *path);


//
// @note: Process stuff
//
inline void os_set_high_process_priority(bool high);
inline u32 os_get_operating_path(u32 size, char *buffer);
inline void os_sleep(u32 milliseconds);


//
// @note: Execute system commands
//
int os_execute_system_command(const char *path, const char *command_line);


//
// @note: Time
//
inline f64 os_get_time(void);


//
// @note: Custom CRT startup
//
void os_init(void);


//
// @note: printf and fprintf
//
#include <cstdarg> // va_list, ...

inline void vfprintf(File_Handle *file, const char *format, va_list args);
inline void fprintf(File_Handle *file, const char *format, ...);

#include <stdio.h>
#include <cstdio>
//#define printf(...)  fprintf(iml_stdout, __VA_ARGS__)


//
// @note: Command line stuff
//
inline const char *os_get_command_line(void);
char **os_command_line_to_argv(const char *CmdLine, int *_argc);

struct Args {
    int count;  // argc
    char **data;  // argv
    
    char *operator[](const int index) {
        assert(index >= 0 && index < count);
        return data[index];
    }
    
    const char *operator[](const int index) const {
        assert(index >= 0 && index < count);
        return data[index];
    }
    
    char **begin(void) const {
        return data;
    }
    
    const char **end(void) const {
        return cast(const char **)data + count;
    }
};

inline Args os_command_line_to_args(const char *command_line) {
    Args args;
    args.data = os_command_line_to_argv(command_line, &args.count);
    return args;
}


//
// @note: Allocator
//
inline BASE_ALLOCATOR_RESERVE_SIGNATURE(os__base_reserve);
inline BASE_ALLOCATOR_FREE_SIGNATURE(os__base_free);

inline Base_Allocator os_make_base_allocator(void);

global Base_Allocator global_os_base_allocator = {};
inline Base_Allocator *os_get_base_allocator(void);


//
// @note: Arena
//
inline Memory_Arena os_make_arena(u64 chunk_size, u64 align);
inline Memory_Arena os_make_arena(u64 chunk_size);
inline Memory_Arena os_make_arena(void);



#endif//IML_OS_H

//
// @note: OS Specific Implementation
//
#ifdef IML_OS_IMPLEMENTATION
#undef IML_OS_IMPLEMENTATION
#ifndef IML_OS_CPP
#define IML_OS_CPP


#define STB_SPRINTF_IMPLEMENTATION
#include "stb/stb_sprintf.h"

#if OS_WINDOWS
#  include "os/win32/specific.cpp"
#  include "os/win32/memory.cpp"
#elif OS_LINUX
#  include "os/linux/specific.cpp"
#  include "os/linux/memory.cpp"
#else
#  error "OS not implemented!"
#endif


inline void vfprintf(File_Handle *file, const char *format, va_list args) {
    char buffer[4096];
    int size = vsnprintf(buffer, sizeof(buffer)/sizeof(buffer[0]), format, args);
    os_write_file(file, size, buffer);
}

inline void fprintf(File_Handle *file, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(file, format, args);
    va_end(args);
}


#endif // IML_OS_CPP
#endif // IML_OS_IMPLEMENTATION
