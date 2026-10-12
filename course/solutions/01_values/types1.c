/* Integer division truncates; 'A' + 2 is the code of 'C'. */

#include <stdio.h>

int main(void) {
    printf("%d\n", 7 / 2);
    printf("%d\n", 7 % 2);
    printf("%.1f\n", 7 / 2.0);
    printf("%d\n", 2 + 3 * 4);
    printf("%c\n", 'A' + 2);
    printf("%zu\n", sizeof(char));
    return 0;
}

/* PREDICT
3
1
3.5
14
C
1
*/
