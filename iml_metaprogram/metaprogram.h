#if !defined(IML_METAPROGRAM_H)
#define IML_METAPROGRAM_H


#include "os/os.h"

#include "iml_profiler.h"

#include "iml_types.h"
#include "iml_array.h"
#include "iml_string_list.h"

#include "iml_memory/memory.h"
#include "iml_memory/temporary_storage.h"


//
// @note: Other helper functions
//
void list_string_occurences(const char *src_path, const char *needle, const char *options = "") {
    const char *format = "grep -inr %s --exclude-dir={.svn,.git,tools} --exclude=first.cpp --exclude=*.added_strings* '%s'";
    const char *command = tprint(format, options, needle);
    os_execute_system_command(src_path, command);
}

//~ @note: Compile-time functions
#define COMPILE_TIME_FUNCTION_SIGNATURE(name)  void name(Memory_Arena *arena, const char *const root_path, const char *const build_path, const char *const src_path)
typedef COMPILE_TIME_FUNCTION_SIGNATURE(Compile_Time_Function);
typedef Compile_Time_Function *Compile_Time_Function_Array[];

void execute_compile_time_functions(Compile_Time_Function_Array functions, int function_count, const char *const root_path, const char *const build_path, const char *const src_path) {
    if (function_count <= 0)  return;
    
    auto arena = get_temporary_storage();
    
    printf("[metaprogram] === Running %d user-code hooks. ===\n", function_count);
    
    for (int i = 0; i < function_count; ++i) {
        if (!functions[i]) {
            printf("ERROR: Compile_Time_Function[%d] was NULL!\n", i);
            continue;
        }
        
        Temporary_Memory_Block temp(arena);
        (functions[i])(arena, root_path, build_path, src_path);
    }
}


//
// @note: Build_Options stuff
//
#if 0
// @note: Jai
Build_Output_Type :: enum u8 {
    NO_OUTPUT       :: 0;
    EXETUTABLE      :: 1;
    DYNAMIC_LIBRARY :: 2;  // Not yet implemented.
    STATIC_LIBRARY  :: 3;  // Not yet implemented.
};

Build_Options :: struct {
    output_type := Build_Options.EXECUTABLE;
    optimization_level := Optimization_Level.DEBUG;
    
    //...
};
#endif

enum Platform_Type {
    PLATFORM_NULL = 0,
    PLATFORM_Windows,
    PLATFORM_Linux,
};

struct Build_Options {
    Platform_Type target_platform = PLATFORM_NULL;
    const char *architecture = "";
    const char *compiler = "";
    
    bool build_debug = true;
    
    int  warnings_all = false;
    bool warnings_are_errors = true;
    const char *warning_flags = "";
    
    const char *root_path  = "";
    const char *src_path   = "";
    const char *build_path = "";
    
    Array <const char *> include_paths;
    Array <const char *> object_files;
    Array <const char *> libraries;
    
    const char *input_filename = "";
    Array <const char *> input_files;  // Additional input files
    Array <const char *> asm_files;  // Additional assembly files
    const char *output_filename = "";
    
    Array <const char *> compiler_flags;
    Array <const char *> linker_flags;
};

Build_Options get_build_options(void) {
    Build_Options options = {};
    
    if constexpr (OS_WINDOWS) {
        options.target_platform = PLATFORM_Windows;
        options.compiler = "cl";
    }
    else if constexpr (OS_LINUX) {
        options.target_platform = PLATFORM_Linux;
        options.compiler = "g++";
    }
    else {
        fprintf(stderr, "OS not supported!\n");
    }
    
    return options;
}

