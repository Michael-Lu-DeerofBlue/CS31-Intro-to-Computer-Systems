/*
 * Swarthmore College, CS 31
 * Copyright (c) 2022 Swarthmore College Computer Science Department,
 * Swarthmore PA
 */

/*
 * A program that calls the sum function you will
 * implement in sum.s in ARM64
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* computes the sum of the values 1 to n
 * n: a long int value
 * returns: the sum of values from 1 to n, or -1 if n is not positive
 */
long sum(long n);


int main(void) {

    long res, n;

    printf("This program computes the sum of 1 to N\n");

    printf("Enter an value for n: ");
    scanf("%ld", &n);

    res = sum(n);

    if (res == -1) {
        printf("ERROR: sum passed a non-positive value %ld\n", n);
        printf("try running again and pass a positive value\n");
    } else {
        printf("The sum of 1 to %ld is %ld\n", n, res);
    }

    return 0;
}
