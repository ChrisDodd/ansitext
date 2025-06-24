#include <stdio.h>
#include "layout_string.h"

void quote_string(const char *s) {
    for (; *s; ++s) {
        if (*s < ' ') {
            putchar('\\');
            switch (*s) {
            case '\a': putchar('a'); break;
            case '\b': putchar('b'); break;
            case '\f': putchar('f'); break;
            case '\n': putchar('n'); break;
            case '\r': putchar('r'); break;
            case '\t': putchar('t'); break;
            case '\v': putchar('v'); break;
            default:
                printf("x%02x", *s & 0xff);
                break; }
        } else {
            if (*s == '\"' || *s == '\\')
                putchar('\\');
            putchar(*s); } }
}

void test_layout(const char *s) {
    printf("test_layout(\"");
    quote_string(s);
    printf("\")\n");
    for (unsigned i = 1; i < 8; i = 2*i + 1) {
        VECTOR(struct line) fmt = layout_string(s, i, i);
        for (unsigned j = 0; j < VECTOR_size(fmt); ++j)
            printf("|%.*s|\n", fmt[j].size, fmt[j].start);
        VECTOR_fini(fmt);
        printf("-------\n");
    }
}

int main() {
    //test_layout("abcdefghijklmnopqrstuvwxyz0123456789");
    //test_layout("abc def ghij klmn pqrst    ");
    //test_layout("  abc  def  ghij  klmn  pqrst    ");
    test_layout("ab\ncd ef \ngh ij kl");
}
