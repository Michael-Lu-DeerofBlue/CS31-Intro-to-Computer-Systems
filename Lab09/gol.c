/*
 * Swarthmore College, CS 31
 * Copyright (c) 2022 Swarthmore College Computer Science Department,
 * Swarthmore PA
 */

// This program runs the Game of Life using multithreading.

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
#define SLEEP_USECS    (100000)

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
    int *partition_nodes;
    int part_in_row_col;
    int num_threads;
    int print_part_or_not;
    int my_id;

    int start_rows, start_cols;
    int end_rows, end_cols;

    /* fields used by ParaVis library (when run in OUTPUT_VISI mode). */
    // NOTE: DO NOT CHANGE their definitions BUT USE these fields
    visi_handle handle;
    color3 *image_buff;
};


/****************** Function Prototypes **********************/
/* the main gol game playing loop (prototype must match this) */
void *play_gol(void *args);

/* init gol data from the input file and run mode cmdline args */
int init_game_data_from_args(struct gol_data *data, char **argv);

// A mostly implemented function, but a bit more for you to add.
/* print board to the terminal (for OUTPUT_ASCII mode) */
void print_board(struct gol_data *data, int round);

/*Generate the board and spawn new cells*/
void generate_board(struct gol_data *data);

/*Doing the partition*/
void partition(struct gol_data *data);

/*Initialize the new cells in the board*/
void spawn_new_cells(int x, int y, int alive, struct gol_data *data);

/*Getting the count of neighbors of one cell*/
int get_neighbor_count(int x, int y, struct gol_data *data);

/*update the colors in ParaVisi*/
void update_colors(struct gol_data *data) ;

//struct gol_data *datacpy(struct gol_data * data);

/************ Definitions for using ParVisi library ***********/
/* initialization for the ParaVisi library (DO NOT MODIFY) */
int setup_animation(struct gol_data* data);
/* register animation with ParaVisi library (DO NOT MODIFY) */
int connect_animation(void (*applfunc)(struct gol_data *data),
        struct gol_data* data);
/* name for visi (you may change the string value if you'd like) */
static char visi_name[] = "GOL!";
/* Mutex */
static pthread_mutex_t my_mutex;
/* Barrier */
static pthread_barrier_t my_barrier;

