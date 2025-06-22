#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main() {
    FILE *fp = fopen("rgb.txt", "r");
    int r,g,b;
    char        color[128];
    while(fscanf(fp, "%d%d%d %[^\n]", &r, &g, &b, color) == 4) {
        printf("#define ");
        int skip;
        for (char *p = color; *p; ++p) {
            if (islower(*p)) putchar(toupper(*p));
            else if (*p == ' ') putchar('_');
            else if (isalnum(*p)) putchar(*p);
            else ++skip;
        }
        printf("%*s", (int)(24 - strlen(color) + skip), "");
        printf("0x%02x, 0x%02x, 0x%02x\n", r, g, b);
    }
}
