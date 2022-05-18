#ifndef IML_MEMORY_H
#define IML_MEMORY_H

#if defined(IML_NAMESPACE)
namespace IML_NAMESPACE {
#  if 0
}
#  endif
#endif
//
//~


#include "iml_profiler.h"
#include "os/os.h"
#include "iml_types.h"
#include "iml_string.h"
#include "iml_memory/allocator.h"


//
// @note
//
void *memcpy(void *destination, const void *source, size_t size);
int memcmp(const void *a, const void *b, size_t size);


//
// @note
//

#define DEBUG_NAME__(a, b, c)  a "|" #b "|" #c
#define DEBUG_NAME_(a, b, c)  DEBUG_NAME__(a, b, c)
#define DEBUG_NAME(name)  DEBUG_NAME_(__FILE__, __LINE__, __COUNTER__)


#if BUILD_DEBUG
#  define DEBUG_MEMORY_NAME(name)  DEBUG_NAME_(__FILE__, __LINE__, __COUNTER__) "|" name,
#  define DEBUG_MEMORY_PARAMETER   const char *GUID,
#  define DEBUG_MEMORY_PASS        GUID,
#else
#  define DEBUG_MEMORY_NAME(name)
#  define DEBUG_MEMORY_PARAMETER
#  define DEBUG_MEMORY_PASS
#endif


#if 0
#  define DEBUG_RECORD_BLOCK_ALLOCATION(new_block) { \
iml_printf("arena__new_node: capacity=%\n", new_block->cursor.capacity); \
}
#  define DEBUG_RECORD_BLOCK_FREE(memory_block)
#  define DEBUG_RECORD_BLOCK_TRUNCATE(current_block)
#  define DEBUG_RECORD_ALLOCATION(current_block, GUID, size, size_unaligned, offset_in_block) { \
iml_printf("%: size=%, size_unaligned=%, offset_in_block=%\n", GUID, size, size_unaligned, offset_in_block); \
}
// @todo DEBUG_RECORD_POP
#else
#  define DEBUG_RECORD_BLOCK_ALLOCATION(new_block)
#  define DEBUG_RECORD_BLOCK_FREE(memory_block)
#  define DEBUG_RECORD_BLOCK_TRUNCATE(current_block)
#  define DEBUG_RECORD_ALLOCATION(current_block, GUID, size, size_unaligned, offset_in_block)
// @todo DEBUG_RECORD_POP
#endif


//~ @note: Data
struct Data {
    u64 size;
    u8 *data;
};

//~ @note: Base_Allocator
enum Memory_Access_Flags : u32 {
    MEMORY_ACCESS_read    = 0x1,
    MEMORY_ACCESS_write   = 0x2,
    MEMORY_ACCESS_execute = 0x4,
};

#define BASE_ALLOCATOR_RESERVE_SIGNATURE(name)     void *name(DEBUG_MEMORY_PARAMETER  void *user_data, u64 size,   u64 *size_out)
#define BASE_ALLOCATOR_COMMIT_SIGNATURE(name)      void  name(DEBUG_MEMORY_PARAMETER  void *user_data, void *data, u64 size)
#define BASE_ALLOCATOR_UNCOMMIT_SIGNATURE(name)    void  name(DEBUG_MEMORY_PARAMETER  void *user_data, void *data, u64 size)
#define BASE_ALLOCATOR_FREE_SIGNATURE(name)        void  name(DEBUG_MEMORY_PARAMETER  void *user_data, void *data)
#define BASE_ALLOCATOR_SET_ACCESS_SIGNATURE(name)  void  name(DEBUG_MEMORY_PARAMETER  void *user_data, void *data, u64 size, Memory_Access_Flags flags)

typedef BASE_ALLOCATOR_RESERVE_SIGNATURE(Base_Allocator_Reserve_Function);
typedef BASE_ALLOCATOR_COMMIT_SIGNATURE(Base_Allocator_Commit_Function);
typedef BASE_ALLOCATOR_UNCOMMIT_SIGNATURE(Base_Allocator_Uncommit_Function);
typedef BASE_ALLOCATOR_FREE_SIGNATURE(Base_Allocator_Free_Function);
typedef BASE_ALLOCATOR_SET_ACCESS_SIGNATURE(Base_Allocator_Set_Access_Function);

struct Base_Allocator {
    Base_Allocator_Reserve_Function    *reserve;
    Base_Allocator_Commit_Function     *commit;
    Base_Allocator_Uncommit_Function   *uncommit;
    Base_Allocator_Free_Function       *free;
    Base_Allocator_Set_Access_Function *set_access;
    
    void *user_data;
};

//~ @note: Memory_Cursor
struct Memory_Cursor {
    u64 capacity;
    umm pos;
    u8 *base;
};

