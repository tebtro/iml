#if !defined(IML_STRING_H)
#define IML_STRING_H

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
// @note: cstring
//
inline void convert_to_back_slashes(char *c) {
    while (*c) {
        if (*c == '/') {
            *c = '\\';
        }
        
        c++;
    }
}

inline void convert_to_forward_slashes(char *c) {
    while (*c) {
        if (*c == '\\') {
            *c = '/';
        }
        
        c++;
    }
}


//
// @note: Buffer
//
struct Buffer {
    smm count;
    u8 *data;
    
    u8 &operator[](smm index) const {
        assert(index >= 0 && index < count);
        return data[index];
    }
};


inline void put_char(Buffer *dest, char value) {
    assert(dest->count > 0, "\nError: Not enough memory in the buffer for put_char!\n\n");
    
    dest->count  -= 1;
    if (dest->count >= 0) {
        *dest->data++ = value;
    }
}

#if 0
inline void put_chars(Buffer *dest, char *value) {
    while (*value) {
        put_char(dest, *value++);
    }
}
#endif

inline void put_chars(Buffer *dest, Buffer value) {
    assert(dest->count > 0, "\nError: Not enough memory in the buffer for put_chars!\n\n");
    
    dest->count -= value.count;
    if (dest->count >= 0) {
#if 0
        for (smm i = 0; i < value.count; ++i) {
            *dest->data++ = value.data[i];
        }
#else // @note: So the compiler wasnt able to realize that it could do a memcpy here. (Release build -O2).
        // 1.34x faster
        memcpy(dest->data, value.data, value.count);
        dest->data += value.count;
#endif
    }
}


//
// @note: String
//
// @todo: Maybe look at Agner Vogs string library (assembly stuff) for length, compares and stuff, ...
//
typedef Buffer  String;


#define file_name_line_number_string  make_string(FILE_AND_LINE_STRING)


inline u32 string_length(const char *string) {
    if (!string)  return 0;
    u32 count = 0;
    while (*string++) {
        ++count;
    }
    return count;
}

inline String make_string(const umm count, char *string) {
    String s;
    s.count = count;
    s.data = cast(u8 *) string;
    return s;
}

inline String make_string(const char *c_string) {
    String s;
    s.count = string_length(c_string);
    s.data  = cast(u8 *) c_string;
    return s;
}


//~ @note string_match

inline const bool strings_match(const char *a, const char *b) {
    if (!a)  return (b == null);
    if (!b)  return false;  // Because we know a is not NULL.
    
    int relationship = strcmp(a, b);
    return (relationship == 0);
}

inline const bool strings_match(const String a, const String b) {
    if (a.count != b.count)  return false;
    
    int relationship = memcmp(a.data, b.data, a.count);
    return (relationship == 0);
}

inline bool strings_match(const char *c_string, const String s_string) {
    if (!c_string)  return (s_string.count == 0);
    
    smm length = string_length(c_string);
    if (length != s_string.count)  return false;
    
    int relationship = memcmp(c_string, s_string.data, length);
    return (relationship == 0);
}
inline bool strings_match(const String s, const char *c) {
    return strings_match(c, s);
}

inline bool operator==(const String &a, const String &b) {
    return strings_match(a, b);
}
inline bool operator==(const String &a, const char *b_cstr) {
    return strings_match(a, b_cstr);
}
inline bool operator==(const char *a_cstr, const String &b) {
    return strings_match(a_cstr, b);
}
inline bool operator!=(const String &a, const String &b) {
    return !strings_match(a, b);
}
inline bool operator!=(const String &a, const char *b_cstr) {
    return !strings_match(a, b_cstr);
}
inline bool operator!=(const char *a_cstr, const String &b) {
    return !strings_match(a_cstr, b);
}

//~ @note string_to_*

// @note: Base 10 only, and only digits with a size of 9 or smaller!
inline int string_to_int(const char *str, smm length) {
    assert(length > 0);
    assert(length <= 9); // 9 digits never overflows
    
    const char *start = str;
    const char *end = start + length;
    
    int value = 0;
    for (; start < end; ++start) {
        value = value*10 + (*start - '0');
    }
    
    return value;
}


// @note No overflow checking!!!
inline s32 s32_from_string_cursor(const char **cursor_pointer) {
    s32 result = 0;
    
    const char *cursor = *cursor_pointer;
    while ((*cursor >= '0') && (*cursor <= '9')) {
        result *= 10;
        result += (*cursor - '0');
        cursor += 1;
    }
    *cursor_pointer = cursor;
    
    return result;
}

inline s32 s32_from_string(const char *str) {
    const char *cursor = str;
    s32 result = s32_from_string_cursor(&cursor);
    return result;
}


//~ @note *_to_string

global_const char decimal_chars[]       = "0123456789";
global_const char lowercase_hex_chars[] = "0123456789abcdef";
global_const char uppercase_hex_chars[] = "0123456789ABCDEF";
inline void u64_to_string(Buffer *buffer, u64 value, const u32 base = 10, const char *digit_chars = decimal_chars) {
    assert(base != 0);
    
    char *start = cast(char *)buffer->data;
    do {
        u64 digit_index = (value % base);
        char digit = digit_chars[digit_index];
        put_char(buffer, digit);
        
        value /= base;
    } while (value != 0);
    char *end = cast(char *)buffer->data;
    
    buffer->count = end - start;
    buffer->data = cast(u8 *)start;
    
    while (start < end) {
        --end;
        char temp = *end;
        *end = *start;
        *start = temp;
        ++start;
    }
}

inline void s64_to_string(Buffer *buffer, s64 value, const u32 base, const char *digit_chars, bool force_sign = false) {
    u8 *start = buffer->data;
    
    u8 prefix[1];
    smm prefix_size = 0;
    bool was_negative = (value < 0);
    if (was_negative) {
        value = -value;
        prefix[0] = '-';
        prefix_size = 1;
    }
    else if (force_sign) {
        prefix[0] = '+';
        prefix_size = 1;
    }
    
    put_chars(buffer, Buffer{prefix_size, prefix});
    u64_to_string(buffer, value, base, digit_chars);
    
    buffer->count += prefix_size;
    buffer->data = start;
}

inline void f64_to_string(Buffer *buffer, f64 value, u32 precision = 6) {
    u8 *start = buffer->data;
    
    if (value < 0) {
        put_char(buffer, '-');
        value = -value;
    }
    
    u64 integer_part = cast(u64)value;
    value -= cast(f64)integer_part;
    
    Buffer temp_buffer = *buffer;
    u64_to_string(&temp_buffer, integer_part);
    buffer->data += temp_buffer.count;
    buffer->count -= temp_buffer.count;
    
    
    put_char(buffer, '.');
    
    // @todo: Note that this is _NOT_ an accurate way to do this!
    for (u32 precision_index = 0; precision_index < precision; ++precision_index) {
        value *= 10.0f;
        u32 integer = cast(u32)value;
        value -= cast(f32)integer;
        put_char(buffer, decimal_chars[integer]);
    }
    
    buffer->count = buffer->data - start;
    buffer->data = start;
}

//
// @note: Utf8
//
typedef char  Utf8;

// @todo
Utf8 *copy_string(Utf8 *s);
Utf8 *copy_string(Utf8 *head, Utf8 *tail);
void free_string(Utf8 *s);


//
//~
#ifdef IML_NAMESPACE
#if 0
{
#endif
}
#endif

#endif // IML_STRING_H