int main(int argc, char **argv) {

    int ret;
    struct gol_data data;
    struct gol_data *tid_args;
    double secs;
    pthread_t *tids;

    /* check number of command line arguments */
    if (argc < 6) {
        printf("usage: %s <infile.txt> <output_mode>[0|1|2] <num_threads> <partition>[0,1] <print_partition>[0,1]\n", argv[0]);
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

    /*Do the board partition*/
    partition(&data);
    tids = malloc(sizeof(pthread_t) * data.num_threads);
    tid_args = malloc(sizeof(struct gol_data) * data.num_threads);
    if (pthread_barrier_init(&my_barrier, NULL, data.num_threads)) {
        printf("pthread_barrier_init error\n");
        exit(1);
    }
     /* Initialize the mutex. */
    if (pthread_mutex_init(&my_mutex, NULL)) {
        printf("pthread_mutex_init error\n");
        exit(1);
    }
    /* initialize ParaVisi animation (if applicable) */
    if (data.output_mode == OUTPUT_VISI) {
        //setup_animation(&data);
        data.handle = init_pthread_animation(data.num_threads, data.rows, data.cols, visi_name);
        if (data.handle == NULL) {
            printf("ERROR init_pthread_animation\n");
            exit(1);
        }
        // get the animation buffer
        data.image_buff = get_animation_buffer(data.handle);
        if(data.image_buff == NULL) {
            printf("ERROR get_animation_buffer returned NULL\n");
            exit(1);
        }
    }

    /* ASCII output: clear screen & print the initial board */
    if (data.output_mode == OUTPUT_ASCII) {
        if (system("clear")) { perror("clear"); exit(1); }
        print_board(&data, 0);
    }

    /* Copy data to each thread's arguments */
    for (int i = 0; i < data.num_threads; i++) {
        tid_args[i].rows = data.rows;
        tid_args[i].cols = data.cols;
        tid_args[i].iters = data.iters;
        tid_args[i].output_mode = data.output_mode;
        tid_args[i].Current_grid = data.Current_grid;
        tid_args[i].Next_grid = data.Next_grid;
        tid_args[i].ini_sec = data.ini_sec;
        tid_args[i].ini_usec = data.ini_usec;
        tid_args[i].partition_nodes = data.partition_nodes;
        tid_args[i].part_in_row_col = data.part_in_row_col;
        tid_args[i].num_threads = data.num_threads;
        tid_args[i].print_part_or_not = data.print_part_or_not;
        tid_args[i].my_id = i;
        tid_args[i].handle = data.handle;
        tid_args[i].image_buff = data.image_buff;
        if (data.part_in_row_col) { // if columns are partitioned
            tid_args[i].start_cols = data.partition_nodes[i];
            if (i == data.num_threads - 1) {
                tid_args[i].end_cols = data.cols - 1;
            } else {
                tid_args[i].end_cols = data.partition_nodes[i + 1] - 1;
            }
            tid_args[i].start_rows = 0;
            tid_args[i].end_rows = data.rows - 1;
        } else { // if rows are partitioned
            tid_args[i].start_rows = data.partition_nodes[i];
            if (i == data.num_threads - 1) {
                tid_args[i].end_rows = data.rows - 1;
            }
            else {
                tid_args[i].end_rows = data.partition_nodes[i + 1] - 1;
            }
            tid_args[i].start_cols = 0;
            tid_args[i].end_cols = data.cols - 1;
        }
    }

    /* Invoke play_gol in different ways based on the run mode */
    if (data.output_mode == OUTPUT_NONE) {  // run with no animation
        for (int i = 0; i < data.num_threads; i++) {
            ret = pthread_create(&tids[i], 0, play_gol, &tid_args[i]);
            if (ret) {
                perror("Error pthread_create\n");
                exit(1);
            }
        }
        for (int i = 0; i < data.num_threads; i++) {
            pthread_join(tids[i], 0);
        }
    }
    else if (data.output_mode == OUTPUT_ASCII) { // run with ascii animation
        if (system("clear")) { perror("clear"); exit(1); }
        for (int i = 0; i < data.num_threads; i++) {
            ret = pthread_create(&tids[i], 0, play_gol, &tid_args[i]);
            if (ret) {
                perror("Error pthread_create\n");
                exit(1);
            }
        }
        for (int i = 0; i < data.num_threads; i++) {
            pthread_join(tids[i], 0);
        }
        if (system("clear")) { perror("clear"); exit(1); }
        print_board(&data, data.iters);
    }
    else {  // OUTPUT_VISI: run with ParaVisi animation
            // tell ParaVisi that it should run play_gol
        for (int i = 0; i < data.num_threads; i++) {
            ret = pthread_create(&tids[i], 0, play_gol, &tid_args[i]);
            if (ret) {
                perror("Error pthread_create\n");
                exit(1);
            }
        }
        run_animation(data.handle, data.iters);
        for (int i = 0; i < data.num_threads; i++) {
            pthread_join(tids[i], 0);
        }
    }

    if (data.output_mode != OUTPUT_VISI) {
        secs = 0.0;
        struct timeval current_time;
        gettimeofday(&current_time, NULL);
        secs = (current_time.tv_sec+current_time.tv_usec*0.000001)-(data.ini_sec+data.ini_usec*0.000001);
        fprintf(stdout, "Total time: %0.3f seconds\n", secs);
        fprintf(stdout, "Number of live cells after %d rounds: %d\n\n",
                data.iters, total_live);
    }

    free(data.Current_grid);
    free(data.Next_grid);
    free(data.partition_nodes);
    free(tids);
    free(tid_args);
    data.partition_nodes = NULL;
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
    data->num_threads = atoi(argv[3]);
    data->part_in_row_col = atoi(argv[4]);
    data->print_part_or_not = atoi(argv[5]);
    /*Genterate the board and spawn new cells*/
    generate_board(data);

    /*Spawning new cells*/
    for (int i = 0; i < total_live; i++){
        int x = 0;
        int y = 0;
        int ret1 = 0;
        int ret2 = 0;
        ret1 = fscanf(infile, "%d", &x);
        if (ret1 == 0) {
            printf("Improper file format.\n");
            exit(1);
        }
        ret2 = fscanf(infile, "%d", &y);
        if (ret2 == 0) {
            printf("Improper file format.\n");
            exit(1);
        }
        spawn_new_cells(x,y,1,data);
        //printf("row: %d col:%d\n",x,y);
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
/*The function that generates the board*/
void generate_board(struct gol_data *data){
    data->Current_grid = NULL;
    data->Current_grid = malloc(data->cols*data->rows*sizeof(int));
    data->Next_grid = NULL;
    data->Next_grid = malloc(data->cols*data->rows*sizeof(int));
    for(int i = 0;i < data->cols*data->rows; i++){
        data->Current_grid[i] = 0;
        data->Next_grid[i] = 0;
    }
    data->partition_nodes = malloc(data->num_threads*sizeof(int));
}

/* The function that calculates the partitioning of the board */
void partition(struct gol_data *data){
    int each_part_has = 0;
    int needed_extra = 0;
    if (data->part_in_row_col == 1){
        each_part_has = data->cols / data->num_threads;
        needed_extra = data->cols % data->num_threads;
    }
    else{
        each_part_has = data->rows / data->num_threads;
        needed_extra = data->rows % data->num_threads;
    }

    int index = 0;
    data->partition_nodes[0]=0;
    for (int i = 1; i < data->num_threads; i++){
        if (needed_extra > 0) {
            data->partition_nodes[i] = index + each_part_has + 1;
            needed_extra--;
        } else {
            data->partition_nodes[i] = index + each_part_has;
        }
        index = data->partition_nodes[i];
    }
}

/* This function sets all board values to 0 */
void clear_board(int *board, int width, int height){
    for(int i = 0;i < width*height; i++){
        board[i] = 0;
    }
}

/*The function that spawns new cells*/ 
void spawn_new_cells(int x, int y, int alive, struct gol_data *data){
    int offset;
    offset = x * data->cols + y;
    data->Current_grid[offset] = alive;
}

int get_neighbor_count(int x, int y, struct gol_data *data) {
    int live_count = 0;

    for (int i = x-1; i < x+2; i++) {
        for (int j = y-1; j < y+2; j++) {
            live_count += data->Current_grid[((i+data->rows) % data->rows)*data->cols + (j+data->cols)%data->cols];
        }
    }
    live_count -= data->Current_grid[x*data->cols + y];

    return live_count;
}


/* the gol application thread loop function:
 *  runs rounds of GOL,
 *    * updates program state for next round (world and total_live)
 *    * for current thread;
 *    * performs any animation step based on the output/run mode
 *
 *   args: pointer to a struct gol_data
 */
void *play_gol(void *args) {
    struct gol_data *data;
    data = (struct gol_data *) args;

    int start_rows = data->start_rows;
    int end_rows = data->end_rows;
    int start_cols = data->start_cols;
    int end_cols = data->end_cols;
    if (data->print_part_or_not){
        printf("tid %d: rows: %d:%d (%d) \tcols: %d:%d (%d)\n", data->my_id, start_rows,
    end_rows, end_rows - start_rows + 1, start_cols, end_cols,
    end_cols - start_cols + 1);
    }  
    pthread_barrier_wait(&my_barrier);
    int state;
    int index;
    int delta_life;
    for (int n = 1; n <= data->iters; n++) {
        state = 0;
        index = 0;
        delta_life = 0;
        for (int i = start_rows; i <= end_rows; i++) {
            for (int j = start_cols; j <= end_cols; j++) {
                index = i*data->cols + j;
                state = data->Current_grid[index];
                int neighbor_count = get_neighbor_count(i, j, data);
                if (state == 1) {
                    if (neighbor_count <= 1 || neighbor_count >= 4) {
                        data->Next_grid[index] = 0;
                    } else {
                        delta_life++;
                        data->Next_grid[index] = 1;
                    }
                } else {
                    if (neighbor_count == 3) {
                        delta_life++;
                        data->Next_grid[index] = 1;
                    } else {
                        data->Next_grid[index] = 0;
                    }
                }
            }
        }
        pthread_barrier_wait(&my_barrier);
        pthread_mutex_lock(&my_mutex);
        total_live = total_live + delta_life; 
        pthread_mutex_unlock(&my_mutex);
        pthread_barrier_wait(&my_barrier);
        if (data->output_mode==OUTPUT_VISI){
            update_colors(data);
            draw_ready(data->handle);
        }
        pthread_barrier_wait(&my_barrier);
        int *temp = data->Current_grid;
        data->Current_grid = data->Next_grid;
        data->Next_grid = temp;
        if (data->my_id == 0){
            if (data->output_mode==OUTPUT_ASCII){
                
                print_board(data, n);
                usleep(SLEEP_USECS);
                clear_board(data->Next_grid, data->rows, data->cols);
            }    
            else if (data->output_mode==OUTPUT_VISI){
                usleep(SLEEP_USECS);
            }
            if (n < data->iters){
                total_live = 0;
            }
        }
        pthread_barrier_wait(&my_barrier);
    }
    return (void *)0; 
}

/* Print the entire board to the terminal.
 *   data: gol game specific data
 *   round: the current round number
 *
 * NOTE: You may add extra printfs if you'd like, but please
                        printf("here +1\n");
 *       leave these fprintf calls exactly as they are to make
 *       grading easier!
 */
void print_board(struct gol_data *data, int round) {
    int i, j;
    if (system("clear")) { perror("clear"); exit(1); }
    /* Print the round number. */
    fprintf(stderr, "Round: %d\n", round);

    for (i = 0; i < data->rows; ++i) {
        for (j = 0; j < data->cols; ++j) {
            if (data->Current_grid[i*data->cols+j] == 1){
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

/* Update Paravisi colors specific to thread*/
void update_colors(struct gol_data *data) {
    int i, j, r, c, index, buff_i;
    color3 *buff;
    buff = data->image_buff;  // just for readability
    r = data->rows;
    c = data->cols;

    for (i = data->start_rows; i <= data->end_rows; i++) {
        for (j = data->start_cols; j <= data->end_cols; j++) {
            index = i*c + j;
            buff_i = (r - (i+1))*c + j;

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
