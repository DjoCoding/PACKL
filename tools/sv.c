#include "sv.h"

String_View sv_from_cstr(char *s) {
    String_View sv = {0};
    sv.content = s;
    if (s) sv.count = strlen(s);
    return sv;
}

char *cstr_from_sv(String_View s) {
    if (!(s.content && s.count)) { return NULL; }
    char *content = (char *)malloc(sizeof(char) * (s.count + 1));
    if (!content) THROW_ERROR("`cstr_from_sv` failed, Couldn't allocate memory for the string\n");
    memcpy(content, s.content, s.count * sizeof(char));
    content[s.count] = '\0';
    return content;
}

String_View sv_from_file(char *filename) {
    if (!filename) THROW_ERROR("file name not provided\n");
    
    FILE *f = fopen(filename, "r");
    if (!f) THROW_ERROR("could not open the file %s\n", filename);

    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *fcontent = (char *)malloc(sizeof(char) * (fsize + 1));
    fread(fcontent, sizeof(char), fsize, f);

    fcontent[fsize] = '\0';

    fclose(f);

    return sv_from_cstr(fcontent);
}

String_View sv_trim_left(String_View s) {
    while (s.count) {
        if (isspace(*s.content)) { s.content++; s.count--; }
        else { break; }
    }

    return s;
}

String_View sv_trim_right(String_View s) {
    while (s.count) {
        if (isspace(*(s.content + s.count - 1))) { s.count--; } 
        else { break; }
    }
    return s;
}

String_View sv_trim(String_View s) {
    return sv_trim_right(sv_trim_left(s));
} 

int sv_eq(String_View s, String_View t) {
    if (s.count != t.count) { return 0; }
    return memcmp(s.content, t.content, s.count) == 0;
}

String_View sv_from_char(char c) {
    char *s = (char *)malloc(sizeof(char));
    *s = c;
    return SV_GET(s, 1);
}

bool sv_empty(String_View s) {
    return (s.count == 0 || !s.content);
}

char is_after_escape(char ch) {
    switch (ch) {
        case 'a':  
            return '\a';
        case 'b': 
            return '\b';
        case 'f':  
            return '\f';
        case 'n': 
            return '\n';
        case 'r':  
            return '\r';
        case 't':  
            return '\t';
        case 'v':  
            return '\v';
        case '\\': 
            return '\\';
        case '\'':  
            return '\'';
        case '"':  
            return '\"';
        case '?':  
            return '\?';
        case '0':
            return '\0';
        default:
            return 0;
    }
}

bool is_escape_character(char ch) {
    switch (ch) {
        case '\a':  
        case '\b': 
        case '\f':  
        case '\n': 
        case '\r':  
        case '\t':  
        case '\v':  
        case '\\': 
        case '\'':  
        case '\"':  
        case '\?':  
        case '\0':
            return true;
        default:
            return false;
    }
}

char escape_to_char(char ch) {
    switch (ch) {
        case '\a':  
            return 'a';
        case '\b':
            return 'b'; 
        case '\f':
            return 'f';
        case '\n':
            return 'n'; 
        case '\r':
            return 'r'; 
        case '\t':
            return 't'; 
        case '\v':
            return 'v'; 
        case '\\':
            return '\\';
        case '\'':
            return '\''; 
        case '\"':
            return '"'; 
        case '\?':
            return '?'; 
        case '\0':
            return '0';
        default:
            ASSERT(false, "unreachable");
    }
}

char *unescape_string(char *s, size_t s_size) {
    char result[1024];
    size_t size = 0;

    for (size_t i = 0; i < s_size; ++i) {
        if (is_escape_character(s[i])) {
            result[size++] = '\\';
            result[size++] = escape_to_char(s[i]);
        } else { result[size++] = s[i]; }
    }

    char *content = malloc(sizeof(char) *(size + 1));
    memcpy(content, result, size);
    content[size] = 0;

    return content;
}

String_View unescape_string_to_sv(String_View s) {
    char *string = unescape_string(s.content, s.count);
    return SV(string);
}

char *sv_escape(String_View s) {
    char result[1024];
    size_t size = 0;
    size_t i = 0;

    while (i < s.count) {
        if (sv_at(s, i) == '\\') {
            char c = sv_at(s, i + 1);

            if (c == EOF) { result[size++] = sv_at(s, i); break; }

            if ((c = is_after_escape(c)) != 0) {    
                result[size++] = c;
                i++;
            } else {
                result[size++] = sv_at(s, i);
            }
        } else { result[size++] = sv_at(s, i); }
        i++;
    }

    char *content = malloc(sizeof(char) * (size + 1));
    memcpy(content, result, size);
    content[size] = 0;
    return content;
}

char sv_at(String_View s, size_t index) {
    if (index >= s.count) { return EOF; }
    return s.content[index];
}

int64_t sv_find_index(String_View s, char c) {
    for (size_t i = 0; i < s.count; ++i) {
        if (s.content[i] == c) { return i; }
    }
    return -1;
}

size_t sv_count_char(String_View s, char c) {
    size_t result = 0;
    for(size_t i = 0; i < s.count; ++i) {
        if (sv_at(s, i) == c) { result++; } 
    }
    return result;
}

