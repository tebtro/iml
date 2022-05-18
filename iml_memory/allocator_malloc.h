#if !defined(IML_MEMORY_ALLOCATOR_MALLOC_H)
#define IML_MEMORY_ALLOCATOR_MALLOC_H

#if defined(IML_NAMESPACE)
namespace IML_NAMESPACE {
#  if 0
}
#  endif
#endif
//
//~



#include "iml_memory/memory.h"


//
// @note: Malloc Base_Allocator
//
#include <stdlib.h>

#if !OS_MAC
# include <malloc.h>
#endif


//
// @note: allocator_malloc
//

internal
BASE_ALLOCATOR_RESERVE_SIGNATURE(base_reserve__malloc) {
    Profile_Zone();
    
    *size_out = size;
    return malloc(cast(size_t)size);
}
internal
BASE_ALLOCATOR_FREE_SIGNATURE(base_free__malloc) {
    Profile_Zone();
    
    free(data);
}

inline Base_Allocator
make_base_allocator_malloc(void) {
    Profile_Zone();
    
    return make_base_allocator(base_reserve__malloc, 0, 0, base_free__malloc, 0, 0);
}

global Base_Allocator global_base_allocator_malloc = {};

inline Base_Allocator *
get_base_allocator_malloc(void) {
    Profile_Zone();
    
    if (!global_base_allocator_malloc.reserve) {
        global_base_allocator_malloc = make_base_allocator_malloc();
    }
    return &global_base_allocator_malloc;
}


//
// @note: allocator_aligned_malloc
//

struct Malloc_Memory_Header {
    void *base_pointer;
};

internal
BASE_ALLOCATOR_RESERVE_SIGNATURE(base_reserve__aligned_malloc) {
    Profile_Zone();
    
#if 0
    dont_compile;  // @important Don't use this version, looks like _aligned_malloc only alignes up to a 32byte boundary.
    
    memory_index alignment = cast(memory_index)user_data;
    
    *size_out = size;
    return _aligned_malloc(cast(size_t)size, alignment);  // @note: Looks like this only works up to 32byte alignment.
#else
    memory_index alignment = cast(memory_index)user_data;
    assert(alignment > 0 && is_pow2((u32)alignment));
    u64 size_aligned = size + sizeof(Malloc_Memory_Header) + alignment;
    size_aligned = align_pow2(size_aligned, alignment);
    
    *size_out = size;
    u8 *base_pointer = cast(u8 *) malloc(cast(size_t)size_aligned);
    u8 *data = base_pointer;
    data += sizeof(Malloc_Memory_Header);
    data += get_alignment_offset(data, alignment);
    Malloc_Memory_Header *header = cast(Malloc_Memory_Header *)(data - sizeof(Malloc_Memory_Header));
    assert((u8 *)header >= base_pointer);
    header->base_pointer = base_pointer;
    
    assert(((u64)data & (alignment - 1)) == 0);
    
    return data;
#endif
}
internal
BASE_ALLOCATOR_FREE_SIGNATURE(base_free__aligned_malloc) {
    Profile_Zone();
    
#if 0
    _aligned_free(data);
#else
    u8 *pointer = cast(u8 *)data;
    Malloc_Memory_Header *header = cast(Malloc_Memory_Header *)(pointer - sizeof(Malloc_Memory_Header));
    free(header->base_pointer);
#endif
}

inline Base_Allocator
make_base_allocator_aligned_malloc(memory_index alignment = 128) {
    Profile_Zone();
    
    return make_base_allocator(base_reserve__aligned_malloc, 0, 0, base_free__aligned_malloc, 0, (void *)alignment);
}

global Base_Allocator global_base_allocator_aligned_malloc = {};

inline Base_Allocator *
get_base_allocator_aligned_malloc(memory_index alignment = 128) {
    Profile_Zone();
    
    if (!global_base_allocator_aligned_malloc.reserve) {
        global_base_allocator_aligned_malloc = make_base_allocator_aligned_malloc(alignment);
    }
    return &global_base_allocator_aligned_malloc;
}


//
// @note: Arena
//

inline Memory_Arena
make_arena_malloc(u64 chunk_size, u64 align) {
    Profile_Zone();
    
    Base_Allocator *allocator = get_base_allocator_aligned_malloc();
    return make_memory_arena(allocator, chunk_size, align);
}
inline Memory_Arena
make_arena_malloc(u64 chunk_size) {
    Profile_Zone();
    
    return make_arena_malloc(chunk_size, 8);
}
inline Memory_Arena
make_arena_malloc(void) {
    Profile_Zone();
    
    return make_arena_malloc(kilobytes(16));
}


//
// @note: Allocator
//
inline void *
malloc_allocator(const Allocator_Mode mode, s64 size, s64 old_size, void *old_memory_pointer, void *allocator_data) {
    Profile_Zone();
    
    if (mode == ALLOCATOR_ALLOC) {
        void *new_memory = malloc(size);
        return new_memory;
    }
    else if (mode == ALLOCATOR_FREE) {
        free(old_memory_pointer);
        return nullptr;
    }
    else if (mode == ALLOCATOR_RESIZE) {
        void *new_memory = malloc(size);
        assert(size >= old_size);
        memcpy(new_memory, old_memory_pointer, old_size);
        free(old_memory_pointer);
        return new_memory;
    }
    
    invalid_path;
    return nullptr;
}


//
//~
#ifdef IML_NAMESPACE
#if 0
{
#endif
}
#endif

#endif // IML_MEMORY_ALLOCATOR_MALLOC_H
