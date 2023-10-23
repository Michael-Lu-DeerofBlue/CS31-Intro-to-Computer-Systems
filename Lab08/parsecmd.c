/*
 * Swarthmore College, CS 31
 * Copyright (c) 2022 Swarthmore College Computer Science Department,
 * Swarthmore PA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parsecmd.h"

/*
 * Initializing a prototype function that gets each token and returns the token 
 * to the result array.
 * start_index: a pointer to the index this function should start at to seek for the token
 * str: a string pointer that points to the copy of the command line
 * len : the length of the command line
 * return: a single generalized token from the command line
*/
char* malloc_arg(int* start_index, char *str, int len);

/*
 * Prase the token inside the command line dynamically
 * cmdline: a string pointer that points to the cmdline input
 * bg: a int pointer that points to a int that indicates whether the command
 * is going to run in the background or not
 * return: the result array that contains each dynmically allocated token
*/
char **parse_cmd_dynamic(const char *cmdline, int *bg) {
    char **result = NULL;
    int len = strlen(cmdline);

    char *copy = malloc((len + 1) * sizeof(char));
    strcpy(copy, cmdline);

    int tok_num = 0;
    int prev_char = 0;
    *bg = 0;

    for (int i = 0; i < len; i++) {
        if (copy[i] == '&') {
            *bg = 1;
            break;
        }
        if (!prev_char) {
            if (!isspace(copy[i])) {
                tok_num++;
                prev_char = 1;
            }
        } else {
            if (isspace(copy[i])) {
                prev_char = 0;
            }
        }
    }
    if (len == 1 && copy[0] == '\n'){
        tok_num = 1;
    }
    result = malloc((tok_num+1) * sizeof(char*));
    int zero = 0;
    int *curr_index = &zero;
    for (int i = 0; i < tok_num; i++) {
        result[i] = malloc_arg(curr_index, copy, len);
    }
    result[tok_num] = NULL;
    for (int i = 0; i < tok_num; i++) {
    }
    free(copy);
    return result;
}

/*
 * This function malloc's a string to store a single argument
 * (the first argument from start_index onwards.)
 * *start_index: the index from which to begin searching
 * *str: the command line string
 * len: the length of the command line string.
 */
char* malloc_arg(int* start_index, char *str, int len) {
    int start_found = 0;
    int tok_start = 0;
    int tok_end = 0;
    for (int i = *start_index; i < len; i++){
        if (!isspace(str[i]) && start_found == 0){
            tok_start= i;
            start_found = 1;        
        }
        if ((isspace(str[i]) || str[i]=='&') && start_found == 1){
            tok_end= i;
            break;        
        }
    }

    char *token = malloc(sizeof(char)*(tok_end-tok_start+1));
    for (int i = tok_start; i <= tok_end; i++){
        token[i-tok_start] = str[i];
        if (i == tok_end){
            token[i-tok_start] = '\0';
        }
    }
    *start_index = tok_end;
    return token;
}
