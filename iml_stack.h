#if !defined(IML_STACK_H)
#define IML_STACK_H


#include "iml_array.h"


//
// @note: Stack
//
template <typename T>
struct Stack {
    Array <T> items;
    
    
    inline void init(Base_Allocator *allocator) {
        this->items.init(allocator);
    }
    
    inline void free(void) {
        items.free();
    }
    
    inline void reset(void) {
        items.reset();
    }
    
    
    inline void push(T it) {
        items.add(it);
    }
    
    inline T pop(void) {
        return items.pop();
    }
    
    inline T *get_current(void) {
        return items.peek();
    }
};


#endif // IML_STACK_H
