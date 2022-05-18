#ifndef IML_OS_LINUX_MEMORY_CPP
#define IML_OS_LINUX_MEMORY_CPP


// TODO https://www.kernel.org/doc/html/latest/core-api/memory-allocation.html


//
// @note: Allocator
//
inline
BASE_ALLOCATOR_RESERVE_SIGNATURE(os__base_reserve) {
    *size_out = size;
    return malloc(size);
}
inline
BASE_ALLOCATOR_FREE_SIGNATURE(os__base_free) {
    free(data);
}

inline Base_Allocator
os_make_base_allocator(void) {
    return make_base_allocator(os__base_reserve, 0, 0, os__base_free, 0, 0);
}

inline Base_Allocator *
os_get_base_allocator() {
#if 0
    // @note: Done in os_init now.
    if (!global_os_base_allocator.reserve) {
        global_os_base_allocator = os_make_base_allocator();
    }
#else
    assert(global_os_base_allocator.reserve);
#endif
    
    return &global_os_base_allocator;
}


//
// @note: Arena
//
inline Memory_Arena
os_make_arena(u64 chunk_size, u64 align) {
    Base_Allocator *allocator = os_get_base_allocator();
    return make_memory_arena(allocator, chunk_size, align);
}
inline Memory_Arena
os_make_arena(u64 chunk_size) {
    return os_make_arena(chunk_size, 8);
}
inline Memory_Arena
os_make_arena(void) {
    return os_make_arena(kilobytes(16));
}


#endif // IML_OS_WIN32_MEMORY_CPP
