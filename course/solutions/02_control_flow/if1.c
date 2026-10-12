/* == compares; = would have overwritten temperature with 100. */

#include <stdio.h>

int main(void) {
    int temperature = 5;

    if (temperature == 100) {
        printf("boiling\n");
    } else {
        printf("not boiling\n");
    }
    return 0;
}
