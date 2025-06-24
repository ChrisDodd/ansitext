#include "layout_string.h"

VECTOR(struct line) layout_string(const char *s, unsigned width, unsigned height) {
    VECTOR(struct line) rv;
    VECTOR_init(rv);
    while (*s && VECTOR_size(rv) < height) {
        const char *last = 0;
        struct line current = { s, 0, 0 }, prefix = { 0, 0, 0 };
        while (current.width <= width) {
            last = s;
            if (*s == ' ') {
                if (current.width > 0) {
                    if (s[-1] != ' ') prefix = current;
                    current.width++;
                } else {
                    current.start++; }
            } else if (*s == '\n') {
                if (current.width > 0 && s[-1] != ' ') prefix = current;
                ++s;
                break;
            } else if (*s == 0) {
                if (current.width > 0 && s[-1] != ' ') prefix = current;
                break;
            } else if (*s == '\x1b' && s[1] == '[') {
                ++s;
                while (*++s == ';' || (*s >= '0' && *s <= '9'));
            } else {
                current.width++;
                if (*s & 0x80) {
                    int len = *s << 1;
                    while (len & 0x80 && (s[1] & 0xc0) == 0x80) {
                        len <<= 1;
                        ++s; } } } 
            current.size = ++s - current.start; }
        if (prefix.start) {
            if (current.width > width) s = prefix.start + prefix.size;
            current = prefix;
        } else if (current.width > width) {
            current.width--;
            current.size = last - current.start;
            s = current.start + current.size; }
        VECTOR_push(rv, current); }
    return rv;
}
