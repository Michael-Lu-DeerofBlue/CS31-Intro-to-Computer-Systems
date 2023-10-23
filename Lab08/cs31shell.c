/*
 * Swarthmore College, CS 31
 * Copyright (c) 2022 Swarthmore College Computer Science Department,
 * Swarthmore PA
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "parsecmd.h"

/* The maximum size of your circular history queue. */
#define MAXHIST 10

/*
 * A struct to keep information one command in the history of
 * command executed
 */
struct histlist_t {
    unsigned int cmd_num;
    char cmdline[MAXLINE]; // command line for this process
};

/* Global variables declared here.
 * For this assignment, only use globals to manage history list state.
 * all other variables should be allocated on the stack or heap.
 *
 * Recall "static" means these variables are only in scope in this .c file. */
static struct histlist_t history[MAXHIST];

// some more global variables for history tracking.
//       These should be the only globals in your program.
static int queue_next = 0;  // the next place to insert in the queue
static int queue_size = 0;  // the number of items in the queue
static int cmd_num = 0;     // the number of the cmdline

/* function prototypes here.
 * add_hist adds the cmdline into the history struct
 * print_hist prints the history of cmdlines
 * redo_a_cmd finds and returns the cmdline that is called by the bash
 * sigchld_handler handles the signal returned by the processes
 * free_argv frees all the allocated memory due to prasing
 */
void add_hist(int cmd_number, char* cmdline);
void print_hist(void);
char* redo_a_cmd(char* copy_cmdline, int searched_cmd_number);
void sigchld_handler(int sig);
void free_argv(char **args);

int main(int argc, char **argv) {
    signal(SIGCHLD, sigchld_handler);
    char cmdline[MAXLINE];
    char **args;
    int bg;

    while (1) {
        // (1) print the shell prompt
        printf("cs31shell> ");
        fflush(stdout);

        // (2) read in the next command entered by the user
        if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin)) {
            perror("fgets error");
        }
        if (feof(stdin)) { /* End of file (ctrl-D) */
            fflush(stdout);
            exit(0);
        }       

        // (3) make a call to your parsing library to parse it into its
        //     args format
        char *copy_cmdline = malloc((MAXLINE+1) * sizeof(char));
        strcpy(copy_cmdline, cmdline);
        args = parse_cmd_dynamic(copy_cmdline,&bg);
        if (args[0][0] == '!'){
            char *ints = malloc(strlen(args[0]) * sizeof(char));
            for(int i = 0; i < strlen(args[0]); i++){
                ints[i] = args[0][i+1];
            }
            ints[strlen(args[0])-1] = '\0';
            copy_cmdline = redo_a_cmd(copy_cmdline,atoi(ints));
            free(ints);
            free_argv(args);
            args = parse_cmd_dynamic(copy_cmdline,&bg);
        }

        // (4) determining the execution of the commands
        add_hist(cmd_num, copy_cmdline);
        cmd_num++;
        if (!strcmp(args[0],"exit")){
            printf("bye bye\n");
            free_argv(args);
            free(copy_cmdline);
            break;
        }
        if (!strcmp(args[0],"history")){
            print_hist();
        }
        else {
            pid_t pid = fork();
            if (pid == 0) {
                int ret = execvp(args[0], args);
                if (ret == -1){
                    copy_cmdline[strlen(copy_cmdline)-1] = '\0';
                    printf("%s: command not found\n",copy_cmdline);
                }
                free_argv(args);
                free(copy_cmdline);
                return 1;
            } else {
                if (bg == 0) {
                    waitpid(pid, NULL, 0);
                }
            }
        printf("%s\n",args[0]);
        }
        free_argv(args);
        free(copy_cmdline);
    }    

    return 0;
}

/*
 * frees all the allocated memory due to prasing
 * args: the pointer array that contains each dynmically allocated token
 */
void free_argv(char **args){
    int i = 0;
    while(args[i] != NULL){
        free(args[i]);
        i++;
    }
    free(args);
}

/*
 * adds the cmdline into the history struct
 * cmd_number: the number of order of the cmdline just entered
 * cmd_line: the cmdline entered by the user
 */
void add_hist(int cmd_number, char* cmdline) {
    queue_size++;
    history[queue_next].cmd_num = cmd_number;
    strcpy(history[queue_next].cmdline,cmdline);
    queue_next++;
    queue_next = queue_next % MAXHIST;
    if (queue_size > MAXHIST){
        queue_size = MAXHIST;
    }
}

/*
 * prints the history of cmdlines
 */
void print_hist(void){
    if (queue_size < MAXHIST){
        for (int i = 0; i < queue_size; i++){
            printf("\t%d\t%s",history[i].cmd_num, history[i].cmdline);
        }
    }
    else if (queue_size == MAXHIST){
        int index = queue_next;
        for (int i = 0; i < queue_size; i++){
            index = index % MAXHIST;
            printf("\t%d\t%s",history[i].cmd_num, history[i].cmdline);
            index++;
        }
    }
}
/*
 * finds and returns the cmdline that is called by the bash
 * copy_cmdline: the cmdline that is going to be parsed
 * searched_cmd_number: the bash number that the user is now searching
 * return: if found, the cmdline stored in the history, then it will be parsed by
 * the parsing function. If not found, it returns the original cmdline entered
 */
char* redo_a_cmd(char* copy_cmdline,int searched_cmd_number){
    int found = 0;
    int index = 0;
    for (int i = 0; i < queue_size; i++){
        if (history[i].cmd_num == searched_cmd_number){
            found = 1;
            index = i;
            break;
        }
    }
    if (found == 1){
        strcpy(copy_cmdline,history[index].cmdline);
    }
    else{
        printf("bash: !%d: event not found\n",searched_cmd_number);
    }
    return copy_cmdline;
}


void sigchld_handler(int sig) {
    int wait_res = 1;
    while (wait_res != -1 && wait_res != 0) {
        wait_res = waitpid(-1, NULL, WNOHANG);
    }
}