bool sv_starts_with(String_View s, char c) {
    if (sv_empty(s)) return false;
    return (s.content[0] == c);
}

bool sv_ends_with(String_View s, String_View t) {
    if (sv_empty(t)) { return true; }
    if (s.count < t.count) { return false; }

    String_View view = SV_GET(s.content + s.count - t.count, t.count);
    return sv_eq(view, t);
}

String_View sv_chop_left(String_View s) {
    if (sv_empty(s)) { return SV_NULL; }
    return SV_GET(s.content + 1, s.count - 1);
}

String_View sv_chop_right(String_View s) {
    if (sv_empty(s)) { return SV_NULL; }
    return SV_GET(s.content, s.count - 1);
}

String_View sv_chop_both(String_View s) {
    return sv_chop_left(sv_chop_right(s));
}

String_View sv_get_before(String_View s, size_t index) {
    if(sv_empty(s) || index >= s.count) { return SV_NULL; }
    return SV_GET(s.content, index);
}

String_View sv_get_after(String_View s, size_t index) {
    if(sv_empty(s) || index >= s.count) { return SV_NULL; }

    return SV_GET(s.content + index + 1, s.count - index - 1);
}

bool sv_is_unsigned(String_View s) {
    if(sv_empty(s)) { return false; }
    for (size_t i = 0; i < s.count; ++i) {
        if (!isdigit(sv_at(s, i))) { return false; }
    }
    return true;
}

bool sv_is_integer(String_View s) {
    if (sv_empty(s)) return false;

    size_t i = 0;
    if (sv_starts_with(s, '-')) { ++i; }

    String_View abs = SV_GET(s.content + i, s.count - i);
    
    return sv_is_unsigned(abs);
}

bool sv_is_float(String_View s) {
    if (sv_empty(s)) return false;

    if (sv_count_char(s, '.') > 1) { return false; }

    int64_t index = sv_find_index(s, '.');

    if (index == -1) { return sv_is_integer(s); }

    String_View before_point = sv_get_before(s, index);
    String_View after_point = sv_get_after(s, index);

    return sv_is_integer(before_point) && sv_is_unsigned(after_point);
}

uint64_t sv_parse_uint(String_View s) {
    uint64_t result = 0;
    for (size_t i = 0; i < s.count; ++i) {
        result *= 10;
        result += sv_at(s, i) - '0';
    }
    return result;
}

bool sv_parse_integer(String_View s, int64_t *result) { 
    if (!sv_is_integer(s)) { return true; }

    String_View abs = s;
    if (sv_starts_with(s, '-')) { abs = sv_get_after(s, 0); }

    uint64_t unsigned_int = sv_parse_uint(abs);

    *result = (int64_t) unsigned_int;

    if (sv_starts_with(s, '-')) { *result = -(*result); }
    return true;
}

/* Helper function */
double get_float(bool is_signed, uint64_t int_part, uint64_t float_part, size_t precision) {
    uint64_t power = 1;
    for (size_t i = 0; i < precision; ++i) {
        power *= 10;
    }

    double float_part_to_double = (double)float_part / power;
    double abs =  (double)int_part + float_part_to_double;

    if (!is_signed) { return abs; }
    return -abs;
}

bool sv_parse_float(String_View s, double *result) {
    if (!sv_is_float(s)) { return false; }

    String_View abs = s;
    if (sv_starts_with(s, '-')) { abs = sv_get_after(s, 0); }

    int64_t index = sv_find_index(abs, '.');

    if (index < 0) {
        *result = (double)sv_parse_uint(abs);
        if (sv_starts_with(s, '-')) { *result = -(*result); }
        return true;
    }

    String_View before_point = sv_get_before(abs, index);
    String_View after_point = sv_get_after(abs, index);

    uint64_t unsigned_before = sv_parse_uint(before_point);
    uint64_t unsigned_after  = sv_parse_uint(after_point);



    *result = get_float(sv_starts_with(s, '-'), unsigned_before, unsigned_after, after_point.count);

    return true;
}

String_Slices sv_split_by_delim(String_View s, int (*compare)(int)) {
    String_Slices slices;
    DA_INIT(&slices, sizeof(String_View));
    
    size_t begin = 0;
    size_t size = 0;

    String_View slice = SV_NULL;
    
    for (size_t i = 0; i < s.count; ++i) {
        if (compare(sv_at(s, i))) {
            if (size != 0) {
                slice = SV_GET(s.content + begin, size);
                begin += size + 1;
                size = 0;
                DA_APPEND(&slices, slice);
            } else {
                begin += 1;
            }
        } else { size++; }
    }

    if (size != 0) {
        slice = SV_GET(s.content + begin, size);
        DA_APPEND(&slices, slice);
    }

    return slices;
}

int isnewline(int c) {
    return c == '\n';
}

String_Slices sv_get_lines(String_View s) {
    return sv_split_by_delim(s, isnewline);
}

String_Slices sv_split(String_View s) {
    return sv_split_by_delim(s, isspace);
}