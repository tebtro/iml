#if !defined(IML_ARRAY_H)
#define IML_ARRAY_H

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
#include "iml_common_array_system.h"


//
// @note: Dynamic array
//
template <typename T>
struct Array {
    Base_Allocator *allocator = &global_default_array_allocator;
    
    T *data = null;
    u64 count = 0;
    u64 allocated = 0;
    
    
    inline void init(Base_Allocator *allocator) {
        this->allocator = allocator;
    }
    
    inline void free(void) {
        if (data) {
            base_free(allocator, data);
            data = null;
        }
        
        count = 0;
        allocated = 0;
    }
    
    inline void reset(void) {
        count = 0;
    }
    
    
    void reserve(u64 size) {
        Profile_Zone();
        
        if (size <= allocated)  return;
        
        s64 new_size = maximum(allocated * 2, size);
        new_size = maximum(new_size, 8);
        
        auto old_bytes = allocated * sizeof(T);
        auto new_bytes = new_size  * sizeof(T);
        
        void *new_data = base_allocate(allocator, new_bytes).data;
        assert(new_data);
        
        if (data) {
            memcpy(new_data, data, old_bytes);
            base_free(allocator, data);
        }
        
        allocated = new_size;
        data = cast(T *) new_data;
    }
    
    void resize(u64 size) {
        reserve(size);
        if (size > count) {
            memset(data + count, 0, (size - count) * sizeof(T));
        }
        count = size;
    }
    
    void resize(u64 size, T value) {
        reserve(size);
        auto old_count = count;
        count = size;
        for (u64 i = old_count; i < count; i++) {
            (*this)[i] = value;
        }
    }
    
    inline T *add_uninitialized(void) {
        Profile_Zone();
        reserve(count + 1);
        T *result = data + count;
        count++;
        return result;
    }
    
    inline T *add(void) {
        T *result = add_uninitialized();
        memset(result, 0, sizeof(T));
        return result;
    }
    
    inline void add(T item) {
        Profile_Zone();
        reserve(count + 1);
        data[count] = item;
        count++;
    }
    
    inline bool add_unique(T item) {
        if (find(item))  return false;
        add(item);
        return true;
    }
    
    void insert_at(u64 index, T item) {
        assert(index <= count);
        reserve(count + 1);
        
        memmove(data + index + 1,
                data + index,
                (count - index) * sizeof(T));
        
        data[index] = item;
        count++;
    }
    
    bool find(T item, u64 *index_return = null) const {
        for (u64 i = 0; i < count; i++) {
            if (data[i] == item) {
                if (index_return)  *index_return = i;
                return true;
            }
        }
        
        if (index_return)  *index_return = -1;
        return false;
    }
    
    bool unordered_remove(T item) {
        for (u64 i = 0; i < count; i++) {
            if (data[i] == item) {
                data[i] = data[count - 1];
                count -= 1;
                return true;
            }
        }
        return false;
    }
    
    T unordered_remove_nth(u64 index) {
        assert(count > 0);
        T result = (*this)[index];
        data[index] = data[count - 1];
        count -= 1;
        return result;
    }
    
    bool ordered_remove(T item) {
        for (u64 i = 0; i < count; i++) {
            if (data[i] == item) {
                memmove(data + i,
                        data + i + 1,
                        (count - i - 1) * sizeof(T));
                count -= 1;
                return true;
            }
        }
        return false;
    }
    
    T ordered_remove_nth(u64 index) {
        assert(count > 0);
        T item = (*this)[index];
        memmove(data + index,
                data + index + 1,
                (count - index - 1) * sizeof(T));
        
        count -= 1;
        return item;
    }
    
    T pop(void) {
        assert(count > 0);
        T result = data[count - 1];
        count -= 1;
        return result;
    }
    
    T *peek(void) const {
        assert(count > 0);
        T *result = data + count - 1;
        return result;
    }
    
    T &operator[](const u64 index) {
        assert(index < count, "index = %, count = %", index, count);
        return data[index];
    }
    
