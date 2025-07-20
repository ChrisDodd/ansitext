#include <stdio.h>

int main() {
    // printf(u8"\u250c\u2500\u2500\u2510\n\u2502  \u2502\n\u2514\u2500\u2500\u2518\n");
    printf("\U0001fba3\u2500\u2500\U0001fba2\n"
           "\U0001fba4  \U0001fba5\n"
           "\U0001fba1\u2500\u2500\U0001fba0\n");
}