struct Temporary_Memory_Cursor {
    Memory_Cursor *cursor;
    umm pos;
};

struct Memory_Cursor_Node {
    union {
        Memory_Cursor_Node *next;
        Memory_Cursor_Node *prev;
    };
    
    Memory_Cursor cursor;
};

//~ @note: Memory_Arena
struct Memory_Arena {
    Base_Allocator *base_allocator;
    Memory_Cursor_Node *cursor_node;
    u64 chunk_size;  // minimum_chunk_size
    u64 alignment;
};

struct Temporary_Memory_Arena {
    Memory_Arena *arena;
    Memory_Cursor_Node *cursor_node;
    u64 pos;
};

//~ @note: Temporary_Memory
enum Linear_Allocator_Kind : s32 {
    LINEAR_ALLOCATOR_cursor = 1,
    LINEAR_ALLOCATOR_arena  = 2,
};

struct Temporary_Memory {
    Linear_Allocator_Kind kind;
    union {
        Temporary_Memory_Cursor cursor;
        Temporary_Memory_Arena  arena;
    };
};

struct Temporary_Memory_Block {
    Temporary_Memory temp;
    
    inline Temporary_Memory_Block(Temporary_Memory temp);
    inline Temporary_Memory_Block(Memory_Arena *arena);
    inline ~Temporary_Memory_Block();
    inline void restore();
    inline void keep_memory();
};


//
// @note: Data
//

inline Data
make_data(void *memory, u64 size) {
    Data data = {
        .size = size,
        .data = cast(u8 *)memory,
    };
    return data;
}

#define make_data_struct(s)  make_data((s), sizeof(*(s)))

global_const Data zero_data = {};

#define data_initr(m, s)      {(u8 *)(m), (s)}
#define data_initr_struct(s)  {(u8 *)(s), sizeof(*(s))}
#define data_initr_array(a)   {(u8 *)(a), sizeof(a)}
#define data_initr_string(s)  {(u8 *)(s), sizeof(s) - 1}


//
// @note: Block
//

inline void
block_zero(void *mem, u64 size) {
    Profile_Zone();
    memset(mem, 0, size);
}

inline void
block_zero(Data data) {
    Profile_Zone();
    block_zero(data.data, data.size);
}

inline void
block_fill_ones(void *mem, u64 size) {
    Profile_Zone();
    memset(mem, 0xFF, size);
}

inline void
block_fill_ones(Data data) {
    Profile_Zone();
    block_fill_ones(data.data, data.size);
}

inline void
block_copy(void *dst, const void *src, u64 size) {
    Profile_Zone();
    memcpy(dst, src, size);
}

function b32
block_match(void *a, void *b, u64 size) {
    Profile_Zone();
    b32 result = true;
    for (u8 *pa = cast(u8*)a, *pb = cast(u8*)b, *ea = pa + size; pa < ea; pa += 1, pb += 1) {
        if (*pa != *pb) {
            result = false;
            break;
        }
    }
    return result;
}

function s32
block_compare(const void *a, const void *b, u64 size) {
    Profile_Zone();
    return memcmp(a, b, size);
}

function void
block_fill_u8(void *a, u64 size, u8 val) {
    Profile_Zone();
    for (u8 *ptr = cast(u8*)a, *e = ptr + size; ptr < e; ptr += 1) {
        *ptr = val;
    }
}

function void
block_fill_u16(void *a, u64 size, u16 val) {
    Profile_Zone();
    assert(size % sizeof(u16) == 0);
    u64 count = size / sizeof(u16);
    for (u16 *ptr = cast(u16*)a, *e = ptr + count; ptr < e; ptr += 1) {
        *ptr = val;
    }
}

function void
block_fill_u32(void *a, u64 size, u32 val) {
    Profile_Zone();
    assert(size % sizeof(u32) == 0);
    u64 count = size / sizeof(u32);
    for (u32 *ptr = cast(u32*)a, *e = ptr + count; ptr < e; ptr += 1) {
        *ptr = val;
    }
}

function void
block_fill_u64(void *a, u64 size, u64 val) {
    Profile_Zone();
    assert(size % sizeof(u64) == 0);
    u64 count = size / sizeof(u64);
    for (u64 *ptr = cast(u64*)a, *e = ptr + count; ptr < e; ptr += 1) {
        *ptr = val;
    }
}

#define block_zero_struct(p) block_zero((p), sizeof(*(p)))
#define block_zero_array(a) block_zero((a), sizeof(a))
#define block_zero_dynamic_array(p,c) block_zero((p), sizeof(*(p))*(c))

