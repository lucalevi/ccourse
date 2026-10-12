/*
 * cast1: converting between types
 *
 * A cast converts a value to another type. You write the target type in
 * parentheses in front of the value:
 *
 *     (int) 9.99      gives 9   (the fraction is cut off, not rounded)
 *     (double) 7      gives 7.0
 *
 * TASK
 * Turn a duration in seconds into whole minutes and remaining seconds.
 * Replace the three zeros, using a cast, / and %. The program must print:
 *
 *     2 min 5 s
 */

#include <stdio.h>

int main(void) {
    double seconds = 125.9;

    int whole = 0;   /* TODO: seconds without its fraction */
    int minutes = 0; /* TODO: how many full minutes are in whole */
    int rest = 0;    /* TODO: the seconds left over */

    printf("%d min %d s\n", minutes, rest);
    return 0;
}
