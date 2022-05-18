#if !defined(IML_MEMORY_ALLOCATOR_H)
#define IML_MEMORY_ALLOCATOR_H


#include "iml_types.h"


enum Allocator_Mode {
    ALLOCATOR_ALLOC,
    ALLOCATOR_FREE,
    ALLOCATOR_RESIZE,
};

//#define ALLOCATOR_SIGNATURE(name)  void *name(const Allocator_Mode mode, s64 size, s64 old_size, void *old_memory_pointer, void *allocator_data)
typedef void *(*Allocator_Function)(const Allocator_Mode mode, s64 size, s64 old_size, void *old_memory_pointer, void *allocator_data);


#if 0
inline void *malloc_allocator(const Allocator_Mode mode, s64 size, s64 old_size, void *old_memory_pointer, void *allocator_data) {
    if (mode == ALLOCATOR_ALLOC) {
        return malloc(size);
    }
    else if (mode == ALLOCATOR_FREE) {
        free(old_memory_pointer);
        return nullptr;
    }
    else if (mode == ALLOCATOR_RESIZE) {
        return realloc(old_memory_pointer, size);
    }
    
    invalid_path;
    return nullptr;
}
#endif



#endif // IML_MEMORY_ALLOCATOR_H