#define block_copy_struct(d,s) block_copy((d), (s), sizeof(*(d)))
#define block_copy_array(d,s) block_copy((d), (s), sizeof(d))
#define block_copy_dynamic_array(d,s,c) block_copy((d), (s), sizeof(*(d))*(c))

#define block_match_struct(a,b) block_match((a), (b), sizeof(*(a)))
#define block_match_array(a,b) block_match((a), (b), sizeof(a))

#if 0  // @todo
inline void
block_range_copy__inner(void *dst, void *src, Range_u64 range, i64 shift) {
    Profile_Zone();
    
    block_copy(cast(u8*)dst + range.first + shift, cast(u8*)src + range.first, range.max - range.min);
}

inline void
block_range_copy__inner(void *dst, void *src, Range_u64 range, i64 shift, u64 item_size) {
    Profile_Zone();
    
    range.first *= item_size;
    range.one_past_last *= item_size;
    shift *= item_size;
    block_range_copy__inner(dst, src, range, shift);
}

#define block_range_copy(d,s,r,h) block_range_copy__inner((d),(s),Iu64(r),(i64)(h))
#define block_range_copy_sized(d,s,r,h,i) block_range_copy__inner((d),(s),Iu64(r),(i64)(h),(i))
#define block_range_copy_typed(d,s,r,h) block_range_copy_sized((d),(s),(r),(h),sizeof(*(d)))

inline void
block_copy_array_shift__inner(void *dst, void *src, u64 it_size, Range_i64 range, i64 shift) {
    Profile_Zone();
    
    u8 *dptr = cast(u8*)dst;
    u8 *sptr = cast(u8*)src;
    dptr += it_size*(range.first + shift);
    sptr += it_size*range.first;
    block_copy(dptr, sptr, cast(u64)(it_size*(range.one_past_last - range.first)));
}

inline void
block_copy_array_shift__inner(void *dst, void *src, u64 it_size, Range_i32 range, i64 shift) {
    Profile_Zone();
    
    u8 *dptr = cast(u8*)dst;
    u8 *sptr = cast(u8*)src;
    dptr += it_size*(range.first + shift);
    sptr += it_size*range.first;
    block_copy(dptr, sptr, cast(u64)(it_size*(range.one_past_last - range.first)));
}

#define block_copy_array_shift(d,s,r,h) block_copy_array_shift__inner((d),(s),sizeof(*(d)),(r),(h))
#endif


//
// @note: Base_Allocator
//

function
BASE_ALLOCATOR_RESERVE_SIGNATURE(base_reserve__noop) {
    *size_out = 0;
    return null;
}
function BASE_ALLOCATOR_COMMIT_SIGNATURE(base_commit__noop) {}
function BASE_ALLOCATOR_UNCOMMIT_SIGNATURE(base_uncommit__noop) {}
function BASE_ALLOCATOR_FREE_SIGNATURE(base_free__noop) {}
function BASE_ALLOCATOR_SET_ACCESS_SIGNATURE(base_set_access__noop) {}

function Base_Allocator
make_base_allocator(Base_Allocator_Reserve_Function    *function_reserve,
                    Base_Allocator_Commit_Function     *function_commit,
                    Base_Allocator_Uncommit_Function   *function_uncommit,
                    Base_Allocator_Free_Function       *function_free,
                    Base_Allocator_Set_Access_Function *function_set_access,
                    void *user_data) {
    Profile_Zone();
    
    if (!function_reserve)     function_reserve    = base_reserve__noop;
    if (!function_commit)      function_commit     = base_commit__noop;
    if (!function_uncommit)    function_uncommit   = base_uncommit__noop;
    if (!function_free)        function_free       = base_free__noop;
    if (!function_set_access)  function_set_access = base_set_access__noop;
    
    Base_Allocator base_allocator = {
        .reserve    = function_reserve,
        .commit     = function_commit,
        .uncommit   = function_uncommit,
        .free       = function_free,
        .set_access = function_set_access,
        .user_data  = user_data
    };
    
    return base_allocator;
}


inline Data
base_allocate__inner(DEBUG_MEMORY_PARAMETER
                     Base_Allocator *allocator, u64 size) {
    Profile_Zone();
    
    assert(allocator);
    assert(allocator->reserve);
    assert(allocator->commit);
    
    u64 full_size = 0;
    void *memory = allocator->reserve(DEBUG_MEMORY_PASS  allocator->user_data, size, &full_size);
    allocator->commit(DEBUG_MEMORY_PASS  allocator->user_data, memory, full_size);
    return make_data(memory, full_size);
}

inline void
base_free__inner(DEBUG_MEMORY_PARAMETER
                 Base_Allocator *allocator, void *data) {
    Profile_Zone();
    
    assert(allocator);
    assert(allocator->free);
    
    if (data) {
        allocator->free(DEBUG_MEMORY_PASS  allocator->user_data, data);
    }
}

