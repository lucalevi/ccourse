/* It is lives that goes down, not the constant max_lives. */

#include <stdio.h>

int main(void) {
    const int max_lives = 3;
    int lives = max_lives;

    lives = lives - 1;

    printf("lives = %d of %d\n", lives, max_lives);
    return 0;
}
