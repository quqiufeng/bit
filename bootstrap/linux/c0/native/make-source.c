#ifdef MAKE_SOURCE_HOST
extern int open(const char *, int, ...);
extern long read(int, void *, unsigned long);
extern long write(int, const void *, unsigned long);
extern int close(int);
extern void *malloc(unsigned long);
extern void free(void *);
#else
extern long long __c1_linux_CreateFileA(char *, unsigned int, int, void *, int, int, long long);
extern int __c1_linux_ReadFile(long long, char *, int, int *, void *);
extern int __c1_linux_WriteFile(long long, char *, int, int *, void *);
extern int __c1_linux_CloseHandle(long long);
extern void *__c1_linux_HeapAlloc(long long, unsigned int, unsigned long long);
extern int __c1_linux_HeapFree(long long, unsigned int, void *);
#endif

typedef struct Buffer Buffer;
struct Buffer {
    char *data;
    unsigned long long length;
    unsigned long long capacity;
};

static void bytes_copy(char *to, char *from, unsigned long long count) {
    unsigned long long i;
    for (i = 0; i < count; i++)
        to[i] = from[i];
}

static void bytes_move(char *to, char *from, unsigned long long count) {
    unsigned long long i;
    if (to < from) {
        for (i = 0; i < count; i++)
            to[i] = from[i];
    } else {
        for (i = count; i > 0; i--)
            to[i - 1] = from[i - 1];
    }
}

static void *bytes_alloc(unsigned long long count) {
#ifdef MAKE_SOURCE_HOST
    return malloc((unsigned long)count);
#else
    return __c1_linux_HeapAlloc(0, 0, count);
#endif
}

static void bytes_free(void *memory) {
#ifdef MAKE_SOURCE_HOST
    free(memory);
#else
    __c1_linux_HeapFree(0, 0, memory);
#endif
}

static unsigned long long text_length(const char *text) {
    unsigned long long length = 0;
    while (text[length])
        length++;
    return length;
}

static int buffer_reserve(Buffer *buffer, unsigned long long need) {
    unsigned long long capacity;
    char *data;
    if (need <= buffer->capacity)
        return 1;
    capacity = buffer->capacity ? buffer->capacity : 65536;
    while (capacity < need) {
        if (capacity > 0x4000000000000000ULL)
            return 0;
        capacity = capacity * 2;
    }
    data = (char *)bytes_alloc(capacity);
    if (!data)
        return 0;
    if (buffer->length)
        bytes_copy(data, buffer->data, buffer->length);
    if (buffer->data)
        bytes_free(buffer->data);
    buffer->data = data;
    buffer->capacity = capacity;
    return 1;
}

static int buffer_append(Buffer *buffer, const char *text, unsigned long long count) {
    if (!buffer_reserve(buffer, buffer->length + count + 1))
        return 0;
    bytes_copy(buffer->data + buffer->length, (char *)text, count);
    buffer->length += count;
    buffer->data[buffer->length] = 0;
    return 1;
}

static int buffer_append_char(Buffer *buffer, char value) {
    if (!buffer_reserve(buffer, buffer->length + 2))
        return 0;
    buffer->data[buffer->length++] = value;
    buffer->data[buffer->length] = 0;
    return 1;
}

static int buffer_append_text(Buffer *buffer, const char *text) {
    return buffer_append(buffer, text, text_length(text));
}

static int buffer_replace(Buffer *buffer, unsigned long long start, unsigned long long count,
                          const char *text, unsigned long long text_count) {
    unsigned long long tail;
    if (start > buffer->length || count > buffer->length - start)
        return 0;
    tail = buffer->length - start - count;
    if (!buffer_reserve(buffer, buffer->length - count + text_count + 1))
        return 0;
    if (tail)
        bytes_move(buffer->data + start + text_count, buffer->data + start + count, tail);
    if (text_count)
        bytes_copy(buffer->data + start, (char *)text, text_count);
    buffer->length = buffer->length - count + text_count;
    buffer->data[buffer->length] = 0;
    return 1;
}

