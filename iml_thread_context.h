#if !defined(IML_THREAD_CONTEXT_H)
#define IML_THREAD_CONTEXT_H

#if defined(IML_NAMESPACE)
namespace IML_NAMESPACE {
#  if 0
}
#  endif
#endif
//
//~


dont_compile;
// @todo
// @todo
// @todo


#include "iml_memory/memory.h"


enum Thread_Kind : u32 {
    THREAD_KIND_main,
    THREAD_KIND_main_coroutine,
    THREAD_KIND_async_tasks,
};

struct Thread_Context {
    Thread_Kind kind;
    Base_Allocator *allocator;
    
    Temporary_Storage *temporary_storage;
    
    void *user_data;
};


#if 0
function void
thread_ctx_init(Thread_Context *tctx, Thread_Kind kind, Base_Allocator *allocator,
                Base_Allocator *prof_allocator) {
    block_zero_struct(tctx);
    tctx->kind = kind;
    tctx->allocator = allocator;
    tctx->node_arena = make_arena(allocator, KB(4), 8);
    
    tctx->prof_allocator = prof_allocator;
    tctx->prof_id_counter = 1;
    tctx->prof_arena = make_arena(prof_allocator, KB(16));
}

function void
thread_ctx_release(Thread_Context *tctx){
    for (Arena_Node *node = tctx->free_arenas;
         node != 0;
         node = node->next){
        linalloc_clear(&node->arena);
    }
    for (Arena_Node *node = tctx->used_first;
         node != 0;
         node = node->next){
        linalloc_clear(&node->arena);
    }
    linalloc_clear(&tctx->node_arena);
    block_zero_struct(tctx);
}
#endif


function void
thread_ctx_init(Thread_Context *tctx, Thread_Kind kind, Base_Allocator *allocator,
                Base_Allocator *prof_allocator) {
    block_zero_struct(tctx);
    tctx->kind = kind;
    tctx->allocator = allocator;
    tctx->temporary_storage = 0;  // @todo
}


inline Thread_Context make_thread_context() {
    Thread_Context tctx = {};
    thread_ctx_init(&tctx, THREAD_KIND_main, make_base_allocator_malloc(), 0);
    return tctx;
}

thread_local Thread_Context global_thread_context = make_thread_context();


//
//~
#ifdef IML_NAMESPACE
#  if 0
{
#  endif
}
#endif

#endif//IML_THREAD_CONTEXT_H
