/*
 * Swarthmore College, CS 31
 * Copyright (c) 2022 Swarthmore College Computer Science Department,
 * Swarthmore PA
 */

/*
 * This program simulates John Conway's Game of Life.
 * We separated the main functionalities into 4 parts: reading the file,
 * initializing the board, iterating through the steps, and printing the game
 * state at each step (if specified).
 */

/*
 * To run:
 * ./gol file1.txt  0  # run with config file file1.txt, do not print board
 * ./gol file1.txt  1  # run with config file file1.txt, ascii animation
 * ./gol file1.txt  2  # run with config file file1.txt, ParaVis animation
 *
 */
#include <pthreadGridVisi.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include "colors.h"

/****************** Definitions **********************/
/* Three possible modes in which the GOL simulation can run */
#define OUTPUT_NONE   (0)   // with no animation
#define OUTPUT_ASCII  (1)   // with ascii animation
#define OUTPUT_VISI   (2)   // with ParaVis animation

/* Used to slow down animation run modes: usleep(SLEEP_USECS);
 * Change this value to make the animation run faster or slower
 */
//#define SLEEP_USECS  (1000000)
#define SLEEP_USECS    (50000)

/* A global variable to keep track of the number of live cells in the
 * world (this is the ONLY global variable you may use in your program)
 */
static int total_live = 0;

/* This struct represents all the data you need to keep track of your GOL
 * simulation.  Rather than passing individual arguments into each function,
 * we'll pass in everything in just one of these structs.
 * this is passed to play_gol, the main gol playing loop
 *
 * NOTE: You will need to use the provided fields here, but you'll also
 *       need to add additional fields. (note the nice field comments!)
 * NOTE: DO NOT CHANGE THE NAME OF THIS STRUCT!!!!
 */
struct gol_data {

    // NOTE: DO NOT CHANGE the names of these 4 fields (but USE them)
    int rows;  // the row dimension
    int cols;  // the column dimension
    int iters; // number of iterations to run the gol simulation
    int output_mode; // set to:  OUTPUT_NONE, OUTPUT_ASCII, or OUTPUT_VISI

    int *Current_grid;
    int *Next_grid; 
    time_t ini_sec;
    suseconds_t ini_usec ;

    /* fields used by ParaVis library (when run in OUTPUT_VISI mode). */
    // NOTE: DO NOT CHANGE their definitions BUT USE these fields
    visi_handle handle;
    color3 *image_buff;
};
/* the main gol game playing loop (prototype must match this) */
void play_gol(struct gol_data *data);

/* init gol data from the input file and run mode cmdline args */
int init_game_data_from_args(struct gol_data *data, char **argv);

// A mostly implemented function, but a bit more for you to add.
/* print board to the terminal (for OUTPUT_ASCII mode) */
void print_board(struct gol_data *data, int round);

/*Generate the board and spawn new cells*/
void generate_board(struct gol_data *data);

/*Fill a board array with 0s*/
void clear_board(int *board, int width, int height);

/*Initialize the new cells in the board*/
void spawn_new_cells(int y, int x, int alive, struct gol_data *data);

/*Getting the count of neighbors of one cell*/
int get_neighbor_count(int x, int y, struct gol_data *data);

/*update the colors in ParaVisi*/
void update_colors(struct gol_data *data) ;

/************ Definitions for using ParVisi library ***********/
/* initialization for the ParaVisi library (DO NOT MODIFY) */
int setup_animation(struct gol_data* data);
/* register animation with ParaVisi library (DO NOT MODIFY) */
int connect_animation(void (*applfunc)(struct gol_data *data),
        struct gol_data* data);
/* name for visi (you may change the string value if you'd like) */
static char visi_name[] = "GOL!";


