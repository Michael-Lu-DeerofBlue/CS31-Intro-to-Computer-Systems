/*
 * Swarthmore College, CS 31
 * Copyright (c) 2022 Swarthmore College Computer Science Department,
 * Swarthmore PA
 */

/*
 * TODO: add your top-level comment here
 *
 */

#include <stdio.h>      // the C standard I/O library
#include <stdlib.h>     // the C standard library
#include <string.h>     // the C string library
#include "readfile.h"   // my file reading routines

/* #define constant values here: */
#define MAXFILENAME  (128)
#define ARRAYSIZE    (1000)

// TODO: Define new functions here:

/* THE FOLLOWING FUNCTION IS PROVIDED FOR YOU
 * DO NOT MODIFY, OR DO SO AT YOUR OWN RISK!
 *
 * This function gets the filename passed in as a command line option
 * and copies it into the filename parameter. It exits with an error
 * message if the command line is badly formed.
 *
 *   filename: the string to fill with the passed filename
 *   argc, argv: the command line parameters from main
 *               (number and strings array)
 */
void get_filename_from_cmdline(char filename[], int argc, char *argv[]) {
    if(argc != 2) {
        printf("Usage: ./exp integer_file\n");
        exit(1);
    }
    if(strlen(argv[1]) >= MAXFILENAME) {
        printf("Filename, %s, is too long, cp to shorter name and try again\n",
                filename);
        exit(1);
    }
    strcpy(filename, argv[1]);
}

int main (int argc, char *argv[]) {
    int ret;                    /* for storing function return values */
    char filename[MAXFILENAME]; /* the input filename  */
    float values[ARRAYSIZE];    /* stores values read in from file */

    // TODO: declare all additional variables here

    /* initialize filename string from the command line arguments */
    get_filename_from_cmdline(filename, argc, argv);

    /* try to open file */
    ret = open_file(filename);
    if(ret == -1) {
        printf("ERROR: can't open %s\n", filename);
        /* exit quits the program immediately. Some errors are not
         * recoverable. Exit with an error message to handle these cases. */
        exit(1);
    }

    // TODO: read in file header to get number of floats needed

    // TODO:
    // tell the user:
    // (1) the name of the file being read
    // (2) how many float values there are
    // (3) the min and max values

    // TODO: read in floats from file

    // TODO: print out the unsorted values on a single line

    // TODO: sort the array in place using a function

    // TODO: check that the array is now sorted

    // TODO: print out sorted values in the array

    close_file();  /* close file */

    /* returning 0 means main completed successfully. */
    return 0;
}
