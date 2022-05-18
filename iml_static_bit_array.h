#if !defined(IML_STATIC_BIT_ARRAY_H)
#define IML_STATIC_BIT_ARRAY_H

#if defined(IML_NAMESPACE)
namespace IML_NAMESPACE {
#  if 0
}
#  endif
#endif
//
//~


#include "iml_types.h"
#include "iml_bits.h"


template<u64 count>
struct Static_Bit_Array;


struct Static_Bit_Array_Proxy {
    u64 *bucket;
    const u64 item_index;
    
    Static_Bit_Array_Proxy(u64 *_bucket, u64 _item_index) : bucket(_bucket), item_index(_item_index) {};
    
    operator bool() const {
        return BIT_CHECK_AT_INDEX(*bucket, item_index);
    }
    
    void operator =(bool value) {
        BIT_SET_VALUE_AT_INDEX(*bucket, item_index, value);
    }
};

template<u64 count>
struct Static_Bit_Array_Iterator {
    Static_Bit_Array<count> *array;
    u64 index;
    
    Static_Bit_Array_Iterator(Static_Bit_Array<count> *_array, u64 _index) : array(_array), index(_index) {};
    
    bool operator != (Static_Bit_Array_Iterator &other) const {
        return (index != other.index);
    }
    
    const Static_Bit_Array_Iterator &operator ++() {
        index += 1;
        return *this;
    }
    
    Static_Bit_Array_Proxy operator *() const;
};

template<u64 count>
struct Static_Bit_Array_Const_Iterator {
    const Static_Bit_Array<count> *array;
    u64 index;
    
    Static_Bit_Array_Const_Iterator(const Static_Bit_Array<count> *_array, u64 _index) : array(_array), index(_index) {};
    
    bool operator != (const Static_Bit_Array_Const_Iterator &other) const {
        return (index != other.index);
    }
    
    const Static_Bit_Array_Const_Iterator &operator ++() {
        index += 1;
        return *this;
    }
    
    bool operator *() const;
};


template<u64 count>
struct Static_Bit_Array {
    static const u64 BUCKET_SIZE = bit_size_of(u64);
    static_assert((count % BUCKET_SIZE) == 0, "'count' must be divisible by the bucket size!");
    static const u64 BUCKET_COUNT = count / BUCKET_SIZE;
    
    static const u64 bucket_index_shift = constexpr_count_bits_set(BUCKET_SIZE - 1);
    static const u64 item_index_mask = BUCKET_SIZE - 1;
    
    static inline const u64 get_bucket_index(u64 index) {
        return index >> bucket_index_shift;
    }
    static inline const u64 get_item_index(u64 index) {
        return index & item_index_mask;
    }
    
    
    u64 buckets[count] = {0};
    
    
    void reset(void) {
        memset(buckets, 0, BUCKET_COUNT * sizeof(u64));
    }
    
    
    inline u64 *get_bucket(u64 index) {
        const u64 bucket_index = get_bucket_index(index);
        return &buckets[bucket_index];
    }
    
    inline const u64 *get_bucket(u64 index) const {
        const u64 bucket_index = get_bucket_index(index);
        return &buckets[bucket_index];
    }
    
    
    void set_bit(u64 index) {
        assert(index < count);
        u64 *bucket = get_bucket(index);
        const u64 item_index = get_item_index(index);
        
        BIT_SET_AT_INDEX(*bucket, item_index);
    }
    
    void set_bit(u64 index, bool value) {
        assert(index < count);
        u64 *bucket = get_bucket(index);
        const u64 item_index = get_item_index(index);
        
        BIT_SET_VALUE_AT_INDEX(*bucket, item_index, value);
    }
    
    void clear_bit(u64 index) {
        assert(index < count);
        u64 *bucket = get_bucket(index);
        const u64 item_index = get_item_index(index);
        
        BIT_CLEAR_AT_INDEX(*bucket, item_index);
    }
    
    void toggle_bit(u64 index) {
        assert(index < count);
        u64 *bucket = get_bucket(index);
        const u64 item_index = get_item_index(index);
        
        BIT_TOGGLE_AT_INDEX(*bucket, item_index);
    }
    
    bool get_bit(u64 index) const {
        assert(index < count);
        const u64 *bucket = get_bucket(index);
        const u64 item_index = get_item_index(index);
        
        bool result = BIT_CHECK_AT_INDEX(*bucket, item_index);
        return result;
    }
    
    u64 bits_set(void) const {
        u64 result = 0;
        For (buckets) {
            result+= count_bits_set(it);
        }
        return result;
    }
    
    
    Static_Bit_Array_Proxy operator[](const u64 index) {
        assert(index < count);
        u64 *bucket = get_bucket(index);
        const u64 item_index = get_item_index(index);
        
        Static_Bit_Array_Proxy result(bucket, item_index);
        return result;
    }
    
    const bool operator[](const u64 index) const {
        assert(index < count);
        const u64 *bucket = get_bucket(index);
        const u64 item_index = get_item_index(index);
        
        return BIT_CHECK_AT_INDEX(*bucket, item_index);
    }
    
    Static_Bit_Array_Iterator <count> begin(void) {
        return Static_Bit_Array_Iterator<count>(this, 0);
    }
    
    Static_Bit_Array_Iterator <count> end(void) {
        return Static_Bit_Array_Iterator<count>(this, count);
    }
    