    const T &operator[](const u64 index) const {
        assert(index < count, "index = %, count = %", index, count);
        return data[index];
    }
    
    T *begin(void) const {
        return data;
    }
    
    T *end(void) const {
        return data + count;
    }
};


//
// @note: Foreach macros
//
#define Array_Foreach(l, x) \
for (u64 __i = 0; (__i < (l)->count) ? ((x) = (l)->data[__i]), true : false; __i++) \

#define Array_Foreach_Pointer(l, x) \
for (u64 __i = 0; (__i < (l)->count) ? ((x) = &(l)->data[__i]), true : false; __i++) \

#define Array_Foreach_Remove_Current_Element(_l) { \
(_l)->data[__i] = (_l)->data[(_l)->count - 1];  \
(_l)->count--; \
__i--; \
}

#define Array_Foreach_Remove_Current_Element_Keeping_Order(_l) { \
(_l)->remove_nth_element_while_keeping_order(__i); \
__i--; \
}


//
// @note: Array_View
//
//~ Array view
template <typename T>
struct Array_View {
    T *data = null;
    u64 count = 0;
    
    T &operator[](const u64 index) {
        assert(index < count);
        return data[index];
    }
    
    const T &operator[](const u64 index) const {
        assert(index < count);
        return data[index];
    }
    
    T *begin() const {
        return data;
    }
    
    const T *end() const {
        return data + count;
    }
};

template <typename T>
Array_View<T> make_array_view(T *data, u64 count) {
    Array_View<T> view = { data, count };
    return view;
}

template <typename T>
Array_View<T> make_array_view(Array<T> *array) {
    return make_array_view<T>(array->data, array->count);
}


//~ Const array view
template <typename T>
struct Const_Array_View {
    T const *data = null;
    u64 count  = 0;
    
    T &operator[](const u64 index) {
        assert(index < count);
        return data[index];
    }
    
    const T &operator[](const u64 index) const {
        assert(index < count);
        return data[index];
    }
    
    T *begin() const {
        return data;
    }
    
    const T *end() const {
        return data + count;
    }
};

template <typename T>
Const_Array_View<T> make_const_array_view(T const *data, const u64 count) {
    Const_Array_View<T> view = { data, count };
    return view;
}

template <typename T>
Const_Array_View<T> make_const_array_view(const Array<T> *const array) {
    return make_const_array_view<T>(array->data, array->count);
}



//
// @note: Tests
//
#include "iml_test_suit.h"