static long long buffer_find(Buffer *buffer, unsigned long long start, const char *text,
                             unsigned long long count) {
    unsigned long long i, j;
    if (!count)
        return (long long)start;
    if (start > buffer->length || count > buffer->length - start)
        return -1;
    for (i = start; i + count <= buffer->length; i++) {
        for (j = 0; j < count; j++)
            if (buffer->data[i + j] != text[j])
                break;
        if (j == count)
            return (long long)i;
    }
    return -1;
}

static int replace_literal(Buffer *buffer, const char *from, const char *to) {
    unsigned long long from_length = text_length(from);
    unsigned long long to_length = text_length(to);
    long long position = 0;
    while ((position = buffer_find(buffer, (unsigned long long)position, from, from_length)) >= 0) {
        if (!buffer_replace(buffer, (unsigned long long)position, from_length, to, to_length))
            return 0;
        position = position + (long long)to_length;
    }
    return 1;
}

static int is_word(char value) {
    return (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z') ||
           (value >= '0' && value <= '9') || value == '_';
}

static int is_space(char value) {
    return value == ' ' || value == '\t' || value == '\n' || value == '\r' ||
           value == '\f' || value == '\v';
}

static unsigned long long skip_space(Buffer *buffer, unsigned long long position) {
    while (position < buffer->length && is_space(buffer->data[position]))
        position++;
    return position;
}

static int match_word(Buffer *buffer, unsigned long long position, const char *word) {
    unsigned long long i;
    unsigned long long length = text_length(word);
    if (position > buffer->length || length > buffer->length - position)
        return 0;
    for (i = 0; i < length; i++)
        if (buffer->data[position + i] != word[i])
            return 0;
    if (position > 0 && is_word(buffer->data[position - 1]))
        return 0;
    if (position + length < buffer->length && is_word(buffer->data[position + length]))
        return 0;
    return 1;
}

static long long find_word_in_range(Buffer *buffer, unsigned long long start,
                                   unsigned long long end, const char *word) {
    unsigned long long length = text_length(word);
    unsigned long long i;
    if (end < start || length > end - start)
        return -1;
    for (i = start; i + length <= end; i++) {
        if (i > start && is_word(buffer->data[i - 1]))
            continue;
        if (i + length < end && is_word(buffer->data[i + length]))
            continue;
        if (buffer_find(buffer, i, word, length) == (long long)i)
            return (long long)i;
    }
    return -1;
}

static int body_replace(Buffer *buffer, const char *signature, const char *body) {
    long long found = buffer_find(buffer, 0, signature, text_length(signature));
    unsigned long long start, end;
    int depth = 0;
    if (found < 0)
        return 0;
    start = (unsigned long long)found;
    while (start < buffer->length && buffer->data[start] != '{')
        start++;
    if (start == buffer->length)
        return 0;
    end = start;
    while (end < buffer->length) {
        if (buffer->data[end] == '{')
            depth++;
        else if (buffer->data[end] == '}') {
            depth--;
            if (depth == 0)
                break;
        }
        end++;
    }
    if (end == buffer->length)
        return 0;
    return buffer_replace(buffer, start, end + 1 - start, body, text_length(body));
}

static int remove_typedef(Buffer *buffer, const char *name) {
    unsigned long long position = 0;
    long long found;
    while ((found = buffer_find(buffer, position, "typedef", 7)) >= 0) {
        unsigned long long start = (unsigned long long)found;
        unsigned long long end, semi, after;
        if (start > 0 && is_word(buffer->data[start - 1])) {
            position = start + 7;
            continue;
        }
        if (start + 7 < buffer->length && is_word(buffer->data[start + 7])) {
            position = start + 7;
            continue;
        }
        if ((found = buffer_find(buffer, start, ";", 1)) < 0)
            return 0;
        semi = (unsigned long long)found;
        end = semi;
        found = find_word_in_range(buffer, start, end, name);
        if (found >= 0) {
            after = skip_space(buffer, (unsigned long long)found + text_length(name));
            if (after < end && buffer->data[after] == '(') {
                while (end > after && is_space(buffer->data[end - 1]))
                    end--;
                if (end > after && buffer->data[end - 1] == ')') {
                    if (!buffer_replace(buffer, start, semi + 1 - start, "", 0))
                        return 0;
                    position = start;
                    continue;
                }
            }
        }
        position = semi + 1;
    }
    return 1;
}

static int remove_pointer_decl(Buffer *buffer, const char *name, const char *variable) {
    unsigned long long position = 0;
    long long found;
    while ((found = buffer_find(buffer, position, name, text_length(name))) >= 0) {
        unsigned long long start = (unsigned long long)found;
        unsigned long long cursor, semi;
        if (!match_word(buffer, start, name)) {
            position = start + 1;
            continue;
        }
        cursor = skip_space(buffer, start + text_length(name));
        if (cursor >= buffer->length || buffer->data[cursor] != '*') {
            position = start + text_length(name);
            continue;
        }
        cursor = skip_space(buffer, cursor + 1);
        if (!match_word(buffer, cursor, variable)) {
            position = start + text_length(name);
            continue;
        }
        found = buffer_find(buffer, cursor + text_length(variable), ";", 1);
        if (found < 0)
            return 0;
        semi = (unsigned long long)found;
        if (!buffer_replace(buffer, start, semi + 1 - start, "", 0))
            return 0;
        position = start;
    }
    return 1;
}

static int replace_clock_signature(Buffer *buffer) {
    long long found;
    unsigned long long position = 0;
    while ((found = buffer_find(buffer, position, "int", 3)) >= 0) {
        unsigned long long start = (unsigned long long)found;
        unsigned long long cursor;
        if (!match_word(buffer, start, "int")) {
            position = start + 1;
            continue;
        }
        cursor = skip_space(buffer, start + 3);
        if (!match_word(buffer, cursor, "pp_clock_read")) {
            position = start + 3;
            continue;
        }
        cursor = skip_space(buffer, cursor + text_length("pp_clock_read"));
        if (cursor >= buffer->length || buffer->data[cursor] != '(') {
            position = start + 3;
            continue;
        }
        cursor = skip_space(buffer, cursor + 1);
        if (!match_word(buffer, cursor, "PP_LocalTime")) {
            position = start + 3;
            continue;
        }
        cursor = skip_space(buffer, cursor + 12);
        if (cursor >= buffer->length || buffer->data[cursor] != '*') {
            position = start + 3;
            continue;
        }
        cursor = skip_space(buffer, cursor + 1);
        if (!match_word(buffer, cursor, "local_time")) {
            position = start + 3;
            continue;
        }
        cursor = skip_space(buffer, cursor + 10);
        if (cursor >= buffer->length || buffer->data[cursor] != ')') {
            position = start + 3;
            continue;
        }
        cursor++;
        if (!buffer_replace(buffer, start, cursor - start, "int pp_clock_read ( void )",
                            text_length("int pp_clock_read ( void )")))
            return 0;
        position = start + text_length("int pp_clock_read ( void )");
    }
    return 1;
}

static int replace_clock_call(Buffer *buffer) {
    long long found;
    unsigned long long position = 0;
    while ((found = buffer_find(buffer, position, "pp_clock_read", text_length("pp_clock_read"))) >= 0) {
        unsigned long long start = (unsigned long long)found;
        unsigned long long cursor;
        if (!match_word(buffer, start, "pp_clock_read")) {
            position = start + 1;
            continue;
        }
        cursor = skip_space(buffer, start + text_length("pp_clock_read"));
        if (cursor >= buffer->length || buffer->data[cursor] != '(') {
            position = start + text_length("pp_clock_read");
            continue;
        }
        cursor = skip_space(buffer, cursor + 1);
        if (cursor >= buffer->length || buffer->data[cursor] != '(') {
            position = start + text_length("pp_clock_read");
            continue;
        }
        cursor = skip_space(buffer, cursor + 1);
        if (!match_word(buffer, cursor, "PP_LocalTime")) {
            position = start + text_length("pp_clock_read");
            continue;
        }
        cursor = skip_space(buffer, cursor + 12);
        if (cursor >= buffer->length || buffer->data[cursor] != '*') {
            position = start + text_length("pp_clock_read");
            continue;
        }
        cursor = skip_space(buffer, cursor + 1);
        if (cursor >= buffer->length || buffer->data[cursor] != ')') {
            position = start + text_length("pp_clock_read");
            continue;
        }
        cursor = skip_space(buffer, cursor + 1);
        if (!match_word(buffer, cursor, "__c1_linux_GetLocalTime")) {
            position = start + text_length("pp_clock_read");
            continue;
        }
        cursor = skip_space(buffer, cursor + text_length("__c1_linux_GetLocalTime"));
        if (cursor >= buffer->length || buffer->data[cursor] != ')') {
            position = start + text_length("pp_clock_read");
            continue;
        }
        cursor++;
        if (!buffer_replace(buffer, start, cursor - start, "pp_clock_read ( )",
                            text_length("pp_clock_read ( )")))
            return 0;
        position = start + text_length("pp_clock_read ( )");
    }
    return 1;
}

static int replace_word_call(Buffer *buffer, const char *name, const char *replacement) {
    long long found;
    unsigned long long position = 0;
    while ((found = buffer_find(buffer, position, name, text_length(name))) >= 0) {
        unsigned long long start = (unsigned long long)found;
        unsigned long long cursor;
        if (!match_word(buffer, start, name)) {
            position = start + 1;
            continue;
        }
        cursor = skip_space(buffer, start + text_length(name));
        if (cursor >= buffer->length || buffer->data[cursor] != '(') {
            position = start + text_length(name);
            continue;
        }
        cursor++;
        if (!buffer_replace(buffer, start, cursor - start, replacement, text_length(replacement)))
            return 0;
        position = start + text_length(replacement);
    }
    return 1;
}

static int replace_write_call(Buffer *buffer) {
    long long found;
    unsigned long long position = 0;
    while ((found = buffer_find(buffer, position, "write", 5)) >= 0) {
        unsigned long long start = (unsigned long long)found;
        unsigned long long cursor;
        if (!match_word(buffer, start, "write")) {
            position = start + 1;
            continue;
        }
        cursor = skip_space(buffer, start + 5);
        if (cursor >= buffer->length || buffer->data[cursor] != '(') {
            position = start + 5;
            continue;
        }
        cursor = skip_space(buffer, cursor + 1);
        if (!match_word(buffer, cursor, "handle")) {
            position = start + 5;
            continue;
        }
        cursor = skip_space(buffer, cursor + 6);
        if (cursor >= buffer->length || buffer->data[cursor] != ',') {
            position = start + 5;
            continue;
        }
        cursor++;
        if (!buffer_replace(buffer, start, cursor - start,
                            "__c1_linux_WriteFile ( handle ,",
                            text_length("__c1_linux_WriteFile ( handle ,")))
            return 0;
        position = start + text_length("__c1_linux_WriteFile ( handle ,");
    }
    return 1;
}

static int replace_close_call(Buffer *buffer) {
    long long found;
    unsigned long long position = 0;
    while ((found = buffer_find(buffer, position, "close", 5)) >= 0) {
        unsigned long long start = (unsigned long long)found;
        unsigned long long cursor;
        if (!match_word(buffer, start, "close")) {
            position = start + 1;
            continue;
        }
        cursor = skip_space(buffer, start + 5);
        if (cursor >= buffer->length || buffer->data[cursor] != '(') {
            position = start + 5;
            continue;
        }
        cursor = skip_space(buffer, cursor + 1);
        if (!match_word(buffer, cursor, "handle")) {
            position = start + 5;
            continue;
        }
        cursor = skip_space(buffer, cursor + 6);
        if (cursor >= buffer->length || buffer->data[cursor] != ')') {
            position = start + 5;
            continue;
        }
        cursor++;
        if (!buffer_replace(buffer, start, cursor - start,
                            "__c1_linux_CloseHandle ( handle )",
                            text_length("__c1_linux_CloseHandle ( handle )")))
            return 0;
        position = start + text_length("__c1_linux_CloseHandle ( handle )");
    }
    return 1;
}

static int remove_assignment(Buffer *buffer, const char *name) {
    long long found;
    unsigned long long position = 0;
    while ((found = buffer_find(buffer, position, name, text_length(name))) >= 0) {
        unsigned long long start = (unsigned long long)found;
        unsigned long long cursor, semi;
        if (!match_word(buffer, start, name)) {
            position = start + 1;
            continue;
        }
        cursor = skip_space(buffer, start + text_length(name));
        if (cursor >= buffer->length || buffer->data[cursor] != '=') {
            position = start + text_length(name);
            continue;
        }
        cursor = skip_space(buffer, cursor + 1);
        if (cursor >= buffer->length || buffer->data[cursor] != '(') {
            position = start + text_length(name);
            continue;
        }
        found = buffer_find(buffer, cursor + 1, ";", 1);
        if (found < 0)
            return 0;
        semi = (unsigned long long)found;
        if (!buffer_replace(buffer, start, semi + 1 - start, "", 0))
            return 0;
        position = start;
    }
    return 1;
}

static int replace_array_postincrement(Buffer *buffer) {
    long long found;
    unsigned long long position = 0;
    while ((found = buffer_find(buffer, position, "[", 1)) >= 0) {
        unsigned long long bracket = (unsigned long long)found;
        unsigned long long start, array_end, cursor, group1_end, variable_start, variable_end;
        unsigned long long body_start, semi;
        Buffer replacement;
        replacement.data = 0;
        replacement.length = 0;
        replacement.capacity = 0;
        cursor = bracket;
        while (cursor > 0 && is_space(buffer->data[cursor - 1]))
            cursor--;
        array_end = cursor;
        while (cursor > 0 && is_word(buffer->data[cursor - 1]))
            cursor--;
        start = cursor;
        if (start == array_end || (buffer->data[start] >= '0' && buffer->data[start] <= '9')) {
            position = bracket + 1;
            continue;
        }
        cursor = skip_space(buffer, bracket + 1);
        group1_end = cursor;
        if (cursor >= buffer->length || buffer->data[cursor] != '(') {
            position = bracket + 1;
            continue;
        }
        cursor = skip_space(buffer, cursor + 1);
        if (cursor >= buffer->length || buffer->data[cursor] != '*') {
            position = bracket + 1;
            continue;
        }
        cursor = skip_space(buffer, cursor + 1);
        variable_start = cursor;
        while (cursor < buffer->length && is_word(buffer->data[cursor]))
            cursor++;
        variable_end = cursor;
        if (variable_start == variable_end) {
            position = bracket + 1;
            continue;
        }
        cursor = skip_space(buffer, cursor);
        if (cursor >= buffer->length || buffer->data[cursor] != ')') {
            position = bracket + 1;
            continue;
        }
        cursor = skip_space(buffer, cursor + 1);
        if (cursor + 1 >= buffer->length || buffer->data[cursor] != '+' ||
            buffer->data[cursor + 1] != '+') {
            position = bracket + 1;
            continue;
        }
        cursor = skip_space(buffer, cursor + 2);
        if (cursor >= buffer->length || buffer->data[cursor] != ']') {
            position = bracket + 1;
            continue;
        }
        cursor = skip_space(buffer, cursor + 1);
        if (cursor >= buffer->length || buffer->data[cursor] != '=') {
            position = bracket + 1;
            continue;
        }
        cursor = skip_space(buffer, cursor + 1);
        body_start = cursor;
        found = buffer_find(buffer, body_start, ";", 1);
        if (found < 0) {
            bytes_free(replacement.data);
            return 0;
        }
        semi = (unsigned long long)found;
        if (!buffer_append(&replacement, buffer->data + start, group1_end - start) ||
            !buffer_append_char(&replacement, '*') ||
            !buffer_append(&replacement, buffer->data + variable_start,
                           variable_end - variable_start) ||
            !buffer_append_char(&replacement, ']') ||
            !buffer_append_text(&replacement, " = ") ||
            !buffer_append(&replacement, buffer->data + body_start, semi - body_start) ||
            !buffer_append_text(&replacement, "; *") ||
            !buffer_append(&replacement, buffer->data + variable_start,
                           variable_end - variable_start) ||
            !buffer_append_text(&replacement, " = *") ||
            !buffer_append(&replacement, buffer->data + variable_start,
                           variable_end - variable_start) ||
            !buffer_append_text(&replacement, " + 1;")) {
            bytes_free(replacement.data);
            return 0;
        }
        if (!buffer_replace(buffer, start, semi + 1 - start, replacement.data,
                            replacement.length)) {
            bytes_free(replacement.data);
            return 0;
        }
        position = start + replacement.length;
        bytes_free(replacement.data);
    }
    return 1;
}

static int load_file(const char *path, Buffer *buffer) {
#ifdef MAKE_SOURCE_HOST
    char chunk[32768];
    int handle;
    long count;
    handle = open(path, 0);
    if (handle < 0)
        return 0;
    for (;;) {
        count = read(handle, chunk, 32768);
        if (count < 0) {
            close(handle);
            return 0;
        }
        if (count == 0)
            break;
        if (!buffer_append(buffer, chunk, (unsigned long long)count)) {
            close(handle);
            return 0;
        }
    }
    close(handle);
    return 1;
#else
    char chunk[32768];
    long long handle;
    int done;
    handle = __c1_linux_CreateFileA((char *)path, 0x80000000U, 7, 0, 3, 0x80, 0);
    if (handle < 0)
        return 0;
    for (;;) {
        done = 0;
        if (!__c1_linux_ReadFile(handle, chunk, 32768, &done, 0)) {
            __c1_linux_CloseHandle(handle);
            return 0;
        }
        if (done == 0)
            break;
        if (!buffer_append(buffer, chunk, (unsigned long long)done)) {
            __c1_linux_CloseHandle(handle);
            return 0;
        }
    }
    __c1_linux_CloseHandle(handle);
    return 1;
#endif
}

static int write_stdout(const char *data, unsigned long long length) {
    unsigned long long offset = 0;
#ifdef MAKE_SOURCE_HOST
    while (offset < length) {
        long count = write(1, data + offset, (unsigned long)length - offset);
        if (count <= 0)
            return 0;
        offset = offset + (unsigned long long)count;
    }
#else
    while (offset < length) {
        unsigned long long remaining = length - offset;
        int count = remaining > 65536 ? 65536 : (int)remaining;
        int done = 0;
        if (!__c1_linux_WriteFile(1, (char *)data + offset, count, &done, 0))
            return 0;
        if (done <= 0)
            return 0;
        offset = offset + (unsigned long long)done;
    }
#endif
    return 1;
}

static int transform(Buffer *buffer) {
    static const char fpe_body[] = "{ return fpe_asm ( type , op , a , b ) ; }";
    static const char wide_body[] =
        "{ unsigned long long v [ 6 ] ; v [ 0 ] = a ; v [ 1 ] = b ; v [ 2 ] = c ; "
        "v [ 3 ] = d ; v [ 4 ] = 0 ; v [ 5 ] = 0 ; sc_wide_asm ( v , t , op ) ; "
        "if ( active && ( ! fpe_finite ( t , v [ 4 ] ) || ! fpe_finite ( t , v [ 5 ] ) ) ) "
        "error_message ( \"complex constant arithmetic out of range\" ) ; "
        "sc_bits = v [ 4 ] ; sc_imaginary = v [ 5 ] ; return 0 ; }";
    static const char *typedef_names[] = {
        "DiagnosticHandle", "DiagnosticWrite", "PP_LocalTime", "PP_Open", "PP_Read",
        "PP_Close", "PP_Environment", "Lead", "Heap", "Allocate", "Release"
    };
    static const char *pointer_names[] = {
        "DiagnosticHandle", "DiagnosticWrite", "PP_Open", "PP_Environment", "PP_Read",
        "PP_Close", "Lead", "Heap", "Allocate", "Release"
    };
    static const char *pointer_variables[] = {
        "get", "write", "open", "environment", "read", "close", "lead", "heap",
        "allocate", "release"
    };
    static const char *assignment_names[] = {
        "get", "write", "open", "environment", "read", "close", "lead", "heap",
        "allocate", "release"
    };
    int i;
    if (!body_replace(buffer, "unsigned long long fpe_arithmetic", fpe_body))
        return 0;
    if (!body_replace(buffer, "int sc_complex_wide", wide_body))
        return 0;
    for (i = 0; i < 11; i++)
        if (!remove_typedef(buffer, typedef_names[i]))
            return 0;
    for (i = 0; i < 10; i++)
        if (!remove_pointer_decl(buffer, pointer_names[i], pointer_variables[i]))
            return 0;
    if (!replace_clock_signature(buffer))
        return 0;
    if (!replace_literal(buffer, "local_time ( clock_value )",
                         "__c1_linux_GetLocalTime ( clock_value )"))
        return 0;
    if (!replace_clock_call(buffer))
        return 0;
    if (!replace_literal(buffer, "lead ( 0 ,", "__c1_linux_IsDBCSLeadByteEx ( 0 ,"))
        return 0;
    if (!replace_literal(buffer, "return open (", "return __c1_linux_CreateFileA ("))
        return 0;
    if (!replace_word_call(buffer, "environment",
                           "__c1_linux_GetEnvironmentVariableA ("))
        return 0;
    if (!replace_literal(buffer, "get ( - 11 )", "__c1_linux_GetStdHandle ( - 11 )"))
        return 0;
    if (!replace_write_call(buffer))
        return 0;
    if (!replace_close_call(buffer))
        return 0;
    if (!replace_word_call(buffer, "read", "__c1_linux_ReadFile ("))
        return 0;
    if (!replace_literal(buffer, "if ( ! close ( handle ) )",
                         "if ( ! __c1_linux_CloseHandle ( handle ) )"))
        return 0;
    if (!replace_literal(buffer, "p = allocate ( heap ( ) , 0 ,",
                         "p = __c1_linux_HeapAlloc ( __c1_linux_GetProcessHeap ( ) , 0 ,"))
        return 0;
    if (!replace_literal(buffer, "release ( heap ( ) , 0 , p )",
                         "__c1_linux_HeapFree ( __c1_linux_GetProcessHeap ( ) , 0 , p )"))
        return 0;
    for (i = 0; i < 10; i++)
        if (!remove_assignment(buffer, assignment_names[i]))
            return 0;
    if (!replace_literal(buffer, "return names [ id * 256 + ( * at ) ++ ] ;",
                         "{ int _idx = * at ; * at = _idx + 1 ; "
                         "return names [ id * 256 + _idx ] ; }"))
        return 0;
    if (!replace_array_postincrement(buffer))
        return 0;
    return 1;
}

int main(int argc, char **argv) {
    Buffer buffer;
    int result;
    if (argc != 2)
        return 2;
    buffer.data = 0;
    buffer.length = 0;
    buffer.capacity = 0;
    if (!load_file(argv[1], &buffer))
        return 1;
    if (!transform(&buffer))
        return 1;
    if (!write_stdout(buffer.data, buffer.length))
        return 1;
    bytes_free(buffer.data);
    result = 0;
    return result;
}
