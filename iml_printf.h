#if !defined(IML_PRINTF_H)
#define IML_PRINTF_H


#include "iml_types.h"


//
// @note: Print error
//
internal void print_format_string_escaped_with_caret(const char *format, const char *start, const char *end) {
    fprintf(stderr, "    Format string: '");
    smm space_count = 0;
    const char *temp_cursor = format;
    while (*temp_cursor && temp_cursor < end) {
        smm temp_space_count = space_count;
        
        if (*temp_cursor == '\\')  fprintf(stderr, "\\\\");
        else if (*temp_cursor == '\r')  fprintf(stderr, "\\r");
        else if (*temp_cursor == '\n')  fprintf(stderr, "\\n");
        else if (*temp_cursor == '\t')  fprintf(stderr, "\\t");
        else if (*temp_cursor == '\v')  fprintf(stderr, "\\v");
        else if (*temp_cursor == '\f')  fprintf(stderr, "\\f");
        else if (*temp_cursor == '\a')  fprintf(stderr, "\\a");
        else if (*temp_cursor == '\b')  fprintf(stderr, "\\b");
        else if (*temp_cursor == '\'')  fprintf(stderr, "\\'");
        else if (*temp_cursor == '\"')  fprintf(stderr, "\\\"");
        else {
            fprintf(stderr, "%c", *temp_cursor);
            temp_space_count += 1;
            goto loop_continue;
        }
        temp_space_count += 2;
        
        loop_continue:;
        if (temp_cursor <= start) {
            space_count = temp_space_count;
        }
        
        temp_cursor += 1;
    }
    space_count -= 1;
    fprintf(stderr, "'\n");
    fprintf(stderr, "                    ");
    for (smm i = 0; i < space_count; ++i) {
        fprintf(stderr, "%c", ' ');
    }
    smm length = end - start;
    for (smm i = 0; i < length; ++i) {
        fprintf(stderr, "%c", '^');
    }
    fprintf(stderr, "\n");
}
inline void print_format_string_escaped_with_caret(const char *format, const char *cursor) {
    print_format_string_escaped_with_caret(format, cursor, cursor);
}


//
// @note: iml_print
//
// @note These versions of print are only possible with cpp template packed parameters and folding expressions.
//       Because when using va_list to get one argument, we would have to know the type of it, to do 'va_arg(args, int);'
//

//
// @note iml_printf (snprintf)
//

int print1(Buffer *buffer, s64 x) {
    s64_to_string(buffer, x, 10, decimal_chars);
    return cast(int)buffer->count;
}
int print1(Buffer *buffer, s32 x) {
    return print1(buffer, cast(s64) x);
}
int print1(Buffer *buffer, s16 x) {
    return print1(buffer, cast(s64) x);
}
int print1(Buffer *buffer, s8 x) {
    return print1(buffer, cast(s64) x);
}

int print1(Buffer *buffer, u64 x) {
    u64_to_string(buffer, x, 10, decimal_chars);
    return cast(int)buffer->count;
}
int print1(Buffer *buffer, u32 x) {
    return print1(buffer, cast(u64) x);
}
int print1(Buffer *buffer, u16 x) {
    return print1(buffer, cast(u64) x);
}
int print1(Buffer *buffer, u8 x) {
    return print1(buffer, cast(u64) x);
}

int print1(Buffer *buffer, f64 x) {
    f64_to_string(buffer, cast(f64)x);
    return cast(int)buffer->count;
}
int print1(Buffer *buffer, f32 x) {
    return print1(buffer, cast(f64) x);
}

int print1(Buffer *buffer, char c) {
    if (buffer->count > 0) {
        buffer->data[0] = c;
    }
    return 1;
}
int print1(Buffer *buffer, const char *cstr) {
    buffer->data = cast(u8 *)cstr;
    buffer->count = string_length(cstr);
    return 0; // @note: 0, because we are just pointing to the actual string.
}
int print1(Buffer *buffer, String string) {
    buffer->data  = string.data;
    buffer->count = string.count;
    return 0; // @note: 0, because we are just pointing to the actual string.
}

struct Pad {
    int n;
    char c = ' ';
};
int print1(Buffer *buffer, Pad pad) {
    int i;
    for (i = 0; i < buffer->count && i < pad.n; ++i) {
        buffer->data[i] = pad.c;
    }
    return i;
}

