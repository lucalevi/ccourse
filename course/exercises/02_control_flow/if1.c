/*
 * if1: making a decision
 *
 *     if (condition) {
 *         ...runs when the condition is true...
 *     } else {
 *         ...runs otherwise...
 *     }
 *
 * Comparison operators:  ==  !=  <  <=  >  >=
 *
 * Beware: a single = stores a value, a double == compares two values.
 * Writing = inside a condition is legal C. It stores the value and then
 * treats that value as the condition, which is almost never what you want.
 *
 * TASK
 * Water at 5 degrees is not boiling. Fix the program so it compiles
 * without warnings and prints:
 *
 *     not boiling
 */

#include <stdio.h>

int main(void) {
    int temperature = 5;

    if (temperature = 100) {
        printf("boiling\n");
    } else {
        printf("not boiling\n");
    }
    return 0;
}
