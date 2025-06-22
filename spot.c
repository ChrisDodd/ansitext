#include <stdio.h>

static int odd(int x) { return x&1; }
// static int abs(int x) { return x > 0 ? x : -x; }

static void spot(int even, int size) {
    for (int y = -size; y <= size; ++y) {
        if ((y&1) == even) printf("   ");
        for (int x = -size; x <= size; ++x) {
            int r = y*y ;
            if (odd(y)) {
                if (even) r += (x+1)*x;
                else r += (x-1)*x;
            } else r += x*x;
            printf("%3d   ", r);
        }
        printf("\n");
    }
}

int main() {
    spot(1, 5);
    printf("\n\n");
    spot(0, 5);

}
