/* Each case ends with break, so execution cannot fall into the next. */

#include <stdio.h>

int main(void) {
    int day = 2;

    switch (day) {
    case 1:
        printf("Monday\n");
        break;
    case 2:
        printf("Tuesday\n");
        break;
    case 3:
        printf("Wednesday\n");
        break;
    default:
        printf("some other day\n");
        break;
    }
    return 0;
}
