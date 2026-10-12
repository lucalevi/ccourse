/* a is 0 and therefore false; b is 5 and therefore true; b - 5 is 0. */

#include <stdio.h>

int main(void) {
    int a = 0;
    int b = 5;

    if (a && b) {
        printf("one\n");
    }
    if (a || b) {
        printf("two\n");
    }
    if (!a) {
        printf("three\n");
    }
    if (b) {
        printf("four\n");
    }
    if (b - 5) {
        printf("five\n");
    }
    if (a == 0 && b > 3) {
        printf("six\n");
    }
    return 0;
}

/* PREDICT
two
three
four
six
*/
