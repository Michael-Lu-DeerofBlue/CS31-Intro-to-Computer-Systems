/*
 * Swarthmore College, CS 31
 * Copyright (c) 2022 Swarthmore College Computer Science Department,
 * Swarthmore PA
 */

/*
 *  This is a program that will take in a file as an input and dynamically 
    store all of the values in the heap. Then, it will store the values and
    print out the number of values, min, max, mean, median, standard deviation
    and unused array capacity. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "readfile.h"

/*
 * N: start size for first dynamically allocated array of values
 * MAXFILENAME: max length of input file name string + 1 (includes '\0')
 */
#define N             (20)
#define MAXFILENAME   (256)

/* This function reads in values from the input file into a dynamically
 * allocated array of floats that it returns.  It uses a doubling algorithm to
 * increase the size of the array as it needs more space for the values read in
 * from the file.
 *
 *   size: An "output" parameter. Your implementation of get_values should
 *   dereference this pointer and set the value so that the caller (main) can
 *   learn how many values it read in from the file (how full the returned
 *   array is).
 *
 *   capacity: An "output" parameter.  Your implementation of get_values should
 *   dereference this pointer and set the value so that the caller (main) can
 *   learn the maximum capacity of the returned array.
 *
 *   filename: the name of the file to open
 *
 *   returns: the base address of the array of values read in
 *            or NULL on error
 */
float *get_values(int *size, int *capacity, char *filename){
    float *values = NULL;
    // TODO: add more local variables here:
    *size = 0;
    int success = 0;

    /* Open the file, checking for errors.  If error, just return NULL. */
    if (open_file(filename)) {
        return NULL;
    }

    *capacity = N;
    values = malloc(sizeof(float)* *capacity);

    /* Checks to see if there are still values being read from the file.
     * If there are values being read, store it in the heap. If not, exit
     * the loop. If the capacity is reached, allocate new spaced in the heap
     * with double the capacity and copy the values over. Free this array 
     * and point the old array to this address.  */
    while (success != -1) {
        if ((*size)==*capacity){
            float *copied_values = NULL;
            *capacity = *capacity * 2;
            copied_values = malloc(sizeof(float)* *capacity);
            for (int i = 0; i < *size; i++){
                copied_values[i] = values[i];
            }

            /* Frees the space in the heap after use */
            free(values);
            values = NULL;
            values = copied_values;
        }
        success = read_float(&values[*size]);
        if (success != -1){
            *size += 1;
        }
    }
    /* Close the file. */
    close_file();

    /* Return the values array. */
    return values;
}

/*
 * This function swaps the a-th and b-th location in the array
 * 
 * array: this is the base address of the data of the txt file in the heap
 * a: the first index to swap
 * b: the second index to swap
 *
 * This function returns nothing 
  */
void swapLoc(float *array, int a, int b) {
    float temp;
    temp = array[a];
    array[a] = array[b];
    array[b] = temp;
}

/*
 * This function uses bubble swap and with the previous swapLoc function
   to sort the array in non-decreasing order
 * 
 * array: this is the base address of the data of the txt file in the heap
 * len: the size of the array stored in heap
 *
 * This function returns nothing 
 */
void bubbleSort(float *array, int len){
    for (int i = 0; i < len; i++){
        for (int j = 0; j < len-i- 1; j++){
            if (array[j] > array[j+1]){
                swapLoc(array,j,j+1);
            }
        }
    }
}

/*
 * This function gets the minimum value of an array
 * 
 * array: this is the base address of the data of the txt file in the heap
 *
 * This function returns the minimum value of the array 
 */
float get_min(float *array) {
    return array[0]; 
}

/*
 * This function gets the maximum value of an array
 * 
 * array: this is the base address of the data of the txt file in the heap
 *
 * This function returns the maximum value of the array 
 */
float get_max(float *array, int size) {
    return array[size-1]; 
}

/*
 * This function gets the mean value of an array by adding up all of the values
 * in the array and then dividing it by the size of the array. 
 * 
 * array: this is the base address of the data of the txt file in the heap
 * size: the size of the array stored in the heap
 *
 * This function returns the mean value of the array
 */
float get_mean(float *array, int size) {
    float sum;
    float mean;
    sum = 0;
    for (int i = 0; i < size; i++){
        sum += array[i]; 
    }
    mean = sum/size;
    return mean;
}

/*
 * This function gets the median value of an array by returning the value of 
 * the array at the index of the array divided by 2. 
 * 
 * array: this is the base address of the data of the txt file in the heap
 * size: the size of the array stored in the heap
 *
 * This function returns the median value of the array
 */
float get_median(float *array, int size) {
    return array[size/2]; 
}

/*
 * This function gets the standard deviation value of an array by using the
 * standard deviation formula.
 * 
 * array: this is the base address of the data of the txt file in the heap
 * size: the size of the array stored in the heap
 * mean: this is the mean of the array that we calculated before 
 *
 * This function returns the standard deviation value of the array
 */
float get_stddev(float *array, int size, float mean) {
    float delta_x_squared;
    float std_dev;
    delta_x_squared = 0;
    for (int i = 0; i < size; i++){
        delta_x_squared += (array[i]-mean)*(array[i]-mean);
    }
    std_dev = sqrt((1.0/(size-1.0))*delta_x_squared);
    return std_dev; 
}

/*
 * This function calculates the unused space in the heap by subtracting the 
 * size of the array from the capacity allocated in the heap. 
 * 
 * size: the size of the array stored in the heap
 * capacity: the amount of space allocated for the array in the heap
 *
 * This function returns the unused capacity in the heap for the array. 
 */
float get_unused(int size, int capacity){
    return capacity-size;
}

int main(int argc, char **argv) {
    char filename[MAXFILENAME];
    float *vals=NULL;
    /* size- the size of the array stored in the heap
     * capacity- the amount of space allocated in the heap for the array
     * min- the minimum value of the array
     * max- the maximum value of the arrays
     * mean- the mean value of the array
     * median- the median value of the array
     * std_dev- the standard deviation of the array
     * unused_size- the unused capacity in the heap allocated for heapo
      */
    int size, capacity;
    float min, max, mean, median, std_dev;
    int unused_size;


    /* This code checks command line args to make sure program was run
     * with a filename argument.  If not, calls exit to quit the program. */
    if (argc != 2) {
        printf ("usage: ./stats filename\n");
        exit(1);
    }

    /* This copies the filename command line argument into the
     * string variable "filename". */
    strncpy(filename, argv[1], MAXFILENAME-1);

    /* Call the get_values function to do the interesting memory stuff. */
    vals= get_values(&size, &capacity, filename);

    /* Calls the functions that sorts the array */
    bubbleSort(vals, size);
    /* Call the functions to get key metrics  */
    min = get_min(vals);
    max = get_max(vals,size);
    mean = get_mean(vals,size);
    median = get_median(vals,size);
    std_dev = get_stddev(vals,size,mean);
    unused_size = get_unused(size,capacity);
    /* Prints out the key metrics for the array */
    printf("Results:\n-------\n");
    printf("num values:\t%d\n",size); 
    printf("min:\t\t%.3f\n",min);
    printf("max:\t\t%.3f\n",max);
    printf("mean:\t\t%.3f\n",mean);
    printf("median:\t\t%.3f\n",median);
    printf("std dev:\t%.3f\n",std_dev);
    printf("unused array capacity:%d\n",unused_size);

    /* Frees the space in the heap after use */
    free(vals);
    vals = NULL;
    return 0;
}
