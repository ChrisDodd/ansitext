#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "ansi.h"

struct terminal_t {
    char        *buffer, *buffer_end, *input, *input_end;
    int         fd;
    int termios_change;
    struct termios   saved_term;
    struct termios   current_term;
} stdin_term;

static enum keycode num_tilde[] = {
    0, KEY_HOME, KEY_INSERT, KEY_DELETE, KEY_END, KEY_PGUP, KEY_PGDN, KEY_HOME, KEY_END, 0,
    KEY_F0, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, 0, KEY_F6, KEY_F7, KEY_F8,
    KEY_F9, KEY_F10, 0, KEY_F11, KEY_F12, KEY_F13, KEY_F14, 0, KEY_F15, KEY_F16,
    0, KEY_F17, KEY_F18, KEY_F19, KEY_F20, };

static enum keycode letter[] = {
    KEY_UP, KEY_DOWN, KEY_RIGHT, KEY_LEFT, KEY_CENTER, KEY_END, KEY_CENTER, KEY_HOME, 0, 0,
    0, 0, 0, 0, 0, KEY_F1, KEY_F2, KEY_F3, KEY_F4, 0,
    0, 0, 0, 0, 0, 0, };

static int modcodes[] = {
    0, 0, MOD_SHIFT, MOD_ALT, MOD_SHIFT+MOD_ALT, MOD_CTRL, MOD_SHIFT+MOD_CTRL, MOD_ALT+MOD_CTRL,
    MOD_SHIFT+MOD_ALT+MOD_CTRL };


static int tinitbuf(struct terminal_t *term) {
    if (!(term->buffer = malloc(64))) return -1;
    term->buffer_end = term->buffer + 64;
    term->input = term->input_end = term->buffer;
    return 0;
}

struct terminal_t *new_terminal(int fd) {
    struct terminal_t *rv = malloc(sizeof *rv);
    tinitbuf(rv);
    rv->fd = fd;
    rv->termios_change = 0;
    return rv;
}

static int expand_buffer(struct terminal_t *term) {
    size_t input = term->input - term->buffer;
    size_t input_end = term->input_end - term->buffer;
    size_t size = term->buffer_end - term->buffer;
    size = size ? size*2 : 64;
    void *newbuf = realloc(term->buffer, size);
    if (newbuf) {
        term->buffer = newbuf;
        term->buffer_end = term->buffer + size;
        term->input = term->buffer + input;
        term->input_end = term->buffer + input_end;
        return 0;
    } else {
        return -1;
    }
}

static int fill_buffer(struct terminal_t *term, int nowait) {
    if (!term->buffer && tinitbuf(term) < 0) return -1;
    if (term->input != term->input_end) nowait = true;
    if (!term->termios_change) {
        tcgetattr(term->fd, &term->saved_term);
        term->current_term = term->saved_term;
        term->current_term.c_iflag |= ICRNL;
        term->current_term.c_oflag |= ONLCR;
        term->current_term.c_lflag &= ~(ICANON | ECHO);
        term->current_term.c_cc[VTIME] = nowait ? 0 : 1;
        term->current_term.c_cc[VMIN] = nowait ? 0 : 10;
        term->termios_change = 1;
        tcsetattr(term->fd, TCSANOW, &term->current_term);
    } else if (term->current_term.c_cc[VTIME] != (nowait ? 0 : 1)) {
        term->current_term.c_cc[VTIME] = nowait ? 0 : 1;
        term->current_term.c_cc[VMIN] = nowait ? 0 : 10;
        tcsetattr(term->fd, TCSANOW, &term->current_term); }
    if (term->input == term->input_end) {
        term->input = term->buffer;
        int len = read(term->fd, term->input, term->buffer_end - term->input - 1);
        if (len < 0) return -1;
        term->input_end = term->input + len;
    } else {
        if (term->input - term->buffer > term->buffer_end - term->input_end) {
            memmove(term->buffer, term->input, term->input_end - term->input);
            term->input_end -= term->input - term->buffer;
            term->input = term->buffer; }
        if (term->buffer_end - term->input_end >= 16) {
            int len = read(term->fd, term->input_end, term->buffer_end - term->input_end - 1);
            if (len > 0)
                term-> input_end += len; } }
    *term->input_end = 0;
    return 0;
}