    Static_Bit_Array_Const_Iterator <count> begin(void) const {
        return Static_Bit_Array_Const_Iterator<count>(this, 0);
    }
    
    Static_Bit_Array_Const_Iterator <count> end(void) const {
        return Static_Bit_Array_Const_Iterator<count>(this, count);
    }
};


template<u64 count>
Static_Bit_Array_Proxy Static_Bit_Array_Iterator<count>::operator *() const {
    return (*array)[index];
}

template<u64 count>
bool Static_Bit_Array_Const_Iterator<count>::operator *() const {
    return array->get_bit(index);
}



//
// @note: Tests
//
#include "iml_test_suit.h"

global_unit("Static_Bit_Array") {
    typedef Static_Bit_Array<128>  Static_Bit_Array_128;
    static Static_Bit_Array_128 *array;
    
    before_all() {
        array = push_struct(&__ts_config__->arena, Static_Bit_Array_128);
        *array = {};
        
        check(Static_Bit_Array_128::BUCKET_SIZE == 64);
        check(Static_Bit_Array_128::BUCKET_COUNT == 2);
        check(Static_Bit_Array_128::bucket_index_shift == 6);
        check(Static_Bit_Array_128::item_index_mask == 0b111111);
        
        check(array->buckets[0] == 0);
        check(array->buckets[1] == 0);
    };
    
    after_each() {
        array->reset();
    };
    
    
    test("reset()") {
        array->buckets[0] = 7531;
        array->buckets[1] = 2690;
        array->reset();
        check(array->buckets[0] == 0);
        check(array->buckets[1] == 0);
    };
    
    test("get_bucket_index(index)") {
        check(array->get_bucket_index(0) == 0);
        check(array->get_bucket_index(64) == 1);
    };
    
    test("get_item_index(index)") {
        check(array->get_item_index(0) == 0);
        check(array->get_item_index(1) == 1);
        check(array->get_item_index(63) == 63);
        check(array->get_item_index(64) == 0);
        check(array->get_item_index(65) == 1);
        check(array->get_item_index(127) == 63);
    };
    
    test("get_bucket(index)") {
        check(array->get_bucket(0) == &array->buckets[0]);
        check(array->get_bucket(64) == &array->buckets[1]);
    };
    
    test("get_bucket(index) const") {
        check(((const Static_Bit_Array_128 *)array)->get_bucket(0) == &array->buckets[0]);
        check(((const Static_Bit_Array_128 *)array)->get_bucket(64) == &array->buckets[1]);
    };
    
    test("set_bit(index)") {
        array->set_bit(0);
        array->set_bit(63);
        check(array->buckets[0] == 0x8000000000000001);
        
        array->set_bit(64);
        array->set_bit(127);
        check(array->buckets[1] == 0x8000000000000001);
    };
    
    test("set_bit(index, value)") {
        array->set_bit(0, true);
        array->set_bit(63, true);
        check(array->buckets[0] == 0x8000000000000001);
        
        array->set_bit(64, true);
        array->set_bit(127, true);
        check(array->buckets[1] == 0x8000000000000001);
        
        array->set_bit(63, false);
        check(array->buckets[0] == 0x1);
        
        array->set_bit(127, false);
        check(array->buckets[1] == 0x1);
    };
    
    test("clear_bit(index)") {
        array->set_bit(64, true);
        array->set_bit(127, true);
        array->clear_bit(127);
        check(array->buckets[1] == 0x1);
    };
    
    test("toggle_bit(index)") {
        array->toggle_bit(64);
        array->toggle_bit(127);
        check(array->buckets[1] == 0x8000000000000001);
        array->toggle_bit(127);
        check(array->buckets[1] == 0x1);
    };
    
    test("get_bit(index)") {
        array->toggle_bit(64);
        check(array->get_bit(0) == false);
        check(array->get_bit(64) == true);
    };
    
    test("bits_set()") {
        array->toggle_bit(1);
        array->toggle_bit(64);
        array->toggle_bit(127);
        check(array->bits_set() == 3);
    };
    
    test("operator[]") {
        (*array)[64] = true;
        (*array)[127] = true;
        check((*array)[64] == true);
        check((*array)[127] == true);
        check(array->buckets[1] == 0x8000000000000001);
        
        (*array)[127] = false;
        check(array->buckets[1] == 0x1);
    };
    
    test("const operator[]") {
        array->toggle_bit(64);
        array->toggle_bit(127);
        check(array->buckets[1] == 0x8000000000000001);
        check((*(const Static_Bit_Array_128 *)array)[64] == true);
    };
    
    group("iterators") {
        static int i;
        
        before_each() {
            i = 0;
            for (int i = 0; i < 128; i++) {
                array->set_bit(i, ((i % 2) == 0));
            }
        };
        
        
        test("for (&)") {
            for (auto it : *array) {
                check(array->get_bit(i) == ((i % 2) == 0));
                check(it == array->get_bit(i));
                it = !it;
                check(array->get_bit(i) == ((i % 2) == 1));
                i++;
            }
        };
        
        test("for (const &)") {
            for (const bool it : *array) {
                check(array->get_bit(i) == ((i % 2) == 0));
                check(it == array->get_bit(i));
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
#endif  // IML_STATIC_BIT_ARRAY_H
