/* A function may contain several return statements; the first one reached wins. */

#include <stdio.h>

int max(int a, int b) {
    if (a > b) {
        return a;
    }
    return b;
}

int main(void) {
    printf("%d\n", max(3, 9));
    printf("%d\n", max(12, -4));
    printf("%d\n", max(5, 5));
    return 0;
}
