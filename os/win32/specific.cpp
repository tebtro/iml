#ifndef IML_OS_WIN32_SPECIFIC_CPP
#define IML_OS_WIN32_SPECIFIC_CPP


#include "os.h"
#include "iml_memory/memory.h"


//
// @note: File stuff
//
inline bool os_file_exists(const char *path) {
    Profile_Zone();
    
    WIN32_FIND_DATA find_data;
    HANDLE handle = FindFirstFile(path, &find_data);
    bool result = (handle != INVALID_HANDLE_VALUE);
    FindClose(handle);
    return result;
}

inline File_Handle os_open_file(const char *file_name, u32 mode_flags) {
    Profile_Zone();
    
    DWORD handle_permissions = 0;
    DWORD handle_creation = 0;
    
    if (mode_flags & OPEN_FILE_read) {
        handle_permissions |= GENERIC_READ;
        handle_creation = OPEN_EXISTING;
    }
    
    if (mode_flags & OPEN_FILE_write) {
        handle_permissions |= GENERIC_WRITE;
        handle_creation = CREATE_ALWAYS;
    }
    
    File_Handle result;
    *((HANDLE *)&result.os) = CreateFile(file_name, handle_permissions,
                                         FILE_SHARE_READ, 0,
                                         handle_creation, FILE_ATTRIBUTE_NORMAL, 0);
    result.no_errors = (result.os != INVALID_HANDLE_VALUE);
    return result;
}

inline void os_close_file(File_Handle *file) {
    Profile_Zone();
    
    HANDLE win32_handle = *(HANDLE *)&file->os;
    CloseHandle(win32_handle);
}

inline u64 os_get_file_size(File_Handle *file) {
    Profile_Zone();
    
    HANDLE win32_handle = *(HANDLE *)&file->os;
    
    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(win32_handle, &file_size))  return 0;
    return file_size.QuadPart;
}

inline u32 os_read_entire_file(File_Handle *file, u64 size, void *destination) {
#if 0
    Profile_Zone();
    
    HANDLE win32_handle = *(HANDLE *)&file->os;
    
    DWORD bytes_read;
    if (!ReadFile(win32_handle, destination, safe_truncate_to_u32(size), &bytes_read, null)) {
        return 0;
    }
    
    return bytes_read;
#else
    return os_read_data_from_file(file, 0, size, destination);
#endif
}

inline u32 os_write_file(File_Handle *file, u64 size, const void *source) {
    Profile_Zone();
    
    HANDLE win32_handle = *(HANDLE *)&file->os;
    
    DWORD bytes_written;
    if (!WriteFile(win32_handle, source, safe_truncate_to_u32(size), &bytes_written, NULL)) {
        return 0;
    }
    
    return bytes_written;
}

inline u32 os_read_data_from_file(File_Handle *file, u64 offset, u64 size, void *destination) {
    Profile_Zone();
    
    HANDLE win32_handle = *(HANDLE *)&file->os;
    
    OVERLAPPED overlapped = {};
    overlapped.Offset = (u32)((offset >> 0) & 0xFFFFFFFF);
    overlapped.OffsetHigh = (u32)((offset >> 32) & 0xFFFFFFFF);
    
    u32 size32 = safe_truncate_to_u32(size);
    
    DWORD bytes_read;
    if (!ReadFile(win32_handle, destination, safe_truncate_to_u32(size), &bytes_read, &overlapped)) {
        return 0;
    }
    
    return bytes_read;
}

inline u32 os_write_data_to_file(File_Handle *file, u64 offset, u64 size, const void *source) {
    Profile_Zone();
    
    HANDLE win32_handle = *(HANDLE *)&file->os;
    
    OVERLAPPED overlapped = {};
    overlapped.Offset = (u32)((offset >> 0) & 0xFFFFFFFF);
    overlapped.OffsetHigh = (u32)((offset >> 32) & 0xFFFFFFFF);
    
    u32 size32 = safe_truncate_to_u32(size);
    
    DWORD bytes_written;
    if (!WriteFile(win32_handle, source, size32, &bytes_written, &overlapped)) {
        return 0;
    }
    
    return bytes_written;
}


//
// @note: Directory stuff
//
inline bool os_directory_exists(const char *path) {
    Profile_Zone();
    
    DWORD ftype = GetFileAttributesA(path);
    if (ftype == INVALID_FILE_ATTRIBUTES)  return false;
    if (ftype & FILE_ATTRIBUTE_DIRECTORY)  return true;
    return false;
}

inline bool os_change_directory(const char *path) {
    not_implemented;
    return false;
}

inline bool os_create_directory(const char *path) {
    Profile_Zone();
    
    if (CreateDirectory(path, NULL))  return true;
    if (GetLastError() == ERROR_ALREADY_EXISTS)  return true;
    return false;
}


//
// @note: Command line stuff
//
inline const char *os_get_command_line(void) {
    Profile_Zone();
    
    return GetCommandLine();
}