#define base_allocate(allocator, size)  base_allocate__inner(DEBUG_MEMORY_NAME("base_allocate")  (allocator), (size))
#define base_array_location(...)  (base_allocate__inner(__VA_ARGS__).data)
#define base_array(allocator, Type, count)  (Type *)base_array_location(DEBUG_MEMORY_NAME("base_array")  (allocator), sizeof(Type) * (count))

#define base_free(allocator, data)  base_free__inner(DEBUG_MEMORY_NAME("base_free")  (allocator), (data))


//
// @note: Arena push and alignment stuff
//
enum Arena_Push_Flags : u32 {
    ARENA_PUSH_no_clear_to_zero = 0x1,
};

struct Arena_Push_Params {
    u32 flags     = 0;
    u32 alignment = 0;
};

inline Arena_Push_Params
align_no_clear(u32 alignment) {
    Arena_Push_Params params = {};
    params.flags |= ARENA_PUSH_no_clear_to_zero;
    params.alignment = alignment;
    return params;
}

inline Arena_Push_Params
align(u32 alignment, b32 clear = true) {
    Arena_Push_Params params = {};
    if (clear) {
        params.flags &= ~ARENA_PUSH_no_clear_to_zero;
    }
    else {
        params.flags |= ARENA_PUSH_no_clear_to_zero;
    }
    params.alignment = alignment;
    return params;
}

inline Arena_Push_Params
no_clear(void) {
    Arena_Push_Params params = {};
    params.flags |= ARENA_PUSH_no_clear_to_zero;
    return params;
}

inline memory_index
get_alignment_offset(void *pointer, memory_index alignment) {
    memory_index alignment_offset = 0;
    
    memory_index result_pointer = cast(memory_index)pointer;
    assert(is_pow2((u32)alignment));
    memory_index alignment_mask = alignment - 1;
    if (result_pointer & alignment_mask) {
        alignment_offset = alignment - (result_pointer & alignment_mask);
    }
    assert(in_range(0, alignment_offset, alignment));
    
    return alignment_offset;
}
inline memory_index
get_alignment_offset(Memory_Cursor *cursor, memory_index alignment) {
    return get_alignment_offset(cursor->base + cursor->pos, alignment);
}
inline memory_index
get_alignment_offset(Memory_Arena *arena, memory_index alignment) {
    return get_alignment_offset(&arena->cursor_node->cursor, alignment);
}

inline memory_index
get_effective_size_for(Memory_Arena *arena, memory_index size_unaligned, memory_index alignment) {
    memory_index size = size_unaligned;
    
    memory_index alignment_offset = get_alignment_offset(arena, alignment);
    size += alignment_offset;
    
    return size;
}


//
// @note: Memory_Cursor
//

inline Memory_Cursor
make_memory_cursor(void *base, u64 size) {
    Profile_Zone();
    Memory_Cursor cursor = {
        .capacity = size,
        .pos = 0,
        .base = cast(u8 *)base
    };
    return cursor;
}
inline Memory_Cursor
make_memory_cursor(Data data) {
    Profile_Zone();
    return make_memory_cursor(data.data, data.size);
}
inline Memory_Cursor
make_memory_cursor(Base_Allocator *allocator, u64 size) {
    Profile_Zone();
    Data memory = base_allocate(allocator, size);
    return make_memory_cursor(memory);
}

inline Data
linalloc_push(DEBUG_MEMORY_PARAMETER
              Memory_Cursor *cursor, u64 size) {
    Profile_Zone();
    Data result = {};
    if ((cursor->pos + size) <= cursor->capacity) {
        result.data = cursor->base + cursor->pos;
        result.size = size;
        cursor->pos += size;
    }
    return result;
}
inline void
linalloc_pop(DEBUG_MEMORY_PARAMETER
             Memory_Cursor *cursor, u64 size) {
    Profile_Zone();
    if (cursor->pos > size) {
        cursor->pos -= size;
    }
    else {
        cursor->pos = 0;
    }
}
inline Data
linalloc_align(DEBUG_MEMORY_PARAMETER
               Memory_Cursor *cursor, u64 alignment, bool clear_to_zero = false) {
    Profile_Zone();
    
#if 0
    u64 pos = round_up_u64(cursor->pos, alignment);
#else
    u64 pos = cursor->pos;
    pos += get_alignment_offset(cursor, alignment);
#endif
    
    u64 new_size = pos - cursor->pos;
    Data result = linalloc_push(DEBUG_MEMORY_PASS  cursor, new_size);
    
    if (clear_to_zero) {
        block_zero(result);
    }
    return result;
}

