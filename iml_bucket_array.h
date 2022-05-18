#if !defined(IML_BUCKET_ARRAY_H)
#define IML_BUCKET_ARRAY_H

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
#include "iml_bits.h"


//
// @note: Bucket array
//
template <typename T, u64 BUCKET_SIZE = 256>
struct Bucket_Array;

template <typename T, u64 BUCKET_SIZE>
struct Bucket_Array_Iterator {
    Bucket_Array <T, BUCKET_SIZE> *array;
    u64 index;
    
    Bucket_Array_Iterator(Bucket_Array<T, BUCKET_SIZE> *_array, u64 _index) : array(_array), index(_index) {};
    
    bool operator != (Bucket_Array_Iterator <T, BUCKET_SIZE> &other) const {
        return (index != other.index);
    }
    
    Bucket_Array_Iterator &operator ++() {
        index += 1;
        return *this;
    }
    
    T &operator *() {
        return (*array)[index];
    }
    const T &operator *() const {
        return (*array)[index];
    }
};

template <typename T, u64 BUCKET_SIZE>
struct Bucket_Array {
    Array <void *> buckets;
    u64 count = 0;
    
    static const u64 bucket_index_shift = constexpr_count_bits_set(BUCKET_SIZE - 1);
    static const u64 item_index_mask = BUCKET_SIZE - 1;
    static_assert((BUCKET_SIZE & item_index_mask) == 0, "Must be power of two!");
    
    static inline const u64 get_bucket_index(u64 index) {
        return index >> bucket_index_shift;
    }
    static inline const u64 get_item_index(u64 index) {
        return index & item_index_mask;
    }
    
    
    inline void init(Base_Allocator *allocator) {
        this->buckets.init(allocator);
    }
    
    inline void free(void) {
        For (buckets) {
            base_free(buckets.allocator, it);
        }
        buckets.free();
        count = 0;
    }
    
    inline void reset(void) {
        count = 0;
    }
    
    void reserve(u64 size) {
        Profile_Zone();
        u64 bucket_count = (size + BUCKET_SIZE - 1) / BUCKET_SIZE;
        if (bucket_count <= buckets.count)  return;
        bucket_count -= buckets.count;
        
        for (int i = 0; i < bucket_count; ++i) {
            add_new_bucket();
        }
    }
    
    void resize(u64 size) {
        reserve(size);
        auto old_count = count;
        count = size;
#if 0
        for (u64 i = old_count; i < count; ++i) {
            auto *it = &(*this)[i];
            memset(it, 0, sizeof(T));
        }
#else
        if (size > old_count) {
            u64 old_bucket_index = get_bucket_index(old_count);
            u64 old_item_index   = get_item_index(old_count);
            
            u64 index = (count > 0) ? count-1 : 0;
            u64 new_bucket_index = get_bucket_index(index);
            
            T *bucket = get_bucket(old_count);
            memset(bucket + old_item_index, 0, (BUCKET_SIZE - old_item_index) * sizeof(T));
            
            for (u64 b = old_bucket_index + 1; b <= new_bucket_index; ++b) {
                T *bucket = cast(T *) buckets[b];
                memset(bucket, 0, BUCKET_SIZE * sizeof(T));
            }
        }
#endif
    }
    
    void resize(u64 size, T value) {
        reserve(size);
        auto old_count = count;
        count = size;
        for (u64 i = old_count; i < count; ++i) {
            (*this)[i] = value;
        }
    }
    
    
    inline void *add_new_bucket() {
        Profile_Zone();
        void *bucket = base_allocate(buckets.allocator, BUCKET_SIZE * sizeof(T)).data;
        buckets.add(bucket);
        return bucket;
    }
    
    inline T *get_bucket(u64 index) {
        const u64 bucket_index = get_bucket_index(index);
        return cast(T *) buckets[bucket_index];
    }
    
    inline T *get_bucket(u64 index) const {
        const u64 bucket_index = get_bucket_index(index);
        return cast(T *) buckets[bucket_index];
    }
    
    
    T *add_uninitialized(void) {
        Profile_Zone();
        const u64 bucket_index = get_bucket_index(count);
        const u64 item_index   = get_item_index(count);
        count += 1;
        
        void *bucket;
        if (bucket_index >= buckets.count) {
            bucket = add_new_bucket();
        }
        else {
            bucket = buckets[bucket_index];
        }
        assert(bucket);
        
        T *result = cast(T *) bucket + item_index;
        
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
        u64 index = count;
        count++;
        (*this)[index] = item;
    }
    
    
    T &operator[](const u64 index) {
        assert(index < count);
        
        T *bucket = get_bucket(index);
        const u64 item_index   = get_item_index(index);
        
        assert(item_index < BUCKET_SIZE);
        
        return bucket[item_index];
    }
    
