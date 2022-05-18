#if !defined(IML_COMMON_ARRAY_SYSTEM_H)
#define IML_COMMON_ARRAY_SYSTEM_H


//
// @note: Common array allocator system / Memory recycler
//
#include "iml_types.h"
#include "iml_memory/memory.h"


inline void init_common_array_system(void);
inline Base_Allocator *get_default_array_base_allocator(void);
global Base_Allocator global_default_array_allocator = {};


#endif // IML_COMMON_ARRAY_SYSTEM_H

//
// @note: Implementation
//
#if defined(IML_COMMON_ARRAY_SYSTEM_IMPLEMENTATION)
#undef IML_COMMON_ARRAY_SYSTEM_IMPLEMENTATION
#if !defined(IML_COMMON_ARRAY_SYSTEM_CPP)
#define IML_COMMON_ARRAY_SYSTEM_CPP


#if 0

//
// @note: Global array heap allocator
//
// Uses a global heap for all arrays.
// On an resize it just copies all the data to a new big enough block.
// The old block will be put back into the heap, and can be used by other arrays.
//
#include "iml_memory/allocator_heap.h"

global Heap global_array_heap_;

inline void init_common_array_system(void) {
    heap_init(&global_array_heap_, os_get_base_allocator());
    global_default_array_allocator = make_base_allocator_heap(&global_array_heap_);
}
//heap_free_all(&global_array_heap);

inline Base_Allocator *get_default_array_base_allocator(void) {
    assert(global_default_array_allocator.user_data);
    return &global_default_array_allocator;
}

#else

//
// @note: Pool memory recycling
//
#include "iml_pool.h"

global Pool global_array_pool_ = {};

inline void init_common_array_system(void) {
    assert(global_default_array_allocator.user_data == null);
    global_default_array_allocator = make_base_allocator_pool(&global_array_pool_);
}

inline Base_Allocator *get_default_array_base_allocator(void) {
    assert(global_default_array_allocator.user_data);
    return &global_default_array_allocator;
}

#endif


#endif // IML_COMMON_ARRAY_SYSTEM_CPP
#endif // IML_COMMON_ARRAY_SYSTEM_IMPLEMENTATION