inline Temporary_Memory_Cursor
linalloc_begin_temporary(Memory_Cursor *cursor) {
    Profile_Zone();
    Temporary_Memory_Cursor temp = {
        .cursor = cursor,
        .pos = cursor->pos
    };
    return temp;
}
inline void
linalloc_end_temporary(Temporary_Memory_Cursor temp) {
    Profile_Zone();
    temp.cursor->pos = temp.pos;
}
inline void
linalloc_keep_temporary(Temporary_Memory_Cursor *temp) {
    Profile_Zone();
    *temp = linalloc_begin_temporary(temp->cursor);
}

inline void
linalloc_clear(Memory_Cursor *cursor) {
    Profile_Zone();
    cursor->pos = 0;
}


//
// @note: Memory_Arena
//

inline Memory_Arena
make_memory_arena(Base_Allocator *allocator, u64 chunk_size, u64 alignment) {
    Profile_Zone();
    Memory_Arena arena = {
        .base_allocator = allocator,
        .cursor_node = 0,
        .chunk_size  = chunk_size,
        .alignment   = alignment
    };
    return arena;
}
inline Memory_Arena
make_memory_arena(Base_Allocator *allocator, u64 chunk_size) {
    Profile_Zone();
    return make_memory_arena(allocator, chunk_size, 8);
}
inline Memory_Arena
make_memory_arena(Base_Allocator *allocator) {
    Profile_Zone();
    return make_memory_arena(allocator, kilobytes(64), 8);
}

function Memory_Cursor_Node *
arena__new_node(DEBUG_MEMORY_PARAMETER
                Memory_Arena *arena, u64 min_size) {
    Profile_Zone();
    
    min_size = clamp_bot(min_size, arena->chunk_size);
    Data memory = base_allocate__inner(DEBUG_MEMORY_PASS  arena->base_allocator, min_size + sizeof(Memory_Cursor_Node));
    Memory_Cursor_Node *cursor_node = cast(Memory_Cursor_Node *)memory.data;
    cursor_node->cursor = make_memory_cursor(cursor_node + 1, memory.size - sizeof(Memory_Cursor_Node));
    sll_stack_push(arena->cursor_node, cursor_node);
    
    DEBUG_RECORD_BLOCK_ALLOCATION(cursor_node);
    return cursor_node;
}

#if 0
dont_compile;  // @deprecated
function Data
linalloc_push(DEBUG_MEMORY_PARAMETER
              Memory_Arena *arena, u64 size) {
    Profile_Zone();
    
    Data result = {};
    if (size > 0) {
        Memory_Cursor_Node *cursor_node = arena->cursor_node;
        if (cursor_node == 0) {
            cursor_node = arena__new_node(DEBUG_MEMORY_PASS  arena, size);
        }
        result = linalloc_push(DEBUG_MEMORY_PASS  &cursor_node->cursor, size);
        if (result.data == 0) {
            cursor_node = arena__new_node(DEBUG_MEMORY_PASS  arena, size);
            result = linalloc_push(DEBUG_MEMORY_PASS  &cursor_node->cursor, size);
        }
        
        // @note(tebtro): This might be zero size, if it's at the end of the cursor_node,
        //        but this is fine, because then the new cursor_node will be aligned.
        Data alignment_data = linalloc_align(&cursor_node->cursor, arena->alignment);
        result.size += alignment_data.size;
    }
    return result;
}
#endif