int main(int argc, char **argv) {

    int ret;
    struct gol_data data;
    double secs;

    /* check number of command line arguments */
    if (argc < 3) {
        printf("usage: %s <infile.txt> <output_mode>[0|1|2]\n", argv[0]);
        printf("(0: no visualization, 1: ASCII, 2: ParaVisi)\n");
        exit(1);
    }

    /* Initialize game state (all fields in data) from information
     * read from input file */
    ret = init_game_data_from_args(&data, argv);
    if (ret != 0) {
        printf("Initialization error: file %s, mode %s\n", argv[1], argv[2]);
        exit(1);
    }
    
    /* initialize ParaVisi animation (if applicable) */
    if (data.output_mode == OUTPUT_VISI) {
        setup_animation(&data);
    }

    /* ASCII output: clear screen & print the initial board */
    if (data.output_mode == OUTPUT_ASCII) {
        if (system("clear")) { perror("clear"); exit(1); }
        print_board(&data, 0);
    }

    /* Invoke play_gol in different ways based on the run mode */
    if (data.output_mode == OUTPUT_NONE) {  // run with no animation
        play_gol(&data);
    }
    else if (data.output_mode == OUTPUT_ASCII) { // run with ascii animation
        play_gol(&data);

        // clear the previous print_board output from the terminal:
        // (NOTE: you can comment out this line while debugging)
        if (system("clear")) { perror("clear"); exit(1); }

        // NOTE: DO NOT modify this call to print_board at the end
        //       (it's to help us with grading your output)
        print_board(&data, data.iters);
    }
    else {  // OUTPUT_VISI: run with ParaVisi animation
            // tell ParaVisi that it should run play_gol
        connect_animation(play_gol, &data);
        // start ParaVisi animation
        run_animation(data.handle, data.iters);
    }

    // NOTE: you need to determine how and where to add timing code
    //       in your program to measure the total time to play the given
    //       number of rounds played.
    if (data.output_mode != OUTPUT_VISI) {
        secs = 0.0;

        /* Print the total runtime, in seconds. */
        // NOTE: do not modify these calls to fprintf;
        struct timeval current_time;
        gettimeofday(&current_time, NULL);
        secs = (current_time.tv_sec+current_time.tv_usec*0.000001)-(data.ini_sec+data.ini_usec*0.000001);
        fprintf(stdout, "Total time: %0.3f seconds\n", secs);
        fprintf(stdout, "Number of live cells after %d rounds: %d\n\n",
                data.iters, total_live);
    }

    free(data.Current_grid);
    free(data.Next_grid);
    data.Current_grid = NULL;
    data.Next_grid = NULL;

    return 0;
}

/* initialize the gol game state from command line arguments
 *       argv[1]: name of file to read game config state from
 *       argv[2]: run mode value
 * data: pointer to gol_data struct to initialize
 * argv: command line args
 *       argv[1]: name of file to read game config state from
 *       argv[2]: run mode
 * returns: 0 on success, 1 on error
 */
int init_game_data_from_args(struct gol_data *data, char **argv) {
    int ret; //the return of the fileread
    FILE *infile;
    infile = fopen(argv[1], "r");
    data->output_mode = atoi(argv[2]);
    if (infile == NULL) {
        printf("Error: failed to open file: %s\n", argv[1]);
        exit(1);
    }
    ret = fscanf(infile, "%d", &data->rows);
    if (ret == 0) {
        printf("Improper file format.\n");
        exit(1);
    }
    ret = fscanf(infile, "%d", &data->cols);
    if (ret == 0) {
        printf("Improper file format.\n");
        exit(1);
    }
    ret = fscanf(infile, "%d", &data->iters);
    if (ret == 0) {
        printf("Improper file format.\n");
        exit(1);
    }
    ret = fscanf(infile, "%d", &total_live);
    if (ret == 0) {
        printf("Improper file format.\n");
        exit(1);
    }

    /*Genterate the board and spawn new cells*/
    generate_board(data);

    /*Spawning new cells*/
    for (int n = 0; n < total_live; n++){
        int y = 0;
        int x = 0;
        int ret1 = 0;
        int ret2 = 0;
        ret1 = fscanf(infile, "%d", &y);
        if (ret1 == 0) {
            printf("Improper file format.\n");
            exit(1);
        }
        ret2 = fscanf(infile, "%d", &x);
        if (ret2 == 0) {
            printf("Improper file format.\n");
            exit(1);
        }
        spawn_new_cells(y,x,1,data);
    }

    /*Get the time of the day*/
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    data->ini_sec = current_time.tv_sec;
    data->ini_usec = current_time.tv_usec;

    ret = fclose(infile);
    if (ret != 0) {
        printf("Error: failed to close file: %s\n", argv[1]);
        exit(1);
    }
    

    return 0;
}

/*The function that generates the board
 * data: pointer to gol_data struct to initialize
 */
void generate_board(struct gol_data *data){
    int rows = data->rows;
    int cols = data->cols;
  
    data->Current_grid = NULL;
    data->Current_grid = malloc(cols*rows*sizeof(int));
    data->Next_grid = NULL;
    data->Next_grid = malloc(cols*rows*sizeof(int));
    clear_board(data->Current_grid,rows,cols);
    clear_board(data->Next_grid,rows,cols);
}

/*This function replaces all places on a board array with 0
 * board: the array representing the board to be cleared
 * width: number of columns
 * height: number of rows
 */
void clear_board(int *board, int width, int height){
    for(int n = 0;n < width*height; n++){
        board[n] = 0;
    }
}

/*The function that spawns new cells during initialization
 * y: the row # of the cell to spawn
 * x: the column # of the cell to spawn
 * alive: whether this cell will be set to alive (1) or dead (0)
 * data: pointer to gol_data struct to initialize
 */
void spawn_new_cells(int y, int x, int alive, struct gol_data *data){
    int cols = data->cols;
    int offset;
    offset = y * cols + x;
    data->Current_grid[offset] = alive;
}

