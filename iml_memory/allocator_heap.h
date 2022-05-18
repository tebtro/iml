#ifndef IML_MEMORY_ALLOCATOR_HEAP_H
#define IML_MEMORY_ALLOCATOR_HEAP_H

#if defined(IML_NAMESPACE)
namespace IML_NAMESPACE {
#  if 0
}
#  endif
#endif
//
//~



#include "iml_memory/memory.h"


#if !BUILD_RELEASE
#  define DO_HEAP_CHECKS
#endif


struct Heap_Basic_Node{
    Heap_Basic_Node *next;
    Heap_Basic_Node *prev;
};

struct Heap_Node{
    union{
        struct{
            Heap_Basic_Node order;
            Heap_Basic_Node alloc;
            u64 size;
        };
        u8 force_size__[64];
    };
};

struct Heap{
    Memory_Arena arena_;
    Memory_Arena *arena;
    Heap_Basic_Node in_order;
    Heap_Basic_Node free_nodes;
    u64 used_space;
    u64 total_space;
};



#define heap__sent_init(s) (s)->next=(s)->prev=(s)
#define heap__insert_next(p,n) ((n)->next=(p)->next,(n)->prev=(p),(n)->next->prev=(n),(p)->next=(n))
#define heap__insert_prev(p,n) ((n)->prev=(p)->prev,(n)->next=(p),(n)->prev->next=(n),(p)->prev=(n))
#define heap__remove(n) ((n)->next->prev=(n)->prev,(n)->prev->next=(n)->next)

#if defined(DO_HEAP_CHECKS)
function void
heap_assert_good(Heap *heap){
    if (heap->in_order.next != 0){
        assert(heap->in_order.prev != 0);
        assert(heap->free_nodes.next != 0);
        assert(heap->free_nodes.prev != 0);
        for (Heap_Basic_Node *node = &heap->in_order;;){
            assert(node->next->prev == node);
            assert(node->prev->next == node);
            node = node->next;
            if (node == &heap->in_order){
                break;
            }
        }
        for (Heap_Basic_Node *node = &heap->free_nodes;;){
            assert(node->next->prev == node);
            assert(node->prev->next == node);
            node = node->next;
            if (node == &heap->free_nodes){
                break;
            }
        }
    }
}
#else
#define heap_assert_good(heap) ((void)(heap))
#endif

function void
heap_init(Heap *heap, Base_Allocator *allocator){
    Profile_Zone();
    
    heap->arena_ = make_memory_arena(allocator);
    heap->arena = &heap->arena_;
    heap__sent_init(&heap->in_order);
    heap__sent_init(&heap->free_nodes);
    heap->used_space = 0;
    heap->total_space = 0;
}

function void
heap_init(Heap *heap, Memory_Arena *arena){
    Profile_Zone();
    
    heap->arena = arena;
    heap__sent_init(&heap->in_order);
    heap__sent_init(&heap->free_nodes);
    heap->used_space = 0;
    heap->total_space = 0;
}

function Base_Allocator*
heap_get_base_allocator(Heap *heap){
    return(heap->arena->base_allocator);
}

function void
heap_free_all(Heap *heap){
    Profile_Zone();
    
    if (heap->arena == &heap->arena_){
        linalloc_clear(heap->arena);
    }
    block_zero_struct(heap);
}

function void
heap__extend(Heap *heap, void *memory, u64 size){
    Profile_Zone();
    
    heap_assert_good(heap);
    if (size >= sizeof(Heap_Node)){
        Heap_Node *new_node = (Heap_Node*)memory;
        heap__insert_prev(&heap->in_order, &new_node->order);
        heap__insert_next(&heap->free_nodes, &new_node->alloc);
        new_node->size = size - sizeof(*new_node);
        heap->total_space += size;
    }
    heap_assert_good(heap);
}

function void
heap__extend_automatic(Heap *heap, u64 size){
    Profile_Zone();
    
    void *memory = push_array(heap->arena, u8, size);
    heap__extend(heap, memory, size);
}

