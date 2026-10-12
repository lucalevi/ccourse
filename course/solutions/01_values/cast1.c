/* The cast drops the fraction; / and % split the whole seconds. */

#include <stdio.h>

int main(void) {
    double seconds = 125.9;

    int whole = (int) seconds;
    int minutes = whole / 60;
    int rest = whole % 60;

    printf("%d min %d s\n", minutes, rest);
    return 0;
}
