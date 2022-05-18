#if !defined(IML_BIT_ARRAY_H)
#define IML_BIT_ARRAY_H

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


struct Bit_Array;

struct Bit_Array_Proxy {
    u64 *bucket;
    const u64 item_index;
    
    Bit_Array_Proxy(u64 *_bucket, u64 _item_index) : bucket(_bucket), item_index(_item_index) {};
    
    operator bool() const {
        return (*bucket >> item_index) & 0x1;
    }
    
    void operator =(bool value) {
        BIT_SET_VALUE_AT_INDEX(*bucket, item_index, value);
    }
};

struct Const_Bit_Array_Proxy {
    const u64 *bucket;
    const u64 item_index;
    
    Const_Bit_Array_Proxy(const u64 *_bucket, const u64 _item_index) : bucket(_bucket), item_index(_item_index) {};
    
    operator bool() const {
        return BIT_CHECK_AT_INDEX(*bucket, item_index);
    }
};

struct Bit_Array_Iterator {
    Bit_Array *array;
    u64 index;
    
    Bit_Array_Iterator(Bit_Array *_array, u64 _index) : array(_array), index(_index) {};
    
    bool operator != (Bit_Array_Iterator &other) const {
        return (index != other.index);
    }
    
    const Bit_Array_Iterator &operator ++() {
        index += 1;
        return *this;
    }
    
    Bit_Array_Proxy operator *() const;
};

struct Bit_Array_Const_Iterator {
    const Bit_Array *array;
    u64 index;
    
    Bit_Array_Const_Iterator(const Bit_Array *_array, u64 _index) : array(_array), index(_index) {};
    
    bool operator != (const Bit_Array_Const_Iterator &other) const {
        return (index != other.index);
    }
    
    const Bit_Array_Const_Iterator &operator ++() {
        index += 1;
        return *this;
    }
    
    bool operator *() const;
};


struct Bit_Array {
    Array <u64> buckets;
    u64 count = 0;
    
    static const u64 BUCKET_SIZE        = bit_size_of(u64);
    static const u64 bucket_index_shift = constexpr_count_bits_set(BUCKET_SIZE - 1);
    static const u64 item_index_mask    = BUCKET_SIZE - 1;
    
    static inline const u64 get_bucket_index(u64 index) {
        return index >> bucket_index_shift;
    }
    static inline const u64 get_item_index(u64 index) {
        return index & item_index_mask;
    }
    
    
    inline void init(Base_Allocator *allocator) {
        buckets.init(allocator);
    }
    
    inline void free(void) {
        buckets.free();
        count = 0;
    }
    
    inline void reset(void) {
        buckets.reset();
        count = 0;
    }
    
    void reserve(u64 size) {
        assert(size >= 0);
        u64 bucket_count = (size + BUCKET_SIZE - 1) / BUCKET_SIZE;
        if (bucket_count <= buckets.count)  return;
        buckets.reserve(bucket_count);  // @note: Not using reserve, so that we are not clearing the memory. This happens on add or reserve.
        buckets.count = bucket_count;
    }
    
    void resize(u64 size) {
        assert(size >= 0);
        reserve(size);
        auto old_count = count;
        count = size;
#if 0
        for (u64 i = old_count; i < size; ++i) {
            clear_bit(i);
        }
#else
        if (size > old_count) {
            u64 old_bucket_index = get_bucket_index(old_count);
            u64 old_item_index   = get_item_index(old_count);
            
            u64 index = (count > 0) ? count-1 : 0;
            u64 new_bucket_index = get_bucket_index(index);
            
            u64 *bucket = get_bucket(old_count);
            u64 mask = ((u64)1 << old_item_index) - 1;
            *bucket &= mask;
            
            for (u64 b = old_bucket_index + 1; b <= new_bucket_index; ++b) {
                buckets[b] = 0;
            }
        }
#endif
    }
    