String get_command_line(const Build_Options &options) {
    auto arena = get_temporary_storage();
    
    // @todo: Implement string builder
    //        append(&builder, "Hello Sailor!");
    String_List command_list = {};
    String_List *list = &command_list;
    
    //~ Compiler flags
    string_list_push(arena, list, options.compiler);
    string_list_push(arena, list, " ");
    
    For (options.compiler_flags) {
        string_list_push(arena, list, it);
        string_list_push(arena, list, " ");
    }
    
    if (strings_match(options.compiler, "cl")) {
        string_list_push(arena, list, tprint("-Fe:%s ", options.output_filename));
    }
    else if (strings_match(options.compiler, "g++")) {
        string_list_push(arena, list, tprint("-o%s ", options.output_filename));
    }
    else {
        not_implemented;
    }
    
    // Debug level
    if (options.build_debug) {
        string_list_push(arena, list, "-DBUILD_DEBUG ");
        
        if (strings_match(options.compiler, "cl")) {
            string_list_push(arena, list, "-Od -Oi -MTd ");
        }
        else if (strings_match(options.compiler, "g++")) {
            string_list_push(arena, list, "-g ");
        }
        else {
            not_implemented;
        }
    } else {
        string_list_push(arena, list, "-DNDEBUG -DBUILD_RELEASE ");
        
        if (strings_match(options.compiler, "cl")) {
            string_list_push(arena, list, "-O2 -Oi -MT ");
        }
        else if (strings_match(options.compiler, "g++")) {
            string_list_push(arena, list, "-Ofast ");
        }
        else {
            not_implemented;
        }
    }
    
    // Additional include paths
    For (options.include_paths) {
        string_list_push(arena, list, tprint("-I%s/%s ", options.root_path, it));
    }
    
    // Warnings level
    // -w -w0   Suppress all warnings.
    // -w1 -w4  Specify warning level (1 to 4).
    // -Wall    To turn all warnings on
    // -WX      To handle warnings as errors
    if (options.warnings_are_errors) {
        if (strings_match(options.compiler, "cl")) {
            string_list_push(arena, list, "-WX ");
        }
        else if (strings_match(options.compiler, "g++")) {
            string_list_push(arena, list, "-Werror ");
        }
        else {
            not_implemented;
        }
    }
    
    if (options.warnings_all) {
        string_list_push(arena, list, "-Wall ");
    } else {
        if (strings_match(options.compiler, "cl")) {
            string_list_push(arena, list, tprint("-W3 "));
        }
        else if (strings_match(options.compiler, "g++")) {
        }
        else {
            not_implemented;
        }
    }
    
    string_list_push(arena, list, options.warning_flags);
    
    // Input filename
    string_list_push(arena, list, tprint("%s/%s ", options.src_path, options.input_filename));
    
    For (options.input_files) {
        string_list_push(arena, list, tprint("%s/%s ", options.root_path, it));
    }
    
    For (options.object_files) {
        string_list_push(arena, list, it);
        string_list_push(arena, list, " ");
    }
    
    
    //~ Linker flags
    if (strings_match(options.compiler, "cl")) {
        string_list_push(arena, list, " /link ");
    }
    else if (strings_match(options.compiler, "g++")) {
    }
    else {
        not_implemented;
    }
    
    For (options.linker_flags) {
        string_list_push(arena, list, it);
        string_list_push(arena, list, " ");
    }
    
    For (options.libraries) {
        string_list_push(arena, list, it);
        string_list_push(arena, list, " ");
    }
    
    
    //~ Build command_line
    String command_line = string_list_flatten(arena, command_list);
    
    return command_line;
}

int execute_build_options(const Build_Options &options, bool print_command_line = false) {
    const char *path = options.build_path;
    os_create_directory(path);
    assert(os_directory_exists(path));
    
    path = tprint("%s/%s", path, options.architecture);
    os_create_directory(path);
    assert(os_directory_exists(path));
    
    if (options.build_debug) {
        printf("\n\n- Compiling debug version:\n");
        path = tprint("%s/debug", path);
    }
    else {
        printf("\n\n- Compiling release version:\n");
        path = tprint("%s/release", path);
    }
    os_create_directory(path);
    assert(os_directory_exists(path));
    
    //~ Assembly phase
    For (options.asm_files) {
        const char *command_line = tprint("ml64 -nologo -WX -W3 -Zd -Zi -c %s", it);
        if (print_command_line)  printf("\nCommand line: %s\n", command_line);
        os_execute_system_command(path, command_line);
    }
    
    //~ Main program compilation
    String command_line = get_command_line(options);
    if (print_command_line) {
        printf("\n\nCommand line: %s\n\n", command_line.data);
    }
    
    int result = os_execute_system_command(path, cast(const char *)command_line.data);
    if (result != 0) {
        fprintf(stderr, "Compiler exited with error code: %d\n", result);
    }
    
    return result;
}


#endif // IML_METAPROGRAM_H
