#if !defined(IML_MEMORY_TEMPORARY_STORAGE_H)
#define IML_MEMORY_TEMPORARY_STORAGE_H

#if defined(IML_NAMESPACE)
namespace IML_NAMESPACE {
#  if 0
}
#  endif
#endif
//
//~



#include "iml_memory/memory.h"


struct Temporary_Storage {
    Memory_Arena arena;
};

#if 0
Temporary_Storage :: struct {
    u8 *data;
    s64 size;
    s64 occupied;
    s64 high_water_mark;
    
    void *overflow_allocator;
    void *overflow_allocator_data;
    
    void *overflow_pages;
    u8 *original_data;
    s64 original_size;
};
#endif


#if 1
//
// @note: Global temporary storage
//
global Temporary_Storage global_temporary_storage_ = {};

Memory_Arena *get_temporary_storage__init(void);
Memory_Arena *get_temporary_storage__fast(void);

#define GET_TEMPORARY_STORAGE_SIGNATURE(name)  Memory_Arena *name(void)
typedef GET_TEMPORARY_STORAGE_SIGNATURE(Get_Temporary_Storage_Function);
global Get_Temporary_Storage_Function *get_temporary_storage = get_temporary_storage__init;

Memory_Arena *get_temporary_storage__init(void) {
    // This function should only get called the first time, so base_allocator should be null.
    assert(!global_temporary_storage_.arena.base_allocator);
    global_temporary_storage_ = { os_make_arena() };
    
    get_temporary_storage = get_temporary_storage__fast;
    
    return &global_temporary_storage_.arena;
}

Memory_Arena *get_temporary_storage__fast(void) {
    return &global_temporary_storage_.arena;
}

#if 0
// @todo Test if the if version is faster, or the function pointer version.
// @note Hard to really test, because in a real world application, the branch prediction might not be stored anymore.
Memory_Arena *get_temporary_storage(void) {
    if (!global_temporary_storage_.arena.base_allocator){
        global_temporary_storage_ = { platform_make_arena() };
    }
    
    get_temporary_storage = get_temporary_storage__fast;
    
    return &global_temporary_storage_.arena;
}

#endif

#else

//
// @note: Thread local temporary storage
//
dont_compile;  // not_implemented;

// @todo Create os_* functions for this stuff.

global DWORD global_tls_index_temporary_storage = TLS_OUT_OF_INDEXES;

void os_init_temporary_storage(void) {
    assert(global_tls_index_temporary_storage == TLS_OUT_OF_INDEXES, "Tls index already set!");
    global_tls_index_temporary_storage = TlsAlloc();
    
    if (global_tls_index_temporary_storage == TLS_OUT_OF_INDEXES) {
        fprintf(stderr, "Could not create a tls index for temporary storage!\n");
        assert(false);
    }
}

void thread_init_temporary_storage(void) {
    Temporary_Storage *temporary_storage = base_allocate(platform_get_base_allocator(), sizeof(Temporary_Storage));
    
    if (!TlsSetValue(global_tls_index_temporary_storage, temporary_storage)) {
        fprintf(stderr, "Could not set temporary storage on the tls index!\n");
        assert(false);
    }
}

void thread_destroy_temporary_storage(void) {
    Temporary_Storage *temporary_storage = cast(Temporary_Storage *) TlsGetValue(global_tls_index_temporary_storage);
    linalloc_clear(temporary_storage->arena);
    base_free(platform_get_base_allocator(), temporary_storage);
}

Memory_Arena *get_temporary_storage(void) {
    assert(global_tls_index_temporary_storage != TLS_OUT_OF_INDEXES);
    Temporary_Storage *temporary_storage = cast(Temporary_Storage *) TlsGetValue(global_tls_index_temporary_storage);
    assert(temporary_storage);
    return temporary_storage;
}

#endif


//
// @note: Temporary storage helper functions
//
inline void reset_temporary_storage() {
    linalloc_clear(get_temporary_storage());  // @todo We should not free all memory.
}

inline void *talloc(size_t size) {
    return push_size(get_temporary_storage(), size);
}

inline void *tcopy(const void *data, u64 count) {
    void *copy = push_size(get_temporary_storage(), count);
    memcpy(copy, data, count);
    return copy;
}

inline char *tcopy_string(const char *string) {
    u32 length = string_length(string) + 1;  // Include zero-terminator.
    return cast(char *) tcopy(string, length);
}

inline char *tprint(const char *format, ...) {
    Memory_Arena *arena = get_temporary_storage();
    char *buffer = null;
    int size = 0;
    
    va_list args;
    va_start(args, format);
    {
        va_list args_copy;
        va_copy(args_copy, args);
        size = vsnprintf(0, 0, format, args_copy);  // @speed?
        va_end(args_copy);
    }

    if (size > 0) {
        size += 1;  // Space for zero-terminator.
        buffer = push_array(arena, char, size);
        vsnprintf(buffer, size, format, args);
    }

    va_end(args);
    
    return buffer;
}


//
//~
#ifdef IML_NAMESPACE
#if 0
{
#endif
}
#endif

#endif // IML_MEMORY_TEMPORARY_STORAGE_H