int (tgetch)(struct terminal_t *term, int nowait) {
    if (!term) term = &stdin_term;
    if (fill_buffer(term, nowait) < 0)
        return -1;
    if (term->input == term->input_end)
        return 0;
    char *p = 0;
    bool isCSI = false;
    if (term->input[0] == 0x1b && (term->input[1] == '[' || term->input[1] == 'O')) {
        // 7-bit CSI or SS3 codes
        isCSI = term->input[1] == '[';
        p = term->input + 2; }
    if ((term->input[0] & 0xff) == 0x9b || (term->input[0] & 0xff)  == 0x8f) {
        // 8-bit CSI or SS3 codes
        isCSI = (term->input[0] & 0xff) == 0x9b;
        p = term->input + 1; }
    if (p) {
        unsigned modifier = 0, modval = 0, val = 0;
        if (isdigit(*p)) {
            val = strtol(p, &p, 10);
            if (*p == ';') {
                modval = strtol(p+1, &p, 10);
                if (modval < sizeof(modcodes)/sizeof(modcodes[0]))
                    modifier |= modcodes[modval]; }
            if (*p == '~') {
                term->input = p+1;
                if (val < sizeof(num_tilde)/sizeof(num_tilde[0]) && num_tilde[val])
                    return num_tilde[val] + modifier;
                return val + modifier + MOD_UNKNOWN; } }
        if (isCSI && *p == 'R') {
            // response to cursor pos request
            term->input = p + 1;
            return (val << 16) + modval;
        } else if (isCSI && *p == 'M' && term->input_end > p + 3) {
            // X10 mouse input
            term->input = p + 4;
            return (((p[1] & 0xff) - 32) << 24) | (((p[2] & 0xff) - 32) << 12) |
                    ((p[3] & 0xff) - 32) | MOUSE;
        } else if (isCSI && *p == '<' && isCSI) {
            // SGR mouse input
            bool ok = true;
            unsigned button = strtol(p+1, &p, 10);
            if (*p++ != ';') ok = false;
            unsigned x = strtol(p, &p, 10);
            if (*p++ != ';') ok = false;
            unsigned y = strtol(p, &p, 10);
            if (ok && term->input_end > p && (*p == 'M' || *p == 'm')) {
                if (*p == 'm') button = 3;
                term->input = p + 1;
                if (button & 0x80) button |= 0x40;  // map 8-9 to 4-5
                return MOUSE | (button << 24) | (x << 12) | y;
            }
        } else if (isupper(*p) && letter[*p - 'A']) {
            term->input = p+1;
            return letter[*p - 'A'] + modifier; } }
    if (term->input[0] == 0x1b && term->input[1] != 0) {
        ++term->input;
        return (*term->input++ & 0xff) | MOD_ALT; }
    return *term->input++ & 0xff;
}

static void setup_query(struct terminal_t *term) {
    fill_buffer(term, 1);
    while (term->buffer_end - term->input_end <= 16 && expand_buffer(term) >= 0)
        fill_buffer(term, 1);
}

static void copy_buffer(struct terminal_t *term, char *p, int len) {
    if (term->buffer_end - term->input_end <= len) {
        if (expand_buffer(term) < 0) return; }
    memcpy(term->input_end, p, len);
    term->input_end += len;
}

static int get_response(struct terminal_t *term) {
    char temp[32];
    while(1) {
        int len = read(term->fd, temp, sizeof(temp) - 1);
        if (len < 0) return -1;
        temp[len] = 0;
        for (char *p = temp; p < temp+len; ++p) {
            char *s, *e;
            if (p[0] == 0x1b && p[1] == '[')
                s = p + 2;
            else if ((p[0] & 0xff) == 0x9b)
                s = p + 1;
            else {
                copy_buffer(term, p, 1);
                continue; }
            e = s + strspn(s, "0123456789;");
            if (*e == 0) {
                if (p == temp) return -1;   // should not be possible
                memmove(temp, p, len - (p - temp));
                int left = len - (p - temp);
                len = read(term->fd, temp + left, sizeof(temp) - left - 1);
                if (len < 0) return -1;
                len += left;
                p = temp - 1;
            } else if (*e == 'R') {
                unsigned row, col;
                sscanf(s, "%u;%u", &row, &col);
                if (++e < temp + len)
                    copy_buffer(term, e, temp + len - e);
                *term->input_end = 0;
                return (row << 16) + col;
            } else {
                copy_buffer(term, p, e - p);
                p = e - 1; }
        }
    }
}

void treset(struct terminal_t *term) {
    if (!term) term = &stdin_term;
    if (term->termios_change) {
        tcsetattr(term->fd, TCSANOW, &term->saved_term);
        term->termios_change = false; }
    write(term->fd, MOUSE_DISABLE, sizeof(MOUSE_DISABLE));
    write(term->fd, MOUSE_BTN_DISABLE, sizeof(MOUSE_BTN_DISABLE));
    write(term->fd, MOUSE_ANY_DISABLE, sizeof(MOUSE_ANY_DISABLE));
}

void tflush_input(struct terminal_t *term) {
    if (!term) term = &stdin_term;
    term->input = term->input_end = term->buffer;
    tcflush(term->fd, TCIFLUSH);
}

unsigned tgetyx(struct terminal_t *term) {
    if (!term) term = &stdin_term;
    setup_query(term);
    write(term->fd, "\x1b[6n", 4);
    return get_response(term);
}

unsigned tgetsize(struct terminal_t *term) {
    if (!term) term = &stdin_term;
    write(term->fd, "\x1b[s\x1b[9999;9999H", 15);
    unsigned rv = tgetyx(term);
    write(term->fd, "\x1b[u", 3);
    return rv;
}

static char temp_buffer[1024], *temp = temp_buffer;

const char *gotoyx(int row, int col) {
    if (temp_buffer + sizeof(temp_buffer) - temp < 16) temp = temp_buffer;
    char *rv = temp;
    temp += sprintf(temp, "\x1b[%d;%dH", row, col);
    ++temp;
    return rv;
}

const char *fgcolor(int r, int g, int b) {
    if (temp_buffer + sizeof(temp_buffer) - temp < 24) temp = temp_buffer;
    char *rv = temp;
    temp += sprintf(temp, "\x1b[38;2;%d;%d;%dm", r, g, b);
    ++temp;
    return rv;
}

const char *bgcolor(int r, int g, int b) {
    if (temp_buffer + sizeof(temp_buffer) - temp < 24) temp = temp_buffer;
    char *rv = temp;
    temp += sprintf(temp, "\x1b[48;2;%d;%d;%dm", r, g, b);
    ++temp;
    return rv;
}