function Data
linalloc_push(DEBUG_MEMORY_PARAMETER
              Memory_Arena *arena, u64 size_unaligned, Arena_Push_Params params = {}) {
    Profile_Zone();
    
    Data result = {};
    if (size_unaligned > 0) {
        if (!params.alignment) {
            params.alignment = cast(u32) arena->alignment;
        }
        
        assert(params.alignment > 0 && params.alignment <= 128);
        assert(is_pow2(params.alignment));
        
        u64 size = 0;
        
        Memory_Cursor_Node *cursor_node = arena->cursor_node;
        if (cursor_node) {
            size = get_effective_size_for(arena, size_unaligned, params.alignment);
        }
        
        if (!cursor_node || (cursor_node->cursor.pos + size) > cursor_node->cursor.capacity) {
#if 1
            // @hack @todo Because the allocator is not aligning it.
            // @hack @todo Because the allocator is not aligning it.
            // @hack @todo Because the allocator is not aligning it.
            // @hack @todo Because the allocator is not aligning it.
            // @hack @todo Because the allocator is not aligning it.
            size = size_unaligned;
            u64 new_block_size = align_pow2(size + params.alignment, params.alignment);
            cursor_node = arena__new_node(DEBUG_MEMORY_PASS  arena, new_block_size);
            linalloc_align(DEBUG_MEMORY_PASS  &cursor_node->cursor, params.alignment);
#else
            size = size_unaligned;  // @note: The base will automatically be aligned, to the page alignment.
            cursor_node = arena__new_node(DEBUG_MEMORY_PASS  arena, size);
            assert(((u64)cursor_node->cursor.base & (params.alignment - 1)) == 0, "Base_Allocator is not aligning. Or the push alignment is bigger than that.");
#endif
        }
        assert(size >= size_unaligned);
        
        assert((cursor_node->cursor.pos + size) <= cursor_node->cursor.capacity);
        memory_index alignment_offset = get_alignment_offset(arena, params.alignment);
        umm offset_in_block = cursor_node->cursor.pos + alignment_offset;
        result.data = cursor_node->cursor.base + offset_in_block;
        result.size = size_unaligned;
        cursor_node->cursor.pos += size;
        
        // @note: This is just to gurantee that nobody passed in an alignment
        // on their first allocation that was _greater_ than the page alignment.
        assert(cursor_node->cursor.pos <= cursor_node->cursor.capacity);
        assert((result.data + result.size) <= (cursor_node->cursor.base + cursor_node->cursor.pos));
        
        if ((params.flags & ARENA_PUSH_no_clear_to_zero) == 0) {
            block_zero(result);
        }
        
        DEBUG_RECORD_ALLOCATION(cursor_node, GUID, size, size_unaligned, offset_in_block);
    }
    return result;
}
function void
linalloc_pop(DEBUG_MEMORY_PARAMETER
             Memory_Arena *arena, u64 size) {
    Profile_Zone();
    
    Base_Allocator *allocator = arena->base_allocator;
    Memory_Cursor_Node *cursor_node = arena->cursor_node;
    for (Memory_Cursor_Node *prev = 0;
         cursor_node != 0 && size != 0;
         cursor_node = prev) {
        prev = cursor_node->prev;
        if (size >= cursor_node->cursor.pos) {
            size -= cursor_node->cursor.pos;
            base_free(allocator, cursor_node);
        }
        else {
            linalloc_pop(DEBUG_MEMORY_PASS  &cursor_node->cursor, size);
            break;  // We don't want to set the cursor_node to prev, this node still has used memory and isn't freed.
        }
    }
    arena->cursor_node = cursor_node;
}
inline Data
linalloc_align(DEBUG_MEMORY_PARAMETER
               Memory_Arena *arena, u64 alignment, bool clear_to_zero = false) {
    Profile_Zone();
    
    arena->alignment = alignment;
    Data data = {};
    Memory_Cursor_Node *cursor_node = arena->cursor_node;
    if (cursor_node != 0) {
        data = linalloc_align(DEBUG_MEMORY_PASS  &cursor_node->cursor, arena->alignment);
    }
    
    if (clear_to_zero) {
        block_zero(data);
    }
    return data;
}

inline Temporary_Memory_Arena
linalloc_begin_temporary(Memory_Arena *arena) {
    Profile_Zone();
    
    Memory_Cursor_Node *cursor_node = arena->cursor_node;
    Temporary_Memory_Arena temp = {
        .arena = arena,
        .cursor_node = cursor_node,
        .pos = (cursor_node == 0) ? 0 : cursor_node->cursor.pos
    };
    return temp;
}
function void
linalloc_end_temporary(Temporary_Memory_Arena temp) {
    Profile_Zone();
    
    Base_Allocator *allocator = temp.arena->base_allocator;
    Memory_Cursor_Node *cursor_node = temp.arena->cursor_node;
    for (Memory_Cursor_Node *prev = 0;
         cursor_node != temp.cursor_node && cursor_node != 0;
         cursor_node = prev) {
        prev = cursor_node->prev;
        base_free(allocator, cursor_node);
    }
    temp.arena->cursor_node = cursor_node;
    if (cursor_node != 0) {
        if (temp.pos > 0) {
            cursor_node->cursor.pos = temp.pos;
        }
        else {
            temp.arena->cursor_node = cursor_node->prev;
            base_free(allocator, cursor_node);
        }
    }
}
inline void
linalloc_keep_temporary(Temporary_Memory_Arena *temp) {
    Profile_Zone();
    *temp = linalloc_begin_temporary(temp->arena);
}

inline void
linalloc_clear(Memory_Arena *arena) {
    Profile_Zone();
    Temporary_Memory_Arena temp = {
        .arena = arena,
        .cursor_node = 0,
        .pos = 0
    };
    linalloc_end_temporary(temp);
}


//
// @note: push functions/macros
//

inline void *
linalloc_wrap_uninitialized(Data data) {
    return data.data;
}
inline void *
linalloc_wrap_zero(Data data) {
    block_zero(data.data, data.size);
    return data.data;
}
inline void *
linalloc_wrap_copy(Data data, u64 size, const void *src) {
    block_copy(data.data, src, clamp_top(data.size, size));
    return data.data;
}


