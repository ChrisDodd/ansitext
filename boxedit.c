#include <stdlib.h>
#include <string.h>
#include "ansi.h"
#include "boxes.h"
#include "editline.h"
#include "utf8.h"

int main(int ac, char **av) {
    int style_idx = 0;
    VECTOR(struct box_t) boxes = 0;
    VECTOR(char) savefile = 0;
    printf("%s%s", CLEARALL, gotoyx(0, 0));
    fflush(stdout);
    if (ac == 2) {
        VECTOR_resize(savefile, strlen(av[1]));
        VECTOR_terminate(savefile, 0);
        strcpy(savefile, av[1]);
        FILE *fp = fopen(savefile, "r");
        if (fp) {
            boxes = read_boxes(fp);
        } else {
            fprintf(stderr, "Can't open %s\n", av[1]); } }
    fputs(MOUSE_BTN_ENABLE MOUSE_SGR, stdout);
    fflush(stdout);
    for (box_t *b = VECTOR_begin(boxes); b < VECTOR_end(boxes); ++b) draw_box(b);
    fflush(stdout);
    box_t *current = 0;
    char *edit = 0;
    while (1) {
        int cursor = 0;
        if (current) {
            cursor = set_box_edit(current, edit);
            fflush(stdout); }
        int ch = getch();
        if (ch & MOUSE) {
            boxedge_t edge;
            current = find_box(boxes, MOUSE_Y(ch), MOUSE_X(ch), &edge);
            if (MOUSE_1(ch) && current) {
                if (edge == NONE) {
                    ch = move_box(current, MOUSE_Y(ch) - current->top,
                                  MOUSE_X(ch) - current->left, boxes);
                } else {
                    ch = resize_box(current, edge, boxes); }
                if (ch & MOUSE)
                    edit = get_box_edit(current, MOUSE_Y(ch), MOUSE_X(ch));
            } else if (MOUSE_3(ch) && !current) {
                struct box_t tmp;
                memset(&tmp, 0, sizeof tmp);
                tmp.style = &styles[style_idx];
                tmp.label = 0;
                tmp.top = tmp.bottom = MOUSE_Y(ch);
                tmp.left = tmp.right = MOUSE_X(ch);
                tmp.bottom++;
                tmp.right++;
                VECTOR_add(boxes, tmp);
                ch = resize_box(&VECTOR_top(boxes), BOTTOM_RIGHT, boxes);
            } else if (MOUSE_3(ch) && edge != NONE) {
                ch = add_link(current, edge, MOUSE_Y(ch), MOUSE_X(ch), boxes);
            } else {
                printf("%smouse: %x", gotoyx(0, 50), ch);
                fflush(stdout);
                // other button -- menu?
            }
        }
        switch (ch) {
        case KEY_END:
            fputs(gotoyx(999, 0), stdout);
            fflush(stdout);
            reset();
            if (savefile && !VECTOR_empty(boxes)) {
                FILE *fp = fopen(savefile, "w");
                if (fp) {
                    write_boxes(boxes, fp);
                    fclose(fp);
                } else {
                    fprintf(stderr, "Can't write to %s\n", savefile); } }
            exit(0);
        case KEY_UP:
            if (current && MOUSE_Y(cursor))
                edit = get_box_edit(current, MOUSE_Y(cursor) - 1, MOUSE_X(cursor));
            break;
        case KEY_DOWN:
            if (current)
                edit = get_box_edit(current, MOUSE_Y(cursor) + 1, MOUSE_X(cursor));
            break;
        case KEY_LEFT:
            if (current && MOUSE_X(cursor)) {
                char *prev = edit;
                edit = get_box_edit(current, MOUSE_Y(cursor), MOUSE_X(cursor) - 1);
                if (prev == edit)
                    edit = get_box_edit(current, MOUSE_Y(cursor) - 1, current->right);
            }
            break;
        case KEY_RIGHT:
            if (current) {
                char *prev = edit;
                edit = get_box_edit(current, MOUSE_Y(cursor), MOUSE_X(cursor) + 1);
                if (prev == edit)
                    edit = get_box_edit(current, MOUSE_Y(cursor) + 1, current->left);
            }
            break;
        case KEY_HOME:
            printf("%sSave file: ", gotoyx(0, 0));
            fflush(stdout);
            edit_string(&savefile);
            if (!VECTOR_empty(boxes)) {
                FILE *fp = fopen(savefile, "w");
                if (fp) {
                    write_boxes(boxes, fp);
                    fclose(fp);
                } else {
                    fprintf(stderr, "Can't write to %s\n", savefile); } }
            break;
        case '\b':
        case 0x7f:
        case KEY_DELETE:
            if (current && edit > current->label) {
                if (edit[-1] == '\n') {
                    VECTOR_erase(current->label, --edit - current->label);
                } else {
                    char *p = get_box_edit(current, MOUSE_Y(cursor), MOUSE_X(cursor) - 1);
                    VECTOR_erase(current->label, p - current->label, edit - p);
                    edit = p; }
                VECTOR_terminate(current->label, 0);
                current->fmt_width = 0;  // force recompute of fmt
                draw_box(current);
                fflush(stdout); }
            break;
        case KEY_F1: style_idx = 0; goto setstyle;
        case KEY_F2: style_idx = 1; goto setstyle;
        case KEY_F3: style_idx = 2; goto setstyle;
        case KEY_F4: style_idx = 3; goto setstyle;
        setstyle:
            if (current) {
                current->style = &styles[style_idx];
                draw_box(current);
                fflush(stdout); }
            break;
        default:
            if (ch < ' ' || ch >= 0x7f) break;
            // fall through
        case '\n':
            if (current) {
                if (edit < current->label || edit > VECTOR_end(current->label))
                    edit = VECTOR_end(current->label);
                int off = edit - current->label;
                VECTOR_insert(current->label, off);
                current->label[off] = ch;
                VECTOR_terminate(current->label, 0);
                edit = current->label + off + 1;
                current->fmt_width = 0;  // force recompute of fmt
                draw_box(current);
                fflush(stdout); }
            break;
        }
    }
}