global_unit("Array") {
    typedef Array<int>  Int_Array;
    static Int_Array *array;
    static const int test_data_count = 4;
    static int test_data[test_data_count] = { 123, 555, 666, 911 };
    
    before_all() {
        array = push_struct(&__ts_config__->arena, Int_Array);
        *array = {};
        
        check(array->allocator);
        check(array->data == null);
        check(array->count == 0);
        check(array->allocated == 0);
    };
    
    after_each() {
        array->free();
    };
    
    
    test("free()") {
        array->add();
        array->free();
        
        check(array->data == null);
        check(array->allocated == 0);
        check(array->count == 0);
    };
    
    test("reset()") {
        array->add();
        check(array->count > 0);
        array->reset();
        check(array->count == 0);
    };
    
    test("reserve(size)") {
        array->reserve(0);
        check(array->data == null);
        check(array->allocated == 0);
        
        array->reserve(2);
        check(array->data != null);
        check(array->allocated >= 2);
        
        array->reset();
        array->add(test_data[0]);
        auto old_allocated = array->allocated;
        array->reserve(old_allocated + 1);
        check(array->allocated >= (old_allocated + 1));
        check((*array)[0] == test_data[0]);
        
        // Test copying of data when resizing
        array->free();
        array->add(0);
        u64 count = array->allocated * 2 * 2 + 4;
        for (int i = 1; i < count; ++i) {
            array->add(i);
        }
        for (int i = 0; i < count; ++i) {
            check((*array)[i] == i);
        }
    };
    
    test("resize(size)") {
        int size = 2;
        array->resize(size);
        check(array->allocated >= size);
        check(array->count == size);
        
        size += 2;
        array->resize(size);
        check((*array)[size - 1] == 0);
        
        size -= 2;
        array->resize(size);
        check(array->count == size);
    };
    
    test("resize(size, value)") {
        int value = 1;
        
        int size = 2;
        array->resize(size, value);
        check(array->allocated >= size);
        check(array->count == size);
        
        size += 2;
        array->resize(size, value);
        check((*array)[size - 1] == value);
        
        size -= 2;
        array->resize(size, value);
        check(array->count == size);
    };
    
    test("add_uninitialized()") {
        int *i0 = array->add_uninitialized();
        check(array->count == 1);
        check(i0);
        
        check(array->allocated >= 2); // @note: Actually the first pointer could already be not valid anymore, if we had to resize on the next add.
        
        int *i1 = array->add_uninitialized();
        check(i1);
        check(i0 != i1);
    };
    
    test("add()") {
        int *i0 = array->add();
        check(array->count == 1);
        check(i0);
        check(*i0 == 0);
        
        check(array->allocated >= 2); // @note: Actually the first pointer could already be not valid anymore, if we had to resize on the next add.
        
        int *i1 = array->add();
        check(i1);
        check(i0 != i1);
    };
    
    test("add(item)") {
        array->add(test_data[0]);
        check(array->count == 1);
        check(array->data[0] == test_data[0]);
        
        array->count = array->allocated; // Force resize on next add.
        array->add(test_data[1]);
        check(array->data[0] == test_data[0]);
    };
    
    test("add_unique(item)") {
        array->add(test_data[0]);
        array->add(test_data[1]);
        
        check(array->add_unique(test_data[2]) == true);
        check(array->add_unique(test_data[1]) == false);
        
        check(array->data[1] == test_data[1]);
        check(array->data[2] == test_data[2]);
    };
    
    test("insert_at(index, item)") {
        array->add(test_data[0]);
        array->add(test_data[1]);
        check(array->count == 2);
        
        array->insert_at(1, test_data[2]);
        check(array->count == 3);
        
        check(array->data[0] == test_data[0]);
        check(array->data[1] == test_data[2]);
        check(array->data[2] == test_data[1]);
        
        // Insert at count
        check(array->count == 3);
        array->insert_at(3, test_data[3]);
        check(array->count == 4);
        check(array->data[2] == test_data[1]);
        check(array->data[3] == test_data[3]);
    };
    
    test("find(item, index_return)") {
        For (test_data)  array->add(it);
        check(array->count == test_data_count);
        
        check(array->find(test_data[3]) == true);
        check(array->find(753197) == false, "Should not be found!");
        
        size_t index;
        array->find(test_data[3], &index);
        check(index == 3);
        
        array->find(753197, &index);
        check(index == -1);
    };
    
    test("unordered_remove(item)") {
        For (test_data)  array->add(it);
        check(array->count == 4);
        
        check(array->unordered_remove(test_data[2]) == true);
        check(array->count == 3);
        check((*array)[0] == test_data[0]);
        check((*array)[1] == test_data[1]);
        check((*array)[2] == test_data[3]);
        
        check(array->unordered_remove(753197) == false);
        check(array->count == 3);
        check((*array)[0] == test_data[0]);
        check((*array)[1] == test_data[1]);
        check((*array)[2] == test_data[3]);
        
        // Test remove item at the end.
        check(array->unordered_remove(test_data[3]));
        check(array->count == 2);
        check((*array)[0] == test_data[0]);
        check((*array)[1] == test_data[1]);
        
        check(array->unordered_remove(test_data[1]));
        check(array->count == 1);
        check((*array)[0] == test_data[0]);
        
        // Test remove last item (count == 1).
        check(array->count == 1);
        check(array->unordered_remove(test_data[0]));
        check(array->count == 0);
        
        // Test remove from empty array (count == 0).
        check(array->count == 0);
        check(array->unordered_remove(test_data[0]) == false);
    };
    
    test("unordered_remove_nth(index)") {
        For (test_data)  array->add(it);
        check(array->count == 4);
        
        int a = array->unordered_remove_nth(2);
        check(a == test_data[2]);
        check(array->count == 3);
        check((*array)[0] == test_data[0]);
        check((*array)[1] == test_data[1]);
        check((*array)[2] == test_data[3]);
        
        // Test remove last.
        int b = array->unordered_remove_nth(2);
        check(b == test_data[3]);
        check(array->count == 2);
        check((*array)[0] == test_data[0]);
        check((*array)[1] == test_data[1]);
        
        // Test remove first.
        int c = array->unordered_remove_nth(0);
        check(c == test_data[0]);
        check(array->count == 1);
        check((*array)[0] == test_data[1]);
        
        // Test if array has count = 1.
        int d = array->unordered_remove_nth(0);
        check(d == test_data[1]);
        check(array->count == 0);
        
        // Remove from array with count = 0 will assert.
    };
    
    test("ordered_remove(item)") {
        For (test_data)  array->add(it);
        check(array->count == 4);
        
        check(array->ordered_remove(test_data[2]) == true);
        check(array->count == 3);
        check((*array)[0] == test_data[0]);
        check((*array)[1] == test_data[1]);
        check((*array)[2] == test_data[3]);
        
        check(array->ordered_remove(753197) == false);
        check(array->count == 3);
        check((*array)[0] == test_data[0]);
        check((*array)[1] == test_data[1]);
        check((*array)[2] == test_data[3]);
        
        // Test remove last.
        check(array->ordered_remove(test_data[3]) == true);
        check(array->count == 2);
        check((*array)[0] == test_data[0]);
        check((*array)[1] == test_data[1]);
        
        // Test remove first.
        check(array->ordered_remove(test_data[0]) == true);
        check(array->count == 1);
        check((*array)[0] == test_data[1]);
        
        // Test if array has count = 1.
        check(array->ordered_remove(test_data[1]) == true);
        check(array->count == 0);
    };
    
    test("ordered_remove_nth(index)") {
        For (test_data)  array->add(it);
        check(array->count == 4);
        
        int a = array->ordered_remove_nth(1);
        check(a == test_data[1]);
        check(array->count == 3);
        check((*array)[0] == test_data[0]);
        check((*array)[1] == test_data[2]);
        check((*array)[2] == test_data[3]);
        
        // Test remove last.
        int b = array->ordered_remove_nth(2);
        check(b == test_data[3]);
        check(array->count == 2);
        check((*array)[0] == test_data[0]);
        check((*array)[1] == test_data[2]);
        
        // Test remove first.
        int c = array->ordered_remove_nth(0);
        check(c == test_data[0]);
        check(array->count == 1);
        check((*array)[0] == test_data[2]);
        
        // Test if array has count = 1.
        int d = array->ordered_remove_nth(0);
        check(d == test_data[2]);
        check(array->count == 0);
    };
    
    test("pop()") {
        For (test_data)  array->add(it);
        check(array->count == 4);
        
        int a = array->pop();
        check(a == test_data[3]);
        check(array->count == 3);
        
        int b = array->pop();
        check(b == test_data[2]);
        check(array->count == 2);
    };
    
    test("peek()") {
        For (test_data)  array->add(it);
        check(array->count == 4);
        
        int *a = array->peek();
        check(a);
        check(*a == test_data[3]);
        check(array->count == 4);
        
        int *b = array->peek();
        check(b);
        check(*b == test_data[3]);
        check(array->count == 4);
    };
    
    test("operator[]") {
        array->add(test_data[0]);
        check((*array)[0] == test_data[0]);
        check((*array)[0] == array->data[0]);
        
        array->add(test_data[1]);
        check(&(*array)[1] == cast(int *)array->data + 1);
        check((*array)[1] == array->data[1]);
        check((*((const Int_Array *)array))[1] == test_data[1]);
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

#endif // IML_ARRAY_H
