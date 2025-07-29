#ifndef _ansi_h_
#define _ansi_h_

struct terminal_t;

struct terminal_t *new_terminal(int fd);

// the 'nowait' arg is optional and defaults to false
int tgetch(struct terminal_t *, int nowait);
#define tgetch(TERM, ...) tgetch(TERM, __VA_ARGS__+0)
#define getch(A)  tgetch(0, A+0)

void treset(struct terminal_t *);
#define reset() treset(0)
void tflush_input(struct terminal_t *);
#define flush_input() tflush_input(0)


unsigned tgetyx(struct terminal_t *);
#define getyx() tgetyx(0)
#define GETX(SS)  ((SS) & 0xffff)
#define GETY(SS)  ((SS) >> 16)
#define MAKEYX(Y, X)    ((((Y) & 0xffff) << 16) | ((X) & 0xffff))
#define TOPLEFT   0x10001   /* top left corner of the screen is 1,1 */
static inline unsigned getcury() { return GETY(getyx()); }
static inline unsigned getcurx() { return GETX(getyx()); }
#define GETYX(ROW, COL)     (ROW = getyx(), COL = GETX(ROW), ROW = GETY(ROW))
unsigned tgetsize(struct terminal_t *);
#define getsize() tgetsize(0)

const char *gotoyx(int row, int col);
const char *fgcolor(int r, int g, int b);
const char *bgcolor(int r, int g, int b);
#define PLAIN           "\x1b[0m"
#define BOLD            "\x1b[1m"
#define FAINT           "\x1b[2m"
#define ITALIC          "\x1b[3m"
#define UNDERLINE       "\x1b[4m"
#define BLINK           "\x1b[5m"
#define FASTBLINK       "\x1b[6m"
#define REVERSE         "\x1b[7m"
#define CONCEAL         "\x1b[8m"
#define STRIKEOUT       "\x1b[9m"
#define FGBLACK         "\x1b[30m"
#define FGRED           "\x1b[31m"
#define FGGREEN         "\x1b[32m"
#define FGYELLOW        "\x1b[33m"
#define FGBLUE          "\x1b[34m"
#define FGMAGENTA       "\x1b[35m"
#define FGCYAN          "\x1b[36m"
#define FGWHITE         "\x1b[37m"
#define BGBLACK         "\x1b[40m"
#define BGRED           "\x1b[41m"
#define BGGREEN         "\x1b[42m"
#define BGYELLOW        "\x1b[43m"
#define BGBLUE          "\x1b[44m"
#define BGMAGENTA       "\x1b[45m"
#define BGCYAN          "\x1b[46m"
#define BGWHITE         "\x1b[47m"
#define CLEAREOL        "\x1b[K"
#define CLEAREOP        "\x1b[J"
#define CLEARLINE       "\x1b[2K"
#define CLEARALL        "\x1b[2J"
// CLEARRECT/FILLRECT (DECERA/DECFRA) are defined and documented by xterm, but don't
// seem to work
#define CLEARRECT       "\x1b[%d;%d;%d;%d$z"  /* top, left, bottom, right */
#define FILLRECT        "\x1b[%d;%d;%d;%d;%d$x"  /* char, top, left, bottom, right */
#define MOVEUP          "\x1b[A"
#define MOVEDN          "\x1b[B"
#define MOVERIGHT       "\x1b[C"
#define MOVELEFT        "\x1b[D"
#define SCROLLUP        "\x1b[S"
#define SCROLLDN        "\x1b[T"
#define CURSORON        "\x1b[?25h"
#define CURSOROFF       "\x1b[?25l"
#define CURSORBLINK     "\x1b[1 q"
#define CURSORBLOCK     "\x1b[2 q"
#define CURSORULINE     "\x1b[3 q"
#define CURSORBLINKUL   "\x1b[4 q"

#define MOUSE_ENABLE            "\x1b[?1000h"
#define MOUSE_BTN_ENABLE        "\x1b[?1002h"
#define MOUSE_ANY_ENABLE        "\x1b[?1003h"
#define MOUSE_SGR               "\x1b[?1006h"
#define MOUSE_PIXEL_POS         "\x1b[?1016h"
#define MOUSE_DISABLE           "\x1b[?1000l"
#define MOUSE_BTN_DISABLE       "\x1b[?1002l"
#define MOUSE_ANY_DISABLE       "\x1b[?1003l"
#define MOUSE_Y(CC)     ((CC) & 0xfff)
#define MOUSE_X(CC)     (((CC) >> 12) & 0xfff)
#define MOUSE_1(CC)     ((((CC) >> 24) & 0x43) == 0)
#define MOUSE_2(CC)     ((((CC) >> 24) & 0x43) == 1)
#define MOUSE_3(CC)     ((((CC) >> 24) & 0x43) == 2)
#define MOUSE_UP(CC)    ((((CC) >> 24) & 0x43) == 3)
#define MOUSE_4(CC)     ((((CC) >> 24) & 0x43) == 0x40)
#define MOUSE_5(CC)     ((((CC) >> 24) & 0x43) == 0x41)

enum keycode {
    KEY_UP = 0x100, KEY_DOWN, KEY_RIGHT, KEY_LEFT, KEY_CENTER,
    KEY_HOME, KEY_END, KEY_PGUP, KEY_PGDN, KEY_INSERT, KEY_DELETE,
    KEY_F0, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9,
    KEY_F10, KEY_F11, KEY_F12, KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18, KEY_F19,
    KEY_F20, KEY_F21, KEY_F22, KEY_F23, KEY_F24, KEY_F25, KEY_F26, KEY_F27, KEY_F28, KEY_F29,

    MOD_SHIFT=0x200, MOD_ALT=0x400, MOD_CTRL=0x800, MOD_UNKNOWN=0x1000,

    MOUSE=0x80000000, MOUSE_MOTION=0x2000000,
    MOUSE_SHIFT=0x4000000, MOUSE_META=0x8000000, MOUSE_CTRL=0x10000000,
};

#endif /* _ansi_h_ */