    void resize(u64 size, bool value) {
        assert(size >= 0);
        reserve(size);
        auto old_count = count;
        count = size;
        for (u64 i = old_count; i < size; ++i) {
            set_bit(i, value);
        }
    }
    
    inline u64 add_uninitialized(void) {
        reserve(count + 1);
        u64 index = count;
        count += 1;
        return index;
    }
    
    inline u64 add(void) {
        u64 result = add_uninitialized();
        clear_bit(result);
        return result;
    }
    
    inline u64 add(bool item) {
        reserve(count + 1);
        u64 index = count;
        count += 1;
        set_bit(index, item);
        return index;
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
    
    u64 bits_set(void) const {
        u64 count = 0;
        For (buckets) {
            count += count_bits_set(it);
        }
        return count;
    }
    
    bool get_bit(u64 index) const {
        assert(index < count);
        const u64 *bucket = get_bucket(index);
        const u64 item_index = get_item_index(index);
        
        bool result = BIT_CHECK_AT_INDEX(*bucket, item_index);
        return result;
    }
    
    
    Bit_Array_Proxy operator[](const u64 index) {
        assert(index < count);
        u64 *bucket = get_bucket(index);
        const u64 item_index = get_item_index(index);
        
        Bit_Array_Proxy result(bucket, item_index);
        return result;
    }
    
    const Const_Bit_Array_Proxy operator[](const u64 index) const {
        assert(index < count);
        const u64 *bucket = get_bucket(index);
        const u64 item_index = get_item_index(index);
        
        Const_Bit_Array_Proxy result(bucket, item_index);
        return result;
    }
    
    Bit_Array_Iterator begin(void) {
        return Bit_Array_Iterator(this, 0);
    }
    
    Bit_Array_Iterator end(void) {
        return Bit_Array_Iterator(this, count);
    }
    
    Bit_Array_Const_Iterator begin(void) const {
        return Bit_Array_Const_Iterator(this, 0);
    }
    
    Bit_Array_Const_Iterator end(void) const {
        return Bit_Array_Const_Iterator(this, count);
    }
};


Bit_Array_Proxy Bit_Array_Iterator::operator *() const {
    return (*array)[index];
}

bool Bit_Array_Const_Iterator::operator *() const {
    return array->get_bit(index);
}



//
// @note: Tests
//
#include "iml_test_suit.h"

global_unit("Bit_Array") {
    typedef Bit_Array  Bit_Array_u64;
    static Bit_Array_u64 *array;
    
    before_all() {
        array = push_struct(&__ts_config__->arena, Bit_Array_u64);
        *array = {};
        
        check(Bit_Array_u64::BUCKET_SIZE == 64);
        check(Bit_Array_u64::bucket_index_shift == 6);
        check(Bit_Array_u64::item_index_mask == 0b111111);
        
        check(array->count == 0);
        
        check(array->buckets.allocator);
        check(array->buckets.data == null);
        check(array->buckets.count == 0);
        check(array->buckets.allocated == 0);
    };
    
    after_each() {
        array->free();
    };
    
    
    test("free()") {
        array->add(true);
        array->free();
        
        check(array->count == 0);
        
        check(array->buckets.count == 0);
        check(array->buckets.data == null);
        check(array->buckets.allocated == 0);
    };
    
    test("reset()") {
        array->reserve(128);
        check(array->buckets.count == 2);
        array->buckets[0] = 7531;
        array->buckets[1] = 2690;
        array->reset();
        check(array->count == 0);
        check(array->buckets.allocated >= 2);
        check(array->buckets.count == 0);
    };
    
    test("reserve(size)") {
        // Reserve zero size.
        array->reserve(0);
        check(array->buckets.count == 0);
        
        // Reserve size more than one bucket.
        array->reserve(65);
        check(array->buckets.count == 2);
        
        // Reserve less than already allocated.
        array->reserve(1);
        check(array->buckets.count == 2);
        
        // Reserve same as already allocated
        array->reserve(128);
        check(array->buckets.count == 2);
        
        // Reserve more than already allocated.
        array->reserve(155);
        check(array->buckets.count == 3);
    };
    
    test("resize(size)") {
        int size = 128;
        array->resize(size);
        check(array->buckets.count == 2);
        check(array->count == size);
        
        // Resize more than in use, should be cleared to zero.
        size += 64;
        array->resize(size);
        check(array->buckets.count == 3);
        check(array->count == size);
        check((*array)[size - 1] == false);
        
        // Resize fewer than in use
        size -= 64;
        array->resize(size);
        check(array->buckets.count == 3);
        check(array->count == size);
        
        // Make sure the memory was cleared.
        array->reset();
        array->resize(192);
        array->buckets[0] = U64_MAX;
        array->buckets[1] = U64_MAX;
        array->buckets[2] = U64_MAX;
        array->resize(32);
        array->resize(192);  // 32 bits of the first buckets get cleared, and the full second and third bucket.
        check(array->buckets.count == 3);
        check(array->buckets[0] == 0x00000000FFFFFFFF);
        check(array->buckets[1] == 0);
        check(array->buckets[2] == 0);
        
        // Test if every possible size gets cleared correctly.
        for (size_t iter = 0; iter < 128; iter++) {
            array->resize(128);
            for (size_t i = 0; i < 128; i++) {
                (*array)[i] = true;
            }
            array->resize(0);
            array->resize(iter);
            for (size_t i = 0; i < array->count; i++) {
                check((*array)[i] == false);
            }
            array->count = 128;
            for (size_t i = array->count; i < (128 - iter); i++) {
                check((*array)[i] == true);
            }
        }
    };
    
    test("resize(size, value)") {
        bool value = true;
        
        int size = 128;
        array->resize(size, value);
        check(array->buckets.count == 2);
        check(array->count == size);
        
        // Resize more than in use, should be cleared to zero.
        size += 64;
        array->resize(size, value);
        check(array->buckets.count == 3);
        check(array->count == size);
        check((*array)[size - 1] == true);
        
        // Resize fewer than in use
        size -= 64;
        array->resize(size, value);
        check(array->buckets.count == 3);
        check(array->count == size);
        
        // Make sure the memory was cleared.
        array->reset();
        array->reserve(128);
        array->resize(32, value);
        array->buckets[0] = 0;
        array->buckets[1] = 0;
        array->resize(192, value);  // 32 bits of the first buckets get cleared, and the full second and third bucket.
        check(array->buckets.count == 3);
        check(array->buckets[0] == 0xFFFFFFFF00000000);
        check(array->buckets[1] == U64_MAX);
        check(array->buckets[2] == U64_MAX);
        
        // Test if every possible size gets cleared correctly.
        for (size_t iter = 0; iter < 128; iter++) {
            array->resize(128);
            for (size_t i = 0; i < 128; i++) {
                (*array)[i] = false;
            }
            array->resize(0);
            array->resize(iter, true);
            for (size_t i = 0; i < array->count; i++) {
                check((*array)[i] == true);
            }
            array->count = 128;
            for (size_t i = array->count; i < (128 - iter); i++) {
                check((*array)[i] == false);
            }
        }
    };
    
    test("add_uninitialized()") {
        array->reserve(64);
        array->buckets[0] = U64_MAX;
        
        size_t index_a = array->add_uninitialized();
        check(index_a == 0);
        check(array->count == 1);
    };
    
    test("add()") {
        array->reserve(64);
        array->buckets[0] = U64_MAX;
        
        size_t index_a = array->add();
        check(index_a == 0);
        check(array->count == 1);
        check((*array)[index_a] == false);
        
        size_t index_b = array->add(true);
        check(index_b == 1);
        check(array->count == 2);
        check((*array)[index_b] == true);
        
        array->free();
        size_t index_c = array->add(true);
        check(index_c == 0);
        check(array->count == 1);
        check(array->buckets.count >= 1);
        check((*array)[index_c] == true);
    };
    
    test("add(item)") {
        array->reserve(64);
        array->buckets[0] = U64_MAX;
        
        size_t index_a = array->add(false);
        check(index_a == 0);
        check(array->count == 1);
        check((*array)[index_a] == false);
        
        size_t index_b = array->add(true);
        check(index_b == 1);
        check(array->count == 2);
        check((*array)[index_b] == true);
        
        array->free();
        size_t index_c = array->add(true);
        check(index_c == 0);
        check(array->count == 1);
        check(array->buckets.count >= 1);
        check((*array)[index_c] == true);
    };
    
    test("get_bucket_index(index)") {
        check(Bit_Array_u64::BUCKET_SIZE == 64);
        check(array->get_bucket_index(0) == 0);
        check(array->get_bucket_index(64) == 1);
    };
    
    test("get_item_index(index)") {
        check(Bit_Array_u64::BUCKET_SIZE == 64);
        check(array->get_item_index(0) == 0);
        check(array->get_item_index(1) == 1);
        check(array->get_item_index(63) == 63);
        check(array->get_item_index(64) == 0);
        check(array->get_item_index(65) == 1);
        check(array->get_item_index(127) == 63);
    };
    
    test("get_bucket(index)") {
        check(Bit_Array_u64::BUCKET_SIZE == 64);
        array->reserve(128);
        check(array->get_bucket(0) == &array->buckets[0]);
        check(array->get_bucket(64) == &array->buckets[1]);
    };
    
    test("get_bucket(index) const") {
        check(Bit_Array_u64::BUCKET_SIZE == 64);
        array->reserve(128);
        check(((const Bit_Array_u64 *)array)->get_bucket(0) == &array->buckets[0]);
        check(((const Bit_Array_u64 *)array)->get_bucket(64) == &array->buckets[1]);
    };
    
    test("set_bit(index)") {
        array->resize(128);
        
        array->set_bit(0);
        array->set_bit(63);
        check(array->buckets[0] == 0x8000000000000001);
        
        array->set_bit(64);
        array->set_bit(127);
        check(array->buckets[1] == 0x8000000000000001);
    };
    
    test("set_bit(index, value)") {
        array->resize(128);
        
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
        array->resize(128);
        
        array->set_bit(64, true);
        array->set_bit(127, true);
        array->clear_bit(127);
        check(array->buckets[1] == 0x1);
    };
    
    test("toggle_bit(index)") {
        array->resize(128);
        
        array->toggle_bit(64);
        array->toggle_bit(127);
        check(array->buckets[1] == 0x8000000000000001);
        array->toggle_bit(127);
        check(array->buckets[1] == 0x1);
    };
    
    test("get_bit(index)") {
        array->resize(128);
        
        array->toggle_bit(64);
        check(array->get_bit(0) == false);
        check(array->get_bit(64) == true);
    };
    
    test("bits_set()") {
        array->resize(128);
        
        array->toggle_bit(1);
        array->toggle_bit(64);
        array->toggle_bit(127);
        check(array->bits_set() == 3);
    };
    
    test("operator[]") {
        array->resize(128);
        
        (*array)[64] = true;
        (*array)[127] = true;
        check((*array)[64] == true);
        check((*array)[127] == true);
        check(array->buckets[1] == 0x8000000000000001);
        
        (*array)[127] = false;
        check(array->buckets[1] == 0x1);
    };
    
    test("const operator[]") {
        array->resize(128);
        
        array->toggle_bit(64);
        array->toggle_bit(127);
        check(array->buckets[1] == 0x8000000000000001);
        check((*(const Bit_Array_u64 *)array)[64] == true);
    };
    
    group("iterators") {
        static int i;
        
        before_each() {
            i = 0;
            array->resize(128);
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

#endif // IML_BIT_ARRAY_H