    const T &operator[](const u64 index) const {
        assert(index < count);
        
        const T *bucket = get_bucket(index);
        const u64 item_index   = get_item_index(index);
        
        assert(item_index < BUCKET_SIZE);
        
        return bucket[item_index];
    }
    
    Bucket_Array_Iterator <T, BUCKET_SIZE> begin(void) {
        return Bucket_Array_Iterator <T, BUCKET_SIZE>(this, 0);
    }
    
    Bucket_Array_Iterator <T, BUCKET_SIZE> end(void) {
        return Bucket_Array_Iterator <T, BUCKET_SIZE>(this, count);
    }
};



//
// @note: Tests
//
#include "iml_test_suit.h"

global_unit("Bucket_Array") {
    typedef Bucket_Array<int, 2>  Int2_Bucket_Array;
    static Int2_Bucket_Array *array;
    static const int test_data_count = 4;
    static int test_data[test_data_count] = { 123, 555, 666, 911 };
    
    before_all() {
        array = push_struct(&__ts_config__->arena, Int2_Bucket_Array);
        *array = {};
        
        check(Int2_Bucket_Array::bucket_index_shift == 1);
        check(Int2_Bucket_Array::item_index_mask == 0x1);
        
        check(array->count == 0);
        
        check(array->buckets.allocator);
        check(array->buckets.data == null);
        check(array->buckets.count == 0);
        check(array->buckets.allocated == 0);
    };
    
    after_each() {
        array->free();
        array->reset();
    };
    
    
    test("add_new_bucket()") {
        void *bucket = array->add_new_bucket();
        check(bucket);
        check(bucket == array->buckets[0]);
    };
    
    test("get_bucket_index(index)") {
        check(array->get_bucket_index(0) == 0);
        check(array->get_bucket_index(1) == 0);
        check(array->get_bucket_index(2) == 1);
        check(array->get_bucket_index(3) == 1);
    };
    
    test("get_item_index(index)") {
        check(array->get_item_index(0) == 0);
        check(array->get_item_index(1) == 1);
        check(array->get_item_index(2) == 0);
        check(array->get_item_index(3) == 1);
    };
    
    test("get_bucket(index)") {
        array->reserve(4);
        check(array->get_bucket(1) == array->buckets[0]);
        check(array->get_bucket(3) == array->buckets[1]);
    };
    
    test("get_bucket(index) const") {
        array->reserve(4);
        check(((const Int2_Bucket_Array *)array)->get_bucket(1) == array->buckets[0]);
        check(((const Int2_Bucket_Array *)array)->get_bucket(3) == array->buckets[1]);
    };
    
    test("free()") {
        array->add(7531);
        array->free();
        
        check(array->count == 0);
        
        check(array->buckets.count == 0);
        check(array->buckets.data == null);
        check(array->buckets.allocated == 0);
    };
    
    test("reset()") {
        array->add();
        check(array->count > 0);
        array->reset();
        check(array->count == 0);
    };
    
    test("reserve(size)") {
        // Reserve zero size.
        array->reserve(0);
        check(array->buckets.count == 0);
        
        // Reserve size more than one bucket.
        array->reserve(3);
        check(array->buckets.count == 2);
        check(array->buckets[0]);
        check(array->buckets[1]);
        
        // Reserve less than already allocated.
        array->reserve(1);
        check(array->buckets.count == 2);
        check(array->buckets[0]);
        check(array->buckets[1]);
        
        // Reserve same as already allocated
        array->reserve(4);
        check(array->buckets.count == 2);
        check(array->buckets[0]);
        check(array->buckets[1]);
        
        // Reserve more than already allocated.
        array->reserve(5);
        check(array->buckets.count == 3);
        check(array->buckets[0]);
        check(array->buckets[1]);
        check(array->buckets[2]);
    };
    
    test("resize(size)") {
        int size = 2;
        array->resize(size);
        check(array->buckets.count == 1);
        check(array->count == size);
        
        // Resize more than in use, should be cleared to zero.
        size += 2;
        array->resize(size);
        check(array->buckets.count == 2);
        check(array->count == size);
        check((*array)[size - 1] == 0);
        
        // Resize fewer than in use
        size -= 2;
        array->resize(size);
        check(array->buckets.count == 2);
        check(array->count == size);
        
        // Make sure memory gets really cleared.
        array->reset();
        array->reserve(4);
        array->count = 4;
        (*array)[0] = 12;
        (*array)[1] = 34;
        (*array)[2] = 56;
        (*array)[3] = 78;
        array->count = 0;
        array->resize(4);
        check((*array)[0] == 0);
        check((*array)[1] == 0);
        check((*array)[2] == 0);
        check((*array)[3] == 0);
        
        // Test if every possible size gets cleared correctly.
        for (size_t iter = 0; iter < 8; iter++) {
            array->resize(8);
            for (size_t i = 0; i < 8; i++) {
                (*array)[i] = 1;
            }
            array->resize(0);
            array->resize(iter);
            for (size_t i = 0; i < array->count; i++) {
                check((*array)[i] == 0);
            }
            array->count = 8;
            for (size_t i = array->count; i < (8 - iter); i++) {
                check((*array)[i] == 1);
            }
        }
    };
    
    test("resize(size, value)") {
        int value = 1;
        
        int size = 2;
        array->resize(size, value);
        check(array->buckets.count == 1);
        check(array->count == size);
        
        // Resize more than in use, should be cleared to zero.
        size += 2;
        array->resize(size, 1);
        check(array->buckets.count == 2);
        check(array->count == size);
        check((*array)[size - 1] == value);
        
        // Resize fewer than in use
        size -= 2;
        array->resize(size, value);
        check(array->buckets.count == 2);
        check(array->count == size);
        
        // Make sure memory gets really cleared.
        array->reset();
        array->reserve(4);
        array->count = 4;
        (*array)[0] = 12;
        (*array)[1] = 34;
        (*array)[2] = 56;
        (*array)[3] = 78;
        array->count = 0;
        array->resize(4, value);
        check((*array)[0] == value);
        check((*array)[1] == value);
        check((*array)[2] == value);
        check((*array)[3] == value);
        
        // Test if every possible size gets cleared correctly.
        for (size_t iter = 0; iter < 8; iter++) {
            array->resize(8);
            for (size_t i = 0; i < 8; i++) {
                (*array)[i] = 0;
            }
            array->resize(0);
            array->resize(iter, 1);
            for (size_t i = 0; i < array->count; i++) {
                check((*array)[i] == 1);
            }
            array->count = 8;
            for (size_t i = array->count; i < (8 - iter); i++) {
                check((*array)[i] == 0);
            }
        }
    };
    
    test("add_uninitialized(void") {
        int *a = array->add_uninitialized();
        check(a);
        check(array->count == 1);
        check(array->buckets.count == 1);
        
        int *b = array->add_uninitialized();
        check(b);
        check(array->count == 2);
        check(array->buckets.count == 1);
        
        int *c = array->add_uninitialized();
        check(c);
        check(array->count == 3);
        check(array->buckets.count == 2);
    };
    
    test("add()") {
        int *a = array->add();
        check(a);
        check(*a == 0);
        check(array->count == 1);
        check(array->buckets.count == 1);
        
        int *b = array->add();
        check(b);
        check(*b == 0);
        check(array->count == 2);
        check(array->buckets.count == 1);
        
        int *c = array->add();
        check(c);
        check(*c == 0);
        check(array->count == 3);
        check(array->buckets.count == 2);
    };
    
    test("add(item)") {
        array->add(test_data[0]);
        check(array->count == 1);
        check(array->buckets.count == 1);
        check((*array)[0] == test_data[0]);
        
        array->add(test_data[1]);
        check(array->count == 2);
        check(array->buckets.count == 1);
        check((*array)[1] == test_data[1]);
        
        array->add(test_data[2]);
        check(array->count == 3);
        check(array->buckets.count == 2);
        check((*array)[2] == test_data[2]);
    };
    
    test("operator[]") {
        array->add(test_data[0]);
        check((*array)[0] == test_data[0]);
        
        array->add(test_data[1]);
        check((*((const Int2_Bucket_Array *)array))[1] == test_data[1]);
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
                check((*array)[i] == (test_data[i] + 2));
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

#endif