/*This function calculates the number of live neighbors for a given cell.
 * y: the row # of the cell to check
 * x: the column # of the cell to check
 * data: pointer to gol_data struct to initialize
 * return: the number of live neighbors
 */
int get_neighbor_count(int y, int x, struct gol_data *data) {
    int live_count = 0;
    int rows = data->rows;
    int cols = data->cols;

    for (int i = y-1; i < y+2; i++) {
        for (int j = x-1; j < x+2; j++) {
            live_count += data->Current_grid[((i+rows) % rows)*cols + (j+cols)%cols];
        }
    }
    live_count -= data->Current_grid[y*cols + x];

    return live_count;
}


/* the gol application main loop function:
 *  runs rounds of GOL,
 *    * updates program state for next round (world and total_live)
 *    * performs any animation step based on the output/run mode
 *
 *   data: pointer to a struct gol_data  initialized with
 *         all GOL game playing state
 */
void play_gol(struct gol_data *data) {
    int state;
    int index;
    int rows = data->rows;
    int cols = data->cols;
    int iters = data->iters;
    int output_mode = data->output_mode;
    
    for (int n = 1; n <= iters; n++) {
        if (system("clear")) { perror("clear"); exit(1); }
        total_live=0;
        state = 0;
        index = 0;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                index = i*cols + j;
                state = data->Current_grid[index];
                int neighbor_count = get_neighbor_count(i, j, data);
                if (state == 1) {
                    if (neighbor_count <= 1 || neighbor_count >= 4) {
                        data->Next_grid[index] = 0;
                    } else {
                        total_live++;
                        data->Next_grid[index] = 1;
                    }
                } else {
                    if (neighbor_count == 3) {
                        total_live++;
                        data->Next_grid[index] = 1;
                    } else {
                        data->Next_grid[index] = 0;
                    }
                }
            }
        }
        int *temp = data->Current_grid;
        data->Current_grid = data->Next_grid;
        data->Next_grid = temp;
        if (output_mode==OUTPUT_ASCII){
            print_board(data, n);
            usleep(SLEEP_USECS);
        }    
        else if (output_mode==OUTPUT_VISI){
            update_colors(data);
            draw_ready(data->handle);
            usleep(SLEEP_USECS);
        }    
        clear_board(data->Next_grid, data->rows, data->cols);
    }

}

/* Print the board to the terminal.
 *   data: gol game specific data
 *   round: the current round number
 */
void print_board(struct gol_data *data, int round) {
    int i, j;
    int rows = data->rows;
    int cols = data->cols;

    /* Print the round number. */
    fprintf(stderr, "Round: %d\n", round);

    for (i = 0; i < rows; ++i) {
        for (j = 0; j < cols; ++j) {
            if (data->Current_grid[i*cols+j] == 1){
                fprintf(stderr, " @");
            }
            else{
                fprintf(stderr, " .");
            }
        }
        fprintf(stderr, "\n");
    }
    /* Print the total number of live cells. */
    fprintf(stderr, "Live cells: %d\n\n", total_live);
}

/*ParaVisi update colors
 * data: gol game specific data
 */
void update_colors(struct gol_data *data) {
    int i, j, rows, cols, index, buff_i;
    color3 *buff;

    buff = data->image_buff;  // just for readability
    rows = data->rows;
    cols = data->cols;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            index = i*cols + j;
            buff_i = (rows - (i+1))*cols + j;

            // update animation buffer
            if (data->Current_grid[index] == 1){
                buff[buff_i] = c3_red;
            }
            else{
                buff[buff_i] = c3_green;
            }                       
        }
    }
}


/**********************************************************/
/***** START: DO NOT MODIFY THIS CODE *****/
/* initialize ParaVisi animation */
int setup_animation(struct gol_data* data) {
    /* connect handle to the animation */
    int num_threads = 1;
    data->handle = init_pthread_animation(num_threads, data->rows,
            data->cols, visi_name);
    if (data->handle == NULL) {
        printf("ERROR init_pthread_animation\n");
        exit(1);
    }
    // get the animation buffer
    data->image_buff = get_animation_buffer(data->handle);
    if(data->image_buff == NULL) {
        printf("ERROR get_animation_buffer returned NULL\n");
        exit(1);
    }
    return 0;
}

/* sequential wrapper functions around ParaVis library functions */
void (*mainloop)(struct gol_data *data);

void* seq_do_something(void * args){
    mainloop((struct gol_data *)args);
    return 0;
}

int connect_animation(void (*applfunc)(struct gol_data *data),
        struct gol_data* data)
{
    pthread_t pid;

    mainloop = applfunc;
    if( pthread_create(&pid, NULL, seq_do_something, (void *)data) ) {
        printf("pthread_created failed\n");
        return 1;
    }
    return 0;
}
/***** END: DO NOT MODIFY THIS CODE *****/
/******************************************************/
