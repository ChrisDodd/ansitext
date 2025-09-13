#ifndef _menu_h_
#define _menu_h_

struct menu_args;

typedef void *menu_callback_t(void *, struct menu_args *);

struct menu {
    unsigned width;
    struct {
        char *label;
        menu_callback_t *action;
        void *action_arg;
    } entries[];
};

struct menu_args {
    unsigned row, col;
    const struct menu *menu;
    unsigned index;
};

void *popup_menu(unsigned row, unsigned col, struct menu *);
void *cascade_menu(void *, struct menu_args *);
unsigned menu_area(struct menu *);

#endif  /* _menu_h_ */
