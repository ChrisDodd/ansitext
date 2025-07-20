#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void line(int width, char *c1, char *c2, char *c3, char *c4) {
    while (width > 1) {
        printf("%s%s%s%s", c1, c2, c3, c4);
        width -= 2;
    }
    printf("%s", c1);
    if (width == 1) printf("%s%s", c2, c3);
    printf("\n");
}

int main(int ac, char **av) {
    int width, height;
    char *p;
    if (ac != 3 || (width = strtol(av[1], &p, 0)) <= 0 || *p ||
                   (height = strtol(av[2], &p, 0)) <= 0 || *p) {
        fprintf(stderr, "usage: %s <width> <height>\n", av[0]);
        exit(1);
    }
    line(width, "\U0001fba3", "\u2500\u2500", "\U0001fba2", "  ");
    
    while (--height > 0) {
        line(width, "\U0001fba4", "  ", "\U0001fba5", "\u2500\u2500");
        line(width, "\U0001fba5", "\u2500\u2500", "\U0001fba4", "  ");
    }

    line(width, "\U0001fba4", "  ", "\U0001fba5", "\u2500\u2500");
    line(width, "\U0001fba1", "\u2500\u2500", "\U0001fba0", "  ");
}
