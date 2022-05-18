#if !defined(IML_BIG_ARRAY_H)
#define IML_BIG_ARRAY_H

#if defined(IML_NAMESPACE)
namespace IML_NAMESPACE {
#  if 0
}
#  endif
#endif
//
//~


//
// @note Big_Array
//
// Useful when you need an array, where you know the real maximum size it can ever have.
// But this array just reserves the whole size you might need,
// and only commits what is actually used.
// Furthermore there is no memory copying when resizing,
// because we are only commiting more and we already have the address space.
// And also because of that all pointers into the array stay valid after resizing.
//
template <typename T>
struct Big_Array {
    T *data = nullptr; // mapped_address
    T *maximum_address_reserved = nullptr; // @note One past last, so the address pointed to is _not_ commited.
    u64 count = 0;
    u64 allocated = 0; // commited
    u64 amount_to_grow = 0;
    
    b32 init(u64 reserve_amount, u64 _amount_to_grow) {
        assert(_amount_to_grow <= reserve_amount);
        amount_to_grow = _amount_to_grow;
        
        // @note Reserve
        LPVOID base_address = 0;
        u64 size = reserve_amount * sizeof(T);
        
        void *memory = VirtualAlloc(base_address, cast(size_t) size, MEM_RESERVE, PAGE_READWRITE);
        assert(memory);
        if (!memory)  return false;
        
        data = cast(T *) memory;
        maximum_address_reserved = (cast(T *) memory) + reserve_amount;
        
        count = 0;
        
        
        b32 success = expand();
        return success;
    }
    
    b32 expand() {
        LPVOID base_address = cast(LPVOID) (data + allocated);
        u64 size = amount_to_grow * sizeof(T);
        
        {
            T *maximum_address = (cast(T *)base_address) + amount_to_grow;
            assert(maximum_address <= maximum_address_reserved);
            if (maximum_address > maximum_address_reserved)  return false;
        }
        
        void *new_memory = VirtualAlloc(base_address, cast(size_t) size, MEM_COMMIT, PAGE_READWRITE);
        assert(new_memory);
        if (!new_memory)  return false;
        
        allocated += amount_to_grow;
        
        return true;
    }
    
    void reset() {
        count = 0;
    }
    
    void free() {
        LPVOID base_address = cast(LPVOID) data;
        // @note Size must be zero when doing a MEM_RELEASE,
        // the whole address space reserved at the base_address is released.
        b32 success = VirtualFree(base_address, 0, MEM_RELEASE);
        assert(success);
        
        data = nullptr;
    }
    
    T *add() {
        if ((count + 1) > allocated) {
            b32 success = expand();
            assert(success);
            if (!success)  return nullptr;
        }
        
        T *result = data + count;
        count += 1;
        
        return result;
    }
    
    force_inline b32 add(T element) {
        T *result = add();
        if (!result)  return false;
        *result = element;
        return true;
    }
    
    T &operator[](const size_t index) {
        assert(index >= 0 && index < count);
        return data[index];
    }
    
    const T &operator[](const size_t index) const {
        assert(index >= 0 && index < count);
        return data[index];
    }
    
    T *begin() const {
        return data;
    }
    
    const T *end() const {
        return data + count;
    }
};


//
//~
#ifdef IML_NAMESPACE
#if 0
{
#endif
}
#endif

#endif // IML_BIG_ARRAY_H
