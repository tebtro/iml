#if !defined(IML_LOCAL_ARRAY_H)
#define IML_LOCAL_ARRAY_H

#if defined(IML_NAMESPACE)
namespace IML_NAMESPACE {
#  if 0
}
#  endif
#endif
//
//~


#include "iml_types.h"


//
// @note: Local array
//
template <typename T>
struct Local_Array {
    u64 count = 0;
    u64 allocated = 0;
    T *data;
    
    void reset(void) {
        count = 0;
    }
    
    T *add_uninitialized(void) {
        assert(count < allocated);
        auto result = data + count;
        count++;
        return result;
    }
    
    inline T *add(void) {
        T *result = add_uninitialized();
        memset(result, 0, sizeof(T));
        return result;
    }
    
    inline void add(T item) {
        assert(count < allocated);
        data[count] = item;
        count++;
    }
    
    T &operator[](const u64 index) {
        assert(index < count);
        return data[index];
    }
    
    const T &operator[](const u64 index) const {
        assert(index < count);
        return data[index];
    }
    
    T *begin() {
        return data;
    }
    T *end() {
        return data + count;
    };
    
    const T *begin() const {
        return data;
    }
    const T *end() const {
        return data + count;
    };
};


#define make_local_array(T, size)  \
Local_Array<T> {  \
.count = 0,  \
.allocated = size,  \
.data = cast(T *) alloca(size * sizeof(T)),  \
}



//
// @note: Tests
//
#include "iml_test_suit.h"

global_unit("Local_Array") {
    typedef Local_Array<int> Int4_Local_Array;
    static Int4_Local_Array *array;
    static const int test_data_count = 4;
    static int test_data[test_data_count] = { 123, 555, 666, 911 };
    
    array = push_struct(&__ts_config__->arena, Int4_Local_Array);
    *array = make_local_array(int, 32);
    
    before_all() {
        check(array->count == 0);
    };
    
    after_each() {
        array->reset();
    };
    
    
    test("reset()") {
        array->add();
        check(array->count > 0);
        array->reset();
        check(array->count == 0);
    };
    
    test("add_uninitialized()") {
        int *i0 = array->add_uninitialized();
        check(array->count == 1);
        check(i0);
        
        int *i1 = array->add_uninitialized();
        check(array->count == 2);
        check(i1);
        check(i0 != i1);
        
        array->reset();
        check(array->count == 0);
        
        i0 = array->add_uninitialized();
        check(array->count == 1);
        check(i0);
    };
    
    test("add()") {
        int *i0 = array->add();
        check(array->count == 1);
        check(i0);
        check(*i0 == 0);
        *i0 = test_data[0];
        
        int *i1 = array->add();
        check(array->count == 2);
        check(i1);
        check(*i1 == 0);
        check(i0 != i1);
        *i1 = test_data[1];
        
        array->reset();
        check(array->count == 0);
        
        i0 = array->add();
        check(array->count == 1);
        check(i0);
        check(*i0 == 0);
    };
    
    test("add(item)") {
        array->add(test_data[0]);
        check(array->count == 1);
        
        array->add(test_data[1]);
        check(array->count == 2);
        
        check(array->data[0] == test_data[0]);
        check(array->data[1] == test_data[1]);
        
        array->reset();
        check(array->count == 0);
        
        array->add(test_data[0]);
        check(array->count == 1);
        check(array->data[0] == test_data[0]);
    };
    
    test("operator[]") {
        array->add(test_data[0]);
        check((*array)[0] == test_data[0]);
        check((*array)[0] == array->data[0]);
        
        array->add(test_data[1]);
        check(&(*array)[1] == cast(int *)array->data + 1);
        check((*array)[1] == array->data[1]);
        check((*((const Int4_Local_Array *)array))[1] == test_data[1]);
    };
    
    group("iterators") {
        static int i;
        
        before_each() {
            i = 0;
            For (test_data)  array->add(it);
            check(array->count == test_data_count);
        };
        
        
        test("for (&)") {
            for (int &it : *array) {
                check(it == test_data[i]);
                it += 2;
                check(array->data[i] == (test_data[i] + 2));
                i++;
            }
        };
        
        test("for (const &)") {
            for (const int &it : *array) {
                check(it == test_data[i]);
                i++;
            }
        };
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

#endif  // IML_LOCAL_ARRAY_H