// @todo: Different integer sizes.
// @todo: Optionall fill hex with zeros.
struct Format_Int {
    bool is_signed;
    s64 value;
    u32 base = 10;
    const char *digit_chars = decimal_chars;
};
Format_Int format_int(s64 value, u32 base = 10, bool upper_case = false, const char *digit_chars = decimal_chars) {
    Format_Int formatter;
    formatter.is_signed = (value < 0);
    formatter.value     = value;
    formatter.base      = base;
    if (formatter.base == 16) {
        formatter.is_signed = false;
        if (upper_case) {
            formatter.digit_chars = uppercase_hex_chars;
        }
        else {
            formatter.digit_chars = lowercase_hex_chars;
        }
    }
    else {
        formatter.digit_chars = digit_chars;
    }
    return formatter;
}
Format_Int format_uint(u64 value, u32 base = 10, bool upper_case = false, const char *digit_chars = decimal_chars) {
    Format_Int formatter = format_int(value, base, upper_case, digit_chars);
    formatter.is_signed = false;
    return formatter;
}
int print1(Buffer *buffer, Format_Int formatter) {
    if (formatter.is_signed) {
        s64_to_string(buffer, formatter.value, formatter.base, formatter.digit_chars);
    }
    else {
        u64_to_string(buffer, formatter.value, formatter.base, formatter.digit_chars);
    }
    return cast(int)buffer->count;
}

struct Format_Float {
    f64 value;
    s32 precision = 6;
};
Format_Float format_float(double value, s32 precision = 6) {
    Format_Float formatter;
    formatter.value = cast(f64)value;
    formatter.precision = precision;
    return formatter;
}
Format_Float format_float(float value, s32 precision = 6) {
    return format_float(cast(double)value, precision);
}
int print1(Buffer *buffer, Format_Float formatter) {
    f64_to_string(buffer, formatter.value, formatter.precision);
    return cast(int)buffer->count;
}


//
// @note: iml_snprintf
//

template<typename T>
int iml_snprintf_one_inner(Buffer *dest, Buffer **arg_string_cursor, T *arg) {
    Buffer *arg_string = *arg_string_cursor;
    *arg_string = *dest;
    
    // @note Input:  arg_string = Buffer with a cursor to write to and a count of the max size able to write.
    //       Output: arg_string = Buffer with the start cursor and a count of the chars written.
    //       Return: size of bytes to advance the actual buffer.
    // @note: Return value can be zero if the data pointer in arg_string is pointing to different memory
    //        and we are not writing to the buffer passed to it.
    //        Return value is zero or bigger than the count in arg_string if an error occured.
    int arg_string_size = print1(arg_string, *arg);
    
    assert(arg_string_size >= 0, "snprintf encoding error!");
    assert(arg_string_size < dest->count, "Buffer is too small to fit all argument strings!");
    if (arg_string_size > 0 && arg_string_size < dest->count) {
        // @note: Not needed because we are using Buffer internally.
        //arg_string_size += 1; // Include the null-terminator
        arg_string->count = arg_string_size;
        dest->data  += arg_string_size;
        dest->count -= arg_string_size;
    }
    
    *arg_string_cursor += 1;
    
    return arg_string_size;
}

#if 0