// CommandLineToArgvA
// @todo Rewrite and remove GlobalAlloc
char **os_command_line_to_argv(const char *CmdLine, int *_argc) {
    Profile_Zone();
    
    PCHAR* argv;
    PCHAR  _argv;
    size_t len;
    ULONG   argc;
    CHAR   a;
    size_t i, j;
    
    BOOLEAN in_QM;
    BOOLEAN in_TEXT;
    BOOLEAN in_SPACE;
    
    len = strlen(CmdLine);
    i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);
    
    argv = (PCHAR*)GlobalAlloc(GMEM_FIXED, i + (len+2)*sizeof(CHAR));
    
    _argv = (PCHAR)(((PUCHAR)argv)+i);
    
    argc = 0;
    argv[argc] = _argv;
    in_QM = false;
    in_TEXT = false;
    in_SPACE = true;
    i = 0;
    j = 0;
    
    while (a = CmdLine[i]) {
        if (in_QM) {
            if (a == '\"') {
                in_QM = false;
            }
            else {
                _argv[j] = a;
                j++;
            }
        }
        else {
            switch (a) {
                case '\"': {
                    in_QM = true;
                    in_TEXT = true;
                    if (in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    in_SPACE = false;
                } break;
                
                case ' ':
                case '\t':
                case '\n':
                case '\r': {
                    if (in_TEXT) {
                        _argv[j] = '\0';
                        j++;
                    }
                    in_TEXT = false;
                    in_SPACE = true;
                } break;
                
                default: {
                    in_TEXT = true;
                    if (in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    _argv[j] = a;
                    j++;
                    in_SPACE = false;
                } break;
            }
        }
        
        i++;
    }
    _argv[j] = '\0';
    argv[argc] = NULL;
    
    (*_argc) = argc;
    return argv;
}


//
// @note: Process stuff
//
inline void os_set_high_process_priority(bool high) {
    Profile_Zone();
    
    if (high) {
        SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    }
    else {
        SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
    }
}

inline char *os_get_operating_path(Memory_Arena *arena) {
    Profile_Zone();
    
    DWORD size = GetCurrentDirectory(0, 0);
    char *operating_path = push_array(arena, char, size);
    GetCurrentDirectory(size, operating_path);
    return operating_path;
}

inline void os_sleep(u32 milliseconds) {
    Sleep(milliseconds);
}


//
// @note: Execute system commands
//
int os_execute_system_command(const char *path, const char *command_line) {
    Profile_Zone();
    
    STARTUPINFO startup_info;
    PROCESS_INFORMATION process_info;
    
    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);
    ZeroMemory(&process_info, sizeof(process_info));
    
    // Start the child process.
    if (!CreateProcess(NULL,                  // null - No module name (use command line)
                       (LPSTR) command_line,  // Command line
                       NULL,           // null - Process handle not inheritable
                       NULL,           // null - Thread handle not inheritable
                       FALSE,          // false - Set handle inheritance to FALSE
                       0,              // 0 - No creation flags
                       NULL,           // null - Use parent's environment block
                       path,           // null - Use parent's starting directory
                       &startup_info,  // Pointer to STARTUPINFO structure
                       &process_info)  // Pointer to PROCESS_INFORMATION structure
        ) {
        DWORD error_code = GetLastError();
        fprintf(stderr, "CreateProcess failed (%d).\n", error_code);
        ExitProcess(-1);
    }
    
    // Wait until child process exits.
    WaitForSingleObject(process_info.hProcess, INFINITE);
    
    DWORD return_code = 0;
    GetExitCodeProcess(process_info.hProcess, &return_code);
    
    // Close process and thread handles.
    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);
    
    return return_code;
}


//
// @note: Time
//
global s64 win32_global_performance_count_frequency;

inline f64 os_get_time(void) {
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    f64 result = (f64) counter.QuadPart / (f64) win32_global_performance_count_frequency;
    return result;
}


//
// @note: Custom CRT startup
//
//global File_Handle win32_stdin;
//global File_Handle win32_stdout;
//global File_Handle win32_stderr;

void os_init(void) {
    Profile_Zone();
    
    //~ Base_Allocator
    global_os_base_allocator = os_make_base_allocator();
    
    //~ Setup stdout
    /*
    *((HANDLE *)&win32_stdin.os)  = GetStdHandle(STD_INPUT_HANDLE);
    *((HANDLE *)&win32_stdout.os) = GetStdHandle(STD_OUTPUT_HANDLE);
    *((HANDLE *)&win32_stderr.os) = GetStdHandle(STD_ERROR_HANDLE);
    win32_stdin.no_errors  = (win32_stdin.os  != INVALID_HANDLE_VALUE);
    win32_stdout.no_errors = (win32_stdout.os != INVALID_HANDLE_VALUE);
    win32_stderr.no_errors = (win32_stderr.os != INVALID_HANDLE_VALUE);
    assert(win32_stdin.no_errors);
    assert(win32_stdout.no_errors);
    assert(win32_stderr.no_errors);
    iml_stdin  = &win32_stdin;
    iml_stdout = &win32_stdout;
    iml_stderr = &win32_stderr;
    */
    
    //~ Time
    LARGE_INTEGER performance_count_frequency_result;
    QueryPerformanceFrequency(&performance_count_frequency_result);
    win32_global_performance_count_frequency = performance_count_frequency_result.QuadPart;
}


#endif // IML_OS_WIN32_SPECIFIC_CPP