function void*
heap__reserve_chunk(Heap *heap, Heap_Node *node, u64 size){
    Profile_Zone();
    
    u8 *ptr = (u8*)(node + 1);
    assert(node->size >= size);
    u64 left_over_size = node->size - size;
    if (left_over_size > sizeof(*node)){
        u64 new_node_size = left_over_size - sizeof(*node);
        Heap_Node *new_node = (Heap_Node*)(ptr + size);
        heap__insert_next(&node->order, &new_node->order);
        heap__insert_next(&node->alloc, &new_node->alloc);
        new_node->size = new_node_size;
    }
    heap__remove(&node->alloc);
    node->alloc.next = 0;
    node->alloc.prev = 0;
    node->size = size;
    heap->used_space += sizeof(*node) + size;
    return(ptr);
}

function void*
heap_allocate(DEBUG_MEMORY_PARAMETER
              Heap *heap, u64 size) {
    Profile_Zone();
    
    b32 first_try = true;
    for (;;){
        if (heap->in_order.next != 0){
            heap_assert_good(heap);
            u64 aligned_size = (size + sizeof(Heap_Node) - 1);
            aligned_size = aligned_size - (aligned_size%sizeof(Heap_Node));
            for (Heap_Basic_Node *n = heap->free_nodes.next;
                 n != &heap->free_nodes;
                 n = n->next){
                Heap_Node *node = cast_from_member(Heap_Node, alloc, n);
                if (node->size >= aligned_size){
                    void *ptr = heap__reserve_chunk(heap, node, aligned_size);
                    heap_assert_good(heap);
                    return(ptr);
                }
            }
            heap_assert_good(heap);
        }
        
        if (first_try){
            u64 extension_size = clamp_bot(KB(64), size*2);
            heap__extend_automatic(heap, extension_size);
            first_try = false;
        }
        else{
            break;
        }
    }
    return(0);
}

function void
heap__merge(Heap *heap, Heap_Node *l, Heap_Node *r){
    Profile_Zone();
    
    if (&l->order != &heap->in_order && &r->order != &heap->in_order &&
        l->alloc.next != 0 && l->alloc.prev != 0 &&
        r->alloc.next != 0 && r->alloc.prev != 0){
        u8 *ptr = (u8*)(l + 1) + l->size;
        if (ptr_dif(ptr, r) == 0){
            heap__remove(&r->order);
            heap__remove(&r->alloc);
            heap__remove(&l->alloc);
            l->size += r->size + sizeof(*r);
            heap__insert_next(&heap->free_nodes, &l->alloc);
        }
    }
}

function void
heap_free(DEBUG_MEMORY_PARAMETER
          Heap *heap, void *memory) {
    Profile_Zone();
    
    if (heap->in_order.next != 0 && memory != 0){
        Heap_Node *node = ((Heap_Node*)memory) - 1;
        assert(node->alloc.next == 0, "Probably trying to free already freed memory!");
        assert(node->alloc.prev == 0, "Probably trying to free already freed memory!");
        heap->used_space -= sizeof(*node) + node->size;
        heap_assert_good(heap);
        heap__insert_next(&heap->free_nodes, &node->alloc);
        heap_assert_good(heap);
        heap__merge(heap, node, cast_from_member(Heap_Node, order, node->order.next));
        heap_assert_good(heap);
        heap__merge(heap, cast_from_member(Heap_Node, order, node->order.prev), node);
        heap_assert_good(heap);
    }
}

#define heap_array(heap, T, c) (T*)(heap_allocate(DEBUG_MEMORY_NAME("heap_array")  (heap), sizeof(T)*(c)))

//
// @note: base_allocator_heap
//

inline
BASE_ALLOCATOR_RESERVE_SIGNATURE(base_reserve__heap) {
    Profile_Zone();
    
    Heap *heap = (Heap*)user_data;
    void *memory = heap_allocate(DEBUG_MEMORY_PASS  heap, size);
    *size_out = size;
    return(memory);
}

inline
BASE_ALLOCATOR_FREE_SIGNATURE(base_free__heap) {
    Profile_Zone();
    
    Heap *heap = (Heap*)user_data;
    heap_free(DEBUG_MEMORY_PASS  heap, data);
}

inline Base_Allocator
make_base_allocator_heap(Heap *heap) {
    return make_base_allocator(base_reserve__heap, 0, 0, base_free__heap, 0, heap);
}



//
//~
#ifdef IML_NAMESPACE
#if 0
{
#endif
}
#endif

#endif // IML_MEMORY_ALLOCATOR_HEAP_H