// 0, %=37, \\=92
global_const u8 snprintf_loop_lookup_table[256] = {
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

// @note: Size returned _does_not_ include the null terminator.
// @note: Currently we are not at all null terminating the string. :null_terminator
inline String iml_snprintf_inner(Buffer *dest, const int arg_count, int *arg_reference_counts, Buffer *arg_strings, const char *format) {
    String result = *dest;
    
    const char *cursor = format;
    while (*cursor != '\0') {
        // @note: I knew this would be faster (~1.22x), but only as we changed put_chars to use memcpy. Before with have done put_char in the if's below.
        {
            Buffer temp;
            temp.data  = cast(u8 *)cursor;
            temp.count = 0;
#if 0
            for (; !(*cursor == '\0' || *cursor == '%' || *cursor == '\\'); ++temp.count) {
                cursor += 1;
            }
#else
            // @note Use lookup table (~1.07x faster)
            while (snprintf_loop_lookup_table[*cursor]) {
                cursor += 1;
            }
            temp.count = cast(u8 *)cursor - temp.data;
#endif
            put_chars(dest, temp);
        }
        
        // @note: The loop above already checked for % and \\, but it's a bit faster this way,
        //        because the loop above is more compact.
        if (*cursor == '%') {
#if BUILD_DEBUG
            const char *debug_argument_start = cursor;
#endif
            cursor += 1;
            
            // @note: Argument selection, "%2"
            s32 arg_index = arg_count; // arg_count will be an error, if we cant find a valid index.
            if (*cursor >= '0' && *cursor <= '9') {
                arg_index = s32_from_string_cursor(&cursor);
            }
            else {
                // @note: Use the first not referenced argument.
                for (int i = 0; i < arg_count; ++i) {
                    if (arg_reference_counts[i] == 0) {
                        arg_index = i;
                        break;
                    }
                }
            }
            
            assert(arg_index >= 0);
            if (arg_index >= arg_count) {
                fprintf(stderr, "Error: Not enough arguments! Argument index is [%d], but argument count is only [%d]!\n", arg_index, arg_count);
#if BUILD_DEBUG
                print_format_string_escaped_with_caret(format, debug_argument_start, cursor);
#else
                print_format_string_escaped_with_caret(format, cursor);
#endif
                put_char(dest, '\n');
                break;
            }
            
            arg_reference_counts[arg_index] = true;
            put_chars(dest, arg_strings[arg_index]);
        }
        else if (*cursor == '\\') {
            if (cursor[1] != '\0' && cursor[1] == '%') {
                put_char(dest, '%');
                cursor += 2;
            }
            else {
                put_char(dest, '\\');
                cursor += 1;
            }
        }
    }
    // :null_terminator
    //assert(dest->count > 0, "Error: Not enough memory in the internal buffer of snprintf!\n");
    //if (dest->count > 0)  *dest->data++ = '\0';
    
    for (int i = 0; i < arg_count; ++i) {
        if (arg_reference_counts[i] == 0) {
            fprintf(stderr, "Error: Too many arguments! Argument at index [%d] is not referenced in the format string.\n", i);
        }
    }
    
    result.count = (result.count - dest->count);
    return result;
}

#else

// @note: Size returned _does_not_ include the null terminator.
// @note: Currently we are not at all null terminating the string. :null_terminator
inline String iml_snprintf_inner(Buffer *dest, const int arg_count, int *arg_reference_counts, Buffer *arg_strings, const char *format) {
    String result = *dest;
    
    Buffer temp;
    const char *cursor = format;
    while (1) {
        temp.data  = cast(u8 *)cursor;
        temp.count = 0;
        
#define DO_PUT_TEMP() \
temp.count = cast(u8 *)cursor - temp.data; \
put_chars(dest, temp);
        
        while (1) {
#if COMPILER_CLANG || COMPILER_GNUC
            // 0, %=37, \\=92
#if 0
            global_const u8 char_lookup_table[256] = {
                1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            };
            local_const void *lookup_table[] = {
                &&keep_looping,
                &&outer_loop_end,
                &&do_percent,
                &&do_escape,
            };
            goto *lookup_table[char_lookup_table[*cursor]];
#else
            global_const void *lookup_table[256] = {
                &&outer_loop_end, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping,
                &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&do_percent, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping,
                &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&do_escape, &&keep_looping, &&keep_looping, &&keep_looping,
                &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping,
                &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping,
                &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping,
                &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping,
                &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping, &&keep_looping,
            };
            goto *lookup_table[*cursor];
#endif
            keep_looping:;
#else
            switch (*cursor) {
                case '\0': goto outer_loop_end;
                case '%':  goto do_percent;
                case '\\': goto do_escape;
            }
#endif
            
            cursor += 1;
        }
        
        continue;
        
        while (0) {
            do_percent:;
            DO_PUT_TEMP();
            
            
#if BUILD_DEBUG
            const char *debug_argument_start = cursor;
#endif
            cursor += 1;
            
            // @note: Argument selection, "%2"
            s32 arg_index = arg_count; // arg_count will be an error, if we cant find a valid index.
            if (*cursor >= '0' && *cursor <= '9') {
                arg_index = s32_from_string_cursor(&cursor);
            }
            else {
                // @note: Use the first not referenced argument.
                for (int i = 0; i < arg_count; ++i) {
                    if (arg_reference_counts[i] == 0) {
                        arg_index = i;
                        break;
                    }
                }
            }
            
            assert(arg_index >= 0);
            if (arg_index >= arg_count) {
                fprintf(stderr, "Error: Not enough arguments! Argument index is [%d], but argument count is only [%d]!\n", arg_index, arg_count);
#if BUILD_DEBUG
                print_format_string_escaped_with_caret(format, debug_argument_start, cursor);
#else
                print_format_string_escaped_with_caret(format, cursor);
#endif
                put_char(dest, '\n');
                goto error_end;
            }
            
            arg_reference_counts[arg_index] = true;
            put_chars(dest, arg_strings[arg_index]);
        }
        
        continue;
        
        while (0) {
            do_escape:;
            DO_PUT_TEMP();
            
            
            if (cursor[1] != '\0' && cursor[1] == '%') {
                put_char(dest, '%');
                cursor += 2;
            }
            else {
                put_char(dest, '\\');
                cursor += 1;
            }
        }
    }
    outer_loop_end:;
    DO_PUT_TEMP();
    error_end:;
    
    // :null_terminator
    //assert(dest->count > 0, "Error: Not enough memory in the internal buffer of snprintf!\n");
    //if (dest->count > 0)  *dest->data++ = '\0';
    
    for (int i = 0; i < arg_count; ++i) {
        if (arg_reference_counts[i] == 0) {
            fprintf(stderr, "Error: Too many arguments! Argument at index [%d] is not referenced in the format string.\n", i);
        }
    }
    
    result.count = (result.count - dest->count);
    return result;
}
#endif


//
// @thread_safety
// @todo: static memory is probably not thread safe, because we are ofc writing to it. But it's faster.
//        So maybe just but a if def around it and specify if you want that.
//

// @todo If we need it, but I don't like having more template arguments. <const umm buffer_size = 4096,
template<typename... T>
String iml_snprintf(Buffer *output_buffer, const char *format, T... args) {
    const int arg_count = sizeof...(args);
    
    local_persist u8 char_buffer[4096]; // @thread_safety
    Buffer arg_buffer = { array_count(char_buffer), char_buffer };
    
    const int temp_arg_count = arg_count + 1; // Because if we have no args we can't allocate an array of constant size 0!
    local_persist Buffer arg_strings[temp_arg_count]; // @thread_safety
    local_persist int arg_reference_counts[temp_arg_count]; // @thread_safety
    memset(arg_reference_counts, 0, sizeof(int) * arg_count);
    
    Buffer arg_buffer_cursor = arg_buffer;
    Buffer *arg_string_cursor = arg_strings;
    (iml_snprintf_one_inner(&arg_buffer_cursor, &arg_string_cursor, &args), ...);
    
    String result = iml_snprintf_inner(output_buffer, arg_count, arg_reference_counts, arg_strings, format);
    return result;
}


//
// @note: iml_printf / iml_fprintf
//

// @note: Interestingly, using a template here is a 8x compile-time speedup, compared to using the macro below.
template<typename... T>
void iml_fprintf(File_Handle *file, const char *format, T... args) {
    local_persist u8 output_buffer[4096]; // @thread_safety
    Buffer output = { array_count(output_buffer), output_buffer };
    String result = iml_snprintf(&output, format, args...);
    os_write_file(file, result.count, result.data);
}


//
// @note print/fprint and println/fprintln
//
template<typename T>
void snprint_one_inner(Buffer *dest, T arg) {
    Buffer arg_string = *dest;
    Buffer *temp_buffer_cursor = &arg_string;
    
    int arg_string_size = iml_snprintf_one_inner(dest, &temp_buffer_cursor, &arg);
    
    if (arg_string_size == 0) {
        assert(arg_string.data != dest->data);
        memcpy(dest->data, arg_string.data, arg_string.count);
        dest->data  += arg_string.count;
        dest->count -= arg_string.count;
    }
}

template<typename... T>
void snprint(Buffer *dest, T... args) {
    (snprint_one_inner(dest, args), ...);
}

template<typename... T>
void fprint(File_Handle *file, T... args) {
    const smm count = 4096;
    local_persist char output_buffer[count]; // @thread_safety
    Buffer output = { count, cast(u8 *)output_buffer };
    
    (snprint_one_inner(&output, args), ...);
    
    assert(output.count > 0, "\nError: Not enough memory in the buffer for fprint!\n\n");
    smm write_count = count - output.count;
    os_write_file(file, write_count, output_buffer);
}
template<typename... T>
void fprint(FILE *file, T... args) {
    const smm count = 4096;
    local_persist char output_buffer[count]; // @thread_safety
    Buffer output = { count, cast(u8 *)output_buffer };
    
    (snprint_one_inner(&output, args), ...);
    
    assert(output.count > 0, "\nError: Not enough memory in the buffer for fprint!\n\n");
    
    smm write_count = count - output.count;
    fprintf(file, "%.*s", (int)write_count, (char *)output_buffer);
}
template<typename... T>
inline void print(T... args) {
    fprint(stdout, args...);
}

template <typename... T>
void fprintln(File_Handle file, T... args) {
    fprint(file, args..., '\n');
}
template <typename... T>
inline void println(T... args) {
    fprintln(stdout, args...);
}


//
// @note print_list / fprint_list
//
// @important: These macros insert a local_persist u8 buffer[4096];
//
// Usage:
#if 0
printf_list(iml_snprintf(&output, "%Hello % Sailor!\n",  Pad{4}, 10),
            "----\n",
            iml_snprintf(&output, "%Second string %!\n", Pad{4}, 2));
fprintf__list(stdout, o,
              iml_snprintf(&o, "%Hello % Sailor!\n",  Pad{4}, 10),
              "----\n",
              iml_snprintf(&o, "%Second string %!\n", Pad{4}, 2));
#endif
//

#define fprintf__list(stream, output, ...) do { \
local_persist u8 output##_memory[4096]; /* @thread_safety */ \
Buffer output = { array_count(output##_memory), output##_memory }; \
fprint(stream, __VA_ARGS__); \
} while (0)
#define fprintf_list(stream, ...)  fprintf__list(stream, output, __VA_ARGS__)

#define printf__list(output, ...)  fprintf__list(stdout, output, __VA_ARGS__)
#define printf_list(...)  printf__list(output, __VA_ARGS__)


#endif // IML_PRINTF_H
