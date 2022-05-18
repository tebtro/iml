#if !defined(IML_POOL_H)
#define IML_POOL_H

#if defined(IML_NAMESPACE)
namespace IML_NAMESPACE {
#  if 0
}
#  endif
#endif
//
//~


#include "iml_types.h"
#include "iml_memory/memory.h"


#if 0

inline s64 pad_to_alignment(s64 current, s64 align) {
    assert(align >= 1);
    
    s64 minum = current & (align - 1);
    if (minum) {
        assert((current % align) != 0);
        current += align - minum;
    }
    
    return current;
}


struct Pool {
    const s64 DEFAULT_NEW_CHUNK_SIZE = 4096 * 4;
    Array<void *> chunks;
    
    void *current_chunk_cursor = nullptr;
    s64 current_chunk_available = 0;
    
    Base_Allocator *allocator;
    
    Pool(Base_Allocator *allocator = os_get_base_allocator()) {
        this->allocator = allocator;
        
        void *data = base_allocate(allocator, DEFAULT_NEW_CHUNK_SIZE).data;
        current_chunk_cursor = data;
        current_chunk_available = DEFAULT_NEW_CHUNK_SIZE;
        chunks.add(data);
    }
    
    inline void *allocate(s64 amount) {
        const int alignment = 8; // @temporary
        // Ensure the _next_ allocation will be properly aligned.
        // Kind of a @hack, but doing this in the short term in order
        // to make sure we don't crash on something like an ARM chip
        // where the alignment of things matter a little more.
        amount = pad_to_alignment(amount, alignment);
        
        if (current_chunk_available >= amount) {
            void *result = current_chunk_cursor;
            current_chunk_cursor = ((u8 *)current_chunk_cursor) + amount;
            current_chunk_available -= amount;
            return result;
        }
        
        // @note All chunks exhausted or amount simply doesn't fit ...
        auto amount_to_allocate = DEFAULT_NEW_CHUNK_SIZE;
        if (amount_to_allocate < amount)  amount_to_allocate = amount;
        
        void *data = base_allocate(allocator, amount_to_allocate).data;
        current_chunk_cursor = ((u8 *)data) + amount;
        current_chunk_available = amount_to_allocate - amount;
        chunks.add(data);
        
        return data;
    }
    
    void free() {
        For (chunks) {
            base_free(allocator, it);
        }
        chunks.free();
    }
};

#else

struct Pool {
    Memory_Arena arena = {
        .base_allocator = &global_os_base_allocator,
        .cursor_node = 0,
        .chunk_size  = kilobytes(64),
        .alignment   = 8
    };
    
    struct Free_Node {
        Free_Node *next;
        Free_Node *prev;
        Data data;
    };
    Free_Node free_list = {
        .next = &free_list,
        .prev = &free_list,
        .data = { .size = 0, .data = null },
    };
    Free_Node *free_nodes_stack = null;  // free free_nodes
    
    
    inline void init(Base_Allocator *allocator) {
        arena.base_allocator = allocator;
    }
    
    inline void free(void) {
        linalloc_clear(&arena);
    }
    
    inline void reset(void) {
        linalloc_clear(&arena);  // @todo reset() should not free memory.
    }
    
    
    inline void *allocate(u64 amount) {
        Profile_Zone();
        
        const u64 _debug_original_amount = amount;
        const u64 alignment = arena.alignment;
        amount += alignment;
        amount += sizeof(Data);
        
        Data data;
        
        {
            Profile_Zone_Named("search freelist");
            
            Free_Node *free_list_pointer = &free_list;
            
            for (Free_Node *free_node = free_list.next;
                 free_node != &free_list;
                 free_node = free_node->next) {
                assert(free_node);
                if (free_node->data.size >= amount) {
                    data = free_node->data;
                    dll_remove(free_node);
                    sll_stack_push(free_nodes_stack, free_node);
                    goto got_memory;
                }
            }
        }
        
        {
            Profile_Zone_Named("allocate new memory block");
            data = linalloc_push(DEBUG_MEMORY_NAME("Pool::allocate")  &arena, amount, align_no_clear(1));
        }
        
        got_memory:;
        
        u8 *allocation_base = data.data + sizeof(Data);
        memory_index alignment_offset = get_alignment_offset(allocation_base, alignment);
        allocation_base += alignment_offset;
        
        assert((cast(memory_index) allocation_base & (alignment - 1)) == 0, "Allocation is not aligned properly!");
        assert((allocation_base + _debug_original_amount) <= (data.data + data.size), "Allocation is overflowing required memory!");
        Data *header = (cast(Data *) allocation_base) - 1;
        *header = data;
        
        return allocation_base;
    }
    