inline void *
push_size_(DEBUG_MEMORY_PARAMETER
           Memory_Cursor *cursor, u64 size) {
    Data result = linalloc_push(DEBUG_MEMORY_PASS  cursor, size);
    return linalloc_wrap_uninitialized(result);
}
inline void *
push_size_copy_(DEBUG_MEMORY_PARAMETER
                Memory_Cursor *cursor, u64 size, const void *src) {
    return linalloc_wrap_copy(linalloc_push(DEBUG_MEMORY_PASS  cursor, size), size, src);
}
inline void
pop_size_(DEBUG_MEMORY_PARAMETER
          Memory_Cursor *cursor, u64 size) {
    linalloc_pop(DEBUG_MEMORY_PASS  cursor, size);
}

inline void *
push_size_(DEBUG_MEMORY_PARAMETER
           Memory_Arena *arena, u64 size, Arena_Push_Params params = {}) {
    return linalloc_push(DEBUG_MEMORY_PASS  arena, size, params).data;
}
inline void *
push_size_copy_(DEBUG_MEMORY_PARAMETER
                Memory_Arena *arena, u64 size, const void *src, u32 alignment = 0) {
    return linalloc_wrap_copy(linalloc_push(DEBUG_MEMORY_PASS  arena, size, align_no_clear(alignment)), size, src);
}
inline void
pop_size_(DEBUG_MEMORY_PARAMETER
          Memory_Arena *arena, u64 size) {
    linalloc_pop(DEBUG_MEMORY_PASS  arena, size);
}


// Sets the default alignment on the arena and pushes an initial alignment.
#define push_align(...)  (linalloc_align(DEBUG_MEMORY_NAME("push_align")  __VA_ARGS__))

#define push_copy(...)  push_size_copy(__VA_ARGS__)

#define push_size(...)       (push_size_(     DEBUG_MEMORY_NAME("push_size")       __VA_ARGS__))
#define push_size_copy(...)  (push_size_copy_(DEBUG_MEMORY_NAME("push_size_copy")  __VA_ARGS__))
#define pop_size(...)        (pop_size_(      DEBUG_MEMORY_NAME("pop_size")        __VA_ARGS__))

// a = arena, T = Type, src = source
#define push_struct(a, T, ...)            ((T *)push_size_(     DEBUG_MEMORY_NAME("push_struct")       (a), sizeof(T),        ## __VA_ARGS__))
#define push_struct_copy(a, T, src, ...)  ((T *)push_size_copy_(DEBUG_MEMORY_NAME("push_struct_copy")  (a), sizeof(T), (src), ## __VA_ARGS__))
#define pop_struct(a, T)                  (     pop_size_(      DEBUG_MEMORY_NAME("pop_struct")        (a), sizeof(T)                       ))

// a = arena, T = Type, c = count, src = source
#define push_array(a, T, c, ...)            ((T *)push_size_(     DEBUG_MEMORY_NAME("push_array")       (a), (c)*sizeof(T),        ## __VA_ARGS__))
#define push_array_copy(a, T, c, src, ...)  ((T *)push_size_copy_(DEBUG_MEMORY_NAME("push_array_copy")  (a), (c)*sizeof(T), (src), ## __VA_ARGS__))
#define pop_array(a, T, c)                  (     pop_size_(      DEBUG_MEMORY_NAME("pop_array")        (a), (c)*sizeof(T)                       ))

#define push_buffer(...)              (push_buffer_(            DEBUG_MEMORY_NAME("push_buffer")              __VA_ARGS__))
#define push_buffer_copy(...)         (push_buffer_copy_(       DEBUG_MEMORY_NAME("push_buffer_copy")         __VA_ARGS__))
#define push_string_copy(...)         (push_string_copy_(       DEBUG_MEMORY_NAME("push_string_copy")         __VA_ARGS__))
#define push_cstring_copy(...)        (push_cstring_copy_(      DEBUG_MEMORY_NAME("push_cstring_copy")        __VA_ARGS__))
#define push_and_null_terminate(...)  (push_and_null_terminate_(DEBUG_MEMORY_NAME("push_and_null_terminate")  __VA_ARGS__))


inline Buffer
push_buffer_(DEBUG_MEMORY_PARAMETER
             Memory_Arena *arena, u64 size, Arena_Push_Params params = {}) {
    Buffer result;
    result.count = size;
    result.data = cast(u8 *) push_size(arena, size, params);
    return result;
}
inline Buffer
push_buffer_copy_(DEBUG_MEMORY_PARAMETER
                  Memory_Arena *arena, Buffer source, u32 alignment = 0) {
    Buffer result;
    result.count = source.count;
    result.data = cast(u8 *) push_size_copy_(DEBUG_MEMORY_PASS  arena, result.count, source.data, alignment);
    return result;
}

