#if !defined(IML_STRING_LIST_H)
#define IML_STRING_LIST_H

#if defined(IML_NAMESPACE)
namespace IML_NAMESPACE {
#  if 0
}
#  endif
#endif
//
//~



#include "iml_memory/memory.h"


struct String_Node {
    String_Node *next;
    String string;
};

struct String_List {
    String_Node *first;
    String_Node *last;
    s32 node_count;
    
    u64 total_size;
};


function void
string_list_push(Memory_Arena *arena, String_List *list, String string) {
    String_Node *node = push_struct(arena, String_Node);
    sll_queue_push(list->first, list->last, node);
    node->string = string;
    list->node_count += 1;
    list->total_size += string.count;
}

function void
string_list_push(Memory_Arena *arena, String_List *list, const char *string) {
    string_list_push(arena, list, make_string(string));
}

function String
string_list_flatten(Memory_Arena *arena, String_List list) {
    String string = {};
    string.data = push_array(arena, u8, list.total_size + 1);
    for (String_Node *node = list.first; node != 0; node = node->next) {
        block_copy_dynamic_array(string.data + string.count, node->string.data, node->string.count);
        string.count += node->string.count;
    }
    string.data[string.count] = '\0';
    return string;
}



//
//~
#ifdef IML_NAMESPACE
#if 0
{
#endif
}
#endif

#endif//IML_STRING_LIST_H
