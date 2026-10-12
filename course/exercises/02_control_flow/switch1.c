/*
 * switch1: choosing by value
 *
 * switch jumps to the case that matches a whole-number value:
 *
 *     switch (day) {
 *     case 1:
 *         ...
 *         break;
 *     default:
 *         ...runs when no case matches...
 *     }
 *
 * The trap: execution does not stop at the next case label. It "falls
 * through" and keeps running the cases below until it meets a break.
 *
 * TASK
 * Day 2 is Tuesday, and nothing else. Fix the program so it prints only:
 *
 *     Tuesday
 */

#include <stdio.h>

int main(void) {
    int day = 2;

    switch (day) {
    case 1:
        printf("Monday\n");
    case 2:
        printf("Tuesday\n");
    case 3:
        printf("Wednesday\n");
    default:
        printf("some other day\n");
    }
    return 0;
}
