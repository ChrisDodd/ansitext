#ifndef _menu_h_
#define _menu_h_

struct menu {
    unsigned width;
    struct {
        char *label;
        void *(*action)(void *);
        void *action_arg;
    } entries[];
};

void *popup_menu(unsigned row, unsigned col, const struct menu *);

#endif  /* _menu_h_ */
