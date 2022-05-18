#if !defined(IML_STRING_VIEW_H)
#define IML_STRING_VIEW_H

#if defined(IML_NAMESPACE)
namespace IML_NAMESPACE {
#  if 0
}
#  endif
#endif
//
//~


struct String_View {
    size_t count;
    const char *data;
    
    [[nodiscard]] inline String_View trim_begin(void) const {
        String_View view = *this;
        
        while (view.count != 0 && is_space(*view.data)) {
            view.data  += 1;
            view.count -= 1;
        }
        
        return view;
    }
    
    [[nodiscard]] inline String_View trim_end(void) const {
        String_View view = *this;
        
        while (view.count != 0 && is_space(view.data[view.count - 1])) {
            view.count -= 1;
        }
        
        return view;
    }
    
    [[nodiscard]] inline String_View trim(void) const {
        return trim_begin().trim_end();
    }
    
    inline void chop(size_t n) {
        if (n > count) {
            data += count;
            count = 0;
        }
        else {
            data  += n;
            count -= n;
        }
    }
    
    inline void chop_back(size_t n) {
        if (n < count) {
            count -= n;
        }
        else {
            count = 0;
        }
    }
    
    inline void grow(size_t n) {
        count += n;
    }
    
    inline String_View subview(size_t start, size_t count) const {
        if ((start + count) < this->count) {
            return {count, data + start};
        }
        
        return {};
    }
    
    inline char operator[](size_t index) const {
        assert(index >= 0 && index < count);
        return data[index];
    }
};


inline String_View make_string_view(const char *cstr) {
    return { string_length(cstr), cstr };
}

inline String_View make_string_view(String string) {
    String_View view;
    view.count = string.count;
    view.data  = cast(const char *) string.data;
    return view;
}

int print1(Buffer *buffer, String_View view) {
    buffer->data  = cast(u8 *) view.data;
    buffer->count = view.count;
    return 0; // @note: 0, because we are just pointing to the actual string.
}


//
// @note: Filename helpers
//
inline String_View basepath(String_View v) {
    // skip trailing slashes
    while (v.count != 0 && (v[v.count - 1] == '/' || v[v.count - 1] == '\\')) {
        v.count -= 1;
    }
    
    while (v.count) {
        if (v[v.count - 1] == '/' || v[v.count - 1] == '\\') {
            return v;
        }
        
        v.count -= 1;
    }
    
    return v;
}

inline String_View basename(String_View v) {
    auto count = v.count;
    
    // skip trailing slashes
    size_t skip = 0;
    while (count != 0 && (v[count - 1] == '/' || v[count - 1] == '\\')) {
        count -= 1;
        skip  += 1;
    }
    
    while (count) {
        if (v[count - 1] == '/' || v[count - 1] == '\\') {
            String_View result;
            result.count = v.count - (count + skip);
            result.data  = v.data + count;
            return result;
        }
        
        count -= 1;
    }
    
    return v;
}


//
// @note: Comparison functions
//
inline bool strings_match(const String_View &a, const String_View &b) {
    if (a.count != b.count)  return false;
    return (memcmp(a.data, b.data, a.count) == 0);
}

inline bool operator==(const String_View &a, const String_View &b) {
    return strings_match(a, b);
}
inline bool operator!=(const String_View &a, const String_View &b) {
    return !strings_match(a, b);
}
// @todo


//
//~
#ifdef IML_NAMESPACE
#  if 0
{
#  endif
}
#endif

#endif//IML_STRING_VIEW_H
