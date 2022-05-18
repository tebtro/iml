#if !defined(IML_SLICE_H)
#define IML_SLICE_H

#if defined(IML_NAMESPACE)
namespace IML_NAMESPACE {
#  if 0
}
#  endif
#endif
//
//~


#include "iml_types.h"
#include "iml_array.h"


template<typename T>
struct Slice {
    T *data;
    u64 count;
    
    T &operator[](const u64 index) {
        assert(index < count, "index = %, count = %", index, count);
        return data[index];
    }
    const T &operator[](const u64 index) const {
        assert(index < count, "index = %, count = %", index, count);
        return data[index];
    }
    
    T *begin(void) {
        return data;
    }
    T *end(void) {
        return data + count;
    }
    
    const T *begin(void) const {
        return data;
    }
    const T *end(void) const {
        return data + count;
    }
};


template<typename T>
Slice<T> make_slice(T *data, u64 count) {
    Slice<T> slice;
    slice.data = data;
    slice.count = count;
    return slice;
}
template<typename T>
Slice<T> make_slice(const T *data, u64 count) {
    Slice<T> slice;
    slice.data = (T *) data;
    slice.count = count;
    return slice;
}


//
//~
#ifdef IML_NAMESPACE
#if 0
{
#endif
}
#endif

#endif // IML_SLICE_H
