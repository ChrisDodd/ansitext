#include "utf8.h"

void quote_utf8(const char *s, FILE *fp) {
    for (; *s; ++s) {
        if ((*s & 0xc0) == 0xc0 && (s[1] & 0xc0) == 0x80) {
            int sz = 1, ok = 1;
            int szbit = 0x20;
            unsigned val = s[1] & 0x3f;
            while (*s && szbit) {
                ok &= (s[++sz] & 0xc0) == 0x80;
                val = (val << 6) | (s[sz] & 0x3f);
                szbit >>= 1; }
            if (ok) {
                val |= (*s & (szbit-1)) << (sz*6);
                if (val > 0xffff)
                    fprintf(fp, "\\U%08x", val);
                else
                    fprintf(fp, "\\u%04x", val);
                s += sz;
                continue; } }
        if (*s < ' ' || *s == '"' || *s == '\\' || *s >= 127) {
            putc('\\', fp);
            switch (*s) {
            case '"':  putc('"', fp);  break;
            case '\\': putc('\\', fp); break;
            case '\a': putc('a', fp);  break;
            case '\b': putc('b', fp);  break;
            case '\f': putc('f', fp);  break;
            case '\n': putc('n', fp);  break;
            case '\r': putc('r', fp);  break;
            case '\t': putc('t', fp);  break;
            case '\v': putc('v', fp);  break;
            default:
                fprintf(fp, "x%02x", *s & 0xff);
                break; }
            continue; }
        putc(*s, fp); }
}

VECTOR(char) unquote_utf8(FILE *fp, int term) {
    VECTOR(char) rv = 0;
    int ch;
    while ((ch = getc(fp)) != EOF && ch != term) {
        if (ch != '\\') {
            VECTOR_add(rv, ch);
            continue; }
        int xchars = 0, shift = 4;
        unsigned val = 0;
        switch((ch = getc(fp))) {
        case EOF:  return rv;
        case 'a': ch = '\a';   break;
        case 'b': ch = '\b';   break;
        case 'e': ch = '\x1b'; break;
        case 'f': ch = '\f';   break;
        case 'n': ch = '\n';   break;
        case 'r': ch = '\r';   break;
        case 't': ch = '\t';   break;
        case 'v': ch = '\v';   break;
        case 'x': xchars = 2;  break;
        case 'u': xchars = 4;  break;
        case 'U': xchars = 8;  break;
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
            val = ch - '0';
            xchars = 2;
            shift = 3;
            break;
        default: break; }
        if (xchars == 0) {
            VECTOR_add(rv, ch);
            continue; }
        for (int i = 0; i < xchars; ++i) {
            ch = getc(fp);
            if (ch >= '0' && ch <= '1' + 2*shift) {
                ch -= '0';
            } else if (shift == 4 && ch >= 'a' && ch <= 'f') {
                ch -= 'a' - 10;
            } else if (shift == 4 && ch >= 'A' && ch <= 'F') {
                ch -= 'A' - 10;
            } else {
                ungetc(ch, fp);
                break; }
            val = (val << shift) + ch; }
        if (xchars == 2 || val < 0x80) {
            VECTOR_add(rv, val);
            continue; }
        int size = 2;
        while (val >= 1U << ((5*size) + 1)) ++size;
        unsigned mask = 0xff & ~(0xff >> size);
        while (--size >= 0) {
            VECTOR_add(rv, mask | ((val >> (size*6)) & 0x3f));
            mask = 0x80; } }
    return rv;
}
