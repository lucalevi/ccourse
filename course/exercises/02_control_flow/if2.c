/*
 * if2: choosing between several cases
 *
 * Conditions are tried from top to bottom and only the first true one
 * runs:
 *
 *     if (score >= 90) {
 *         ...
 *     } else if (score >= 80) {
 *         ...
 *     } else {
 *         ...
 *     }
 *
 * TASK
 * Set grade from score:  90 and above 'A',  80 and above 'B',
 * 70 and above 'C',  60 and above 'D',  anything lower 'F'.
 * The program must print:
 *
 *     score 73 -> grade C
 */

#include <stdio.h>

int main(void) {
    int score = 73;
    char grade = '?';

    /* TODO: set grade */

    printf("score %d -> grade %c\n", score, grade);
    return 0;
}
