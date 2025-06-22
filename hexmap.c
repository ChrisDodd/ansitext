#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ansi.h"
#include "color.h"
#include "mapedit.h"
#include "terrain.h"
#include "vector.h"

struct terrain terrain[] = {
                        /*   background           foreground    */
    { ' ', "unknown",   { BLACK },              { GREY },               "" },
    { '+', "clear",     { LAWN_GREEN },         { FOREST_GREEN },       "" },
    { '#', "woods",     { FOREST_GREEN },       { PALE_GREEN },         "" },
    { '.', "sea",       { CYAN },               { BLUE },               "" },
    { '^', "mountain",  { BROWN },              { BLACK },              "" },
    { '~', "river",     { AQUAMARINE },         { SKY_BLUE },           "" },
};

int terrain_size = sizeof(terrain)/sizeof(terrain[0]);
unsigned char terrain_map[128];

VECTOR(VECTOR(char)) read_map(const char *name) {
    FILE *fp = fopen(name, "r");
    if (!fp) return 0;
    int rows, cols;
    VECTOR(VECTOR(char)) map = 0;
    if (fscanf(fp, "%d%d", &rows, &cols) == 2) {
        VECTOR_init(map, rows);
        VECTOR_resize(map, rows);
        fscanf(fp, "%*[^\n]"); fscanf(fp, "%*c");
        for (int i = 0; i < rows; ++i) {
            VECTOR_init(map[i], cols);
            VECTOR_resize(map[i], cols);
            memset(map[i], ' ', cols);
            for (int j = 0; j < cols; ++j) {
                int ch = getc(fp);
                if (ch == EOF || ch == '\n') goto nextline;
                map[i][j] = ch; }
            fscanf(fp, "%*[^\n]"); fscanf(fp, "%*c");
            nextline: ; } }
    fclose(fp);
    return map;
}

int save_map(VECTOR(VECTOR(char)) map, const char *name) {
    FILE *fp = fopen(name, "w");
    if (!fp) return -1;
    fprintf(fp, "%d %d\n", VECTOR_size(map), VECTOR_size(map[0]));
    for (unsigned i = 0; i < VECTOR_size(map); ++i)
        fprintf(fp, "%.*s\n", VECTOR_size(map[i]), map[i]);
    fclose(fp);
    return 0;
}

void draw_map(VECTOR(VECTOR(char)) map, int top, int left, int wrap) {
    unsigned screen = getsize();
    fputs(gotoyx(0, 0), stdout);
    for (unsigned i = top; i < VECTOR_size(map) && i < top + GETY(screen); ++i) {
        unsigned prev = (left + VECTOR_size(map[i]) - 1) % VECTOR_size(map[i]);
        int prev_color = -1;
        for (unsigned j = 0; j < GETX(screen)/2; ++j) {
            unsigned x = (prev + 1) % VECTOR_size(map[i]);
            if (!wrap && j && !x) break;
            int color = terrain_map[map[i][x] & 0x7f];
            if ((i&1) == 0 || j != 0 ) {
                int gap_color = -1;
                if (map[i][prev] == map[i][x]) {
                    if (j == 0) gap_color = color;
                } else if (i > 0 && i < VECTOR_size(map) - 1) {
                    if (i&1) {
                        if (map[i-1][prev] == map[i+1][prev])
                            gap_color = terrain_map[map[i-1][prev] & 0x7f];
                    } else {
                        if (map[i-1][x] == map[i+1][x])
                            gap_color = terrain_map[map[i-1][x] & 0x7f]; } }
                if (gap_color < 0) {
                    if (prev_color >= 0  && prev_color < color)
                        gap_color = prev_color;
                    else
                        gap_color = color; }
                if (gap_color != prev_color)
                    fputs(terrain[gap_color].color, stdout);
                putchar(' ');
                prev_color = gap_color;
            }
            if (color != prev_color)
                fputs(terrain[color].color, stdout);
            putchar(terrain[terrain_map[map[i][x] & 0x7f]].text);
            prev_color = color;
            prev = x;
        }
        printf("%s%s\n", PLAIN, CLEAREOL);
    }
    fputs(CLEAREOP, stdout);
}

int main(int ac, char **av) {
    for (int i = 0; i < terrain_size; ++i) {
        sprintf(terrain[i].color, "%s%s",
                bgcolor(terrain[i].bg.r, terrain[i].bg.g, terrain[i].bg.b),
                fgcolor(terrain[i].fg.r, terrain[i].fg.g, terrain[i].fg.b));
        terrain_map[terrain[i].text & 0x7f] = i;
    }
    int wrap = 0;
    if (ac == 3 && av[1][0] == '-' && av[1][1] == 'w' && av[1][2] == 0)
        wrap = 1;
    if (ac != wrap+2) {
        fprintf(stderr, "usage: %s [-w] <file>\n", av[0]);
        exit(1); }
    VECTOR(VECTOR(char)) map = read_map(av[wrap+1]);
    if (!map) {
        fprintf(stderr, "can't read %s\n", av[wrap+1]);
        exit(1); }
    map = edit_map(map, wrap);
    printf("%s%s%s", gotoyx(GETY(getsize())-20, 0), PLAIN, CLEAREOP);
    if (save_map(map, av[wrap+1]) < 0)
        fprintf(stderr, "can't write %s\n", av[wrap+1]);
    reset();
}