inline String
push_string_copy_(DEBUG_MEMORY_PARAMETER
                  Memory_Arena *arena, const char *source, u32 alignment = 0) {
    String result;
    result.count = string_length(source);
    result.data = cast(u8 *) push_size_copy_(DEBUG_MEMORY_PASS  arena, result.count, source, alignment);
    return result;
}
inline String
push_string_copy_(DEBUG_MEMORY_PARAMETER
                  Memory_Arena *arena, String source, u32 alignment = 0) {
    String result;
    result.count = source.count;
    result.data = cast(u8 *) push_size_copy_(DEBUG_MEMORY_PASS  arena, result.count, source.data, alignment);
    return result;
}

inline char *
push_and_null_terminate_(DEBUG_MEMORY_PARAMETER
                         Memory_Arena *arena, u32 length, const char *source, u32 alignment = 0) {
    Profile_Zone();
    char *dest = cast(char *)push_size_(DEBUG_MEMORY_PASS  arena, length + 1, align_no_clear(alignment));
    memcpy(dest, source, length);
    dest[length] = 0;
    return dest;
}

inline char *
push_cstring_copy_(DEBUG_MEMORY_PARAMETER
                   Memory_Arena *arena, const char *source, u32 alignment = 0) {
    return push_and_null_terminate_(DEBUG_MEMORY_PASS  arena, string_length(source), source);
}
inline char *
push_cstring_copy_(DEBUG_MEMORY_PARAMETER
                   Memory_Arena *arena, String source, u32 alignment = 0) {
    return push_and_null_terminate_(DEBUG_MEMORY_PASS  arena, cast(u32)source.count, cast(const char *)source.data);
}


//
// @note: Temporary_Memory
//

inline Temporary_Memory
begin_temporary(Memory_Cursor *cursor) {
    Profile_Zone();
    Temporary_Memory temp = { .kind = LINEAR_ALLOCATOR_cursor };
    temp.cursor = linalloc_begin_temporary(cursor);
    return temp;
}
inline Temporary_Memory
begin_temporary(Memory_Arena *arena) {
    Profile_Zone();
    Temporary_Memory temp = { .kind = LINEAR_ALLOCATOR_arena };
    temp.arena = linalloc_begin_temporary(arena);
    return temp;
}
inline void
end_temporary(Temporary_Memory temp) {
    Profile_Zone();
    switch (temp.kind) {
        case LINEAR_ALLOCATOR_cursor: {
            linalloc_end_temporary(temp.cursor);
        } break;
        
        case LINEAR_ALLOCATOR_arena: {
            linalloc_end_temporary(temp.arena);
        } break;
    }
}

inline void
keep_temporary(Temporary_Memory *temp) {
    Profile_Zone();
    switch (temp->kind) {
        case LINEAR_ALLOCATOR_cursor: {
            linalloc_keep_temporary(&temp->cursor);
        } break;
        
        case LINEAR_ALLOCATOR_arena: {
            linalloc_keep_temporary(&temp->arena);
        } break;
    }
}


inline
Temporary_Memory_Block::Temporary_Memory_Block(Temporary_Memory t) {
    this->temp = t;
}
inline
Temporary_Memory_Block::Temporary_Memory_Block(Memory_Arena *arena) {
    this->temp = begin_temporary(arena);
}
inline
Temporary_Memory_Block::~Temporary_Memory_Block() {
    end_temporary(this->temp);
}
inline void
Temporary_Memory_Block::restore() {
    end_temporary(this->temp);
}
inline void
Temporary_Memory_Block::keep_memory() {
    keep_temporary(&this->temp);
}
inline void
keep_temporary(Temporary_Memory_Block *temp) {
    temp->keep_memory();
}


//
// @note: Data
//

inline Data
push_data(Memory_Arena *arena, u64 size) {
    Data result = {};
    result.data = push_array(arena, u8, size);
    result.size = size;
    return result;
}

inline Data
push_data_copy(Memory_Arena *arena, Data data) {
    Data result = {};
    result.data = push_array_copy(arena, u8, data.size, data.data);
    result.size = data.size;
    return result;
}

inline b32
data_match(Data a, Data b) {
    if (a.size != b.size)  return false;
    return block_match(a.data, b.data, a.size);
}



//
// @note: Tests
//
#if 0
#include "iml_test_suit.h"

global_unit("iml_memory/memory") {
    unit("Memory_Arena") {
        test("@todo") {};  // @todo
    };
};
#endif



//
//~
#ifdef IML_NAMESPACE
#if 0
{
#endif
}
#endif

#endif//IML_MEMORY_H
