#ifndef IML_OS_LINUX_SPECIFIC_CPP
#define IML_OS_LINUX_SPECIFIC_CPP


#include "../os.h"
#include "iml_memory/memory.h"


//
// @note: File stuff
//
inline bool os_file_exists(const char *path) {
    return (access(path, F_OK) == 0);
}

inline File_Handle os_open_file(const char *file_name, u32 mode_flags) {
    int flags = 0;
    int mode = 0;
    
    const bool mode_read  = (mode_flags & OPEN_FILE_read);
    const bool mode_write = (mode_flags & OPEN_FILE_write);
    
    //
    // @TODO More optional iml_os mode_flags for executing permissions and stuff...
    // https://linux.die.net/man/2/open
    //
    if (mode_read && mode_write) {
        flags |= O_RDWR;
        flags |= O_CREAT;
        flags |= O_TRUNC;
        mode |= S_IRWXU | S_IRGRP | S_IROTH;
    } else {
        if (mode_flags & OPEN_FILE_read) {
            flags |= O_RDONLY;
        }
        
        if (mode_flags & OPEN_FILE_write) {
            flags |= O_WRONLY;
            flags |= O_CREAT;
            flags |= O_TRUNC;
            mode |= S_IRWXU | S_IRGRP | S_IROTH;
        }
    }
    
    int handle = open(file_name, flags, mode);
    
    File_Handle result;
    *((int *) &result.os) = handle;
    result.no_errors = (handle != -1);
    
    return result;
}

inline void os_close_file(File_Handle *file) {
    int handle = *(int *) &file->os;
    close(handle);
}

inline u64 os_get_file_size(File_Handle *file) {
    int handle = *(int *) &file->os;
    
    struct stat file_status;
    if (fstat(handle, &file_status) != -1) {
        return file_status.st_size;
    }
    
    return 0;
}

inline u32 os_read_entire_file(File_Handle *file, u64 size, void *destination) {
    int handle = *(int *) &file->os;
    
    u32 bytes_to_read = size;
    u8 *next_byte_location = (u8 *) destination;
    while (bytes_to_read) {
        u32 bytes_read = read(handle, next_byte_location, bytes_to_read);
        if (bytes_read == -1) {
            break;
        }
        
        bytes_to_read -= bytes_read;
        next_byte_location += bytes_read;
    }
    
    return size - bytes_to_read;
}

inline u32 os_write_file(File_Handle *file, u64 size, const void *source) {
    int handle = *(int *) &file->os;
    
    u32 bytes_to_write = size;
    u8 *next_byte_location = (u8 *) source;
    while (bytes_to_write) {
        u32 bytes_written = write(handle, next_byte_location, bytes_to_write);
        if (bytes_written == -1) {
            break;
        }
        
        bytes_to_write -= bytes_written;
        next_byte_location += bytes_written;
    }
    
    return size - bytes_to_write;
}

inline u32 os_read_data_from_file(File_Handle *file, u64 offset, u64 size, void *destination) {
    not_implemented;
    return 0;
}

inline u32 os_write_data_to_file(File_Handle *file, u64 offset, u64 size, const void *source) {
    not_implemented;
    return 0;
}


//
// @note: Directory stuff
//
inline bool os_directory_exists(const char *path) {
    struct stat file_status;
    
    if (stat(path, &file_status) == 0 && S_ISDIR(file_status.st_mode)) {
        return true;
    }
    
    return false;
}

inline bool os_change_directory(const char *path) {
    int result = chdir(path);
    return (result == 0);
}

inline bool os_create_directory(const char *path) {
    int mode = S_IRWXU | S_IRGRP | S_IROTH;
    bool result = mkdir(path, mode);
    return (result == 0);
}


//
// @note: Command line stuff
//
inline const char *os_get_command_line(void) {
    not_implemented;
    return "";
}

// CommandLineToArgvA
// @todo Rewrite and remove GlobalAlloc
char **os_command_line_to_argv(const char *CmdLine, int *_argc) {
    not_implemented;
    return null;
}


//
// @note: Process stuff
//
inline void os_set_high_process_priority(bool high) {
    // TODO
}

inline const char *os_get_operating_path(Memory_Arena *arena) {
    not_implemented;
    return "";
}

inline void os_sleep(u32 milliseconds) {
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    if (milliseconds >= 1000)
      sleep(milliseconds / 1000);
    usleep((milliseconds % 1000) * 1000);
#endif
}


//
// @note: Execute system commands
//
int os_execute_system_command(const char *path, const char *command_line) {
    int result = chdir(path);
    
    if (result == 0) {
        result = system(command_line);
    }
    
    return result;

#if 0
    static int exec_prog(const char **argv)
    {
        pid_t   my_pid;
        int     status, timeout /* unused ifdef WAIT_FOR_COMPLETION */;

        if (0 == (my_pid = fork())) {
                if (-1 == execve(argv[0], (char **)argv , NULL)) {
                        perror("child process execve failed [%m]");
                        return -1;
                }
        }

    #ifdef WAIT_FOR_COMPLETION
        timeout = 1000;

        while (0 == waitpid(my_pid , &status , WNOHANG)) {
                if ( --timeout < 0 ) {
                        perror("timeout");
                        return -1;
                }
                sleep(1);
        }

        printf("%s WEXITSTATUS %d WIFEXITED %d [status %d]\n",
                argv[0], WEXITSTATUS(status), WIFEXITED(status), status);

        if (1 != WIFEXITED(status) || 0 != WEXITSTATUS(status)) {
                perror("%s failed, halt system");
                return -1;
        }

    #endif
        return 0;
    }
#endif
}


//
// @note: Time
//
inline f64 os_get_time(void) {
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    f64 result = (f64)(1000000000 * ts.tv_sec + ts.tv_nsec) * 1e-9f;  // TODO Better version, thats weird!
    return result;
}


//
// @note: Custom CRT startup
//
void os_init(void) {
    //~ Base Allocator
    global_os_base_allocator = os_make_base_allocator();
}


#endif // IML_OS_WIN32_SPECIFIC_CPP