    inline void free(void *memory) {
        Profile_Zone();
        
        Free_Node *node = free_nodes_stack;
        if (node) {
            sll_stack_pop(free_nodes_stack);
        }
        else {
            node = push_struct(&arena, Free_Node, no_clear());
        }
        
        Data *header = (cast(Data *) memory) - 1;
        node->data = *header;
        
        dll_insert_back(&free_list, node);
    }
};
#endif


//
// @note: base_allocator_pool
//
function
BASE_ALLOCATOR_RESERVE_SIGNATURE(base_reserve__pool) {
    Profile_Zone();
    
    Pool *pool = cast(Pool *) user_data;
    void *memory = pool->allocate(size);
    *size_out = size;
    return memory;
}

function
BASE_ALLOCATOR_FREE_SIGNATURE(base_free__pool) {
    Profile_Zone();
    
    Pool *pool = cast(Pool *) user_data;
    pool->free(data);
}

inline Base_Allocator
make_base_allocator_pool(Pool *pool) {
    return make_base_allocator(base_reserve__pool, 0, 0, base_free__pool, 0, pool);
}



//
// @note: Tests
//
#include "iml_test_suit.h"

global_unit("Pool") {
    test("@todo") {};  // @todo
    
    test("crasher") {  // Crash if free_list sentinel wasn't initialized.
        const u64 N = 2;
        const u64 N2 = 1000;
        
        for (u64 i = 0; i < N; i++) {
            void *pointers[N2];
            Pool pool;
            defer { pool.free(); };
            
            for (u64 i = 0; i < N2; i++) {
                pointers[i] = pool.allocate(sizeof(u64) * 64);
            }
            
            for (u64 i = 0; i < N2; i++) {
                pool.free(pointers[i]);
                pool.allocate(sizeof(u64) * 64);
            }
        }
    };
    
    test("alignment") {
        Pool pool;
        defer { pool.free(); };
        
        pool.arena.alignment = 2;
        
        void *p1 = pool.allocate(sizeof(u8) *  9);
        void *p2 = pool.allocate(sizeof(u8) * 10);
        void *p3 = pool.allocate(sizeof(u8) * 11);
        void *p4 = pool.allocate(sizeof(u8) * 12);
        void *p5 = pool.allocate(sizeof(u8) * 13);
        void *p6 = pool.allocate(sizeof(u8) * 14);
        void *p7 = pool.allocate(sizeof(u8) * 15);
        void *p8 = pool.allocate(sizeof(u8) * 16);
        void *p9 = pool.allocate(sizeof(u8) * 64);
        pool.free(p1);
        pool.free(p2);
        pool.free(p3);
        pool.free(p4);
        pool.free(p5);
        pool.free(p6);
        pool.free(p7);
        pool.free(p8);
        pool.free(p9);
        
        pool.arena.alignment = 16;
        
        pool.allocate(sizeof(u8) * 16);  // first allocation will be aligned, because it's at the page boundary.
        pool.allocate(sizeof(u8) * 16);
    };
};


//
//~
#ifdef IML_NAMESPACE
#if 0
{
#endif
}
#endif

#endif//IML_POOL_H
