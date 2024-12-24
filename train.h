#ifndef TRAIN
#define TRAIN

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#include "mdata.h"  

#define NANOSECOND_CONVERSION 1e9

extern pthread_mutex_t start_timer;
extern pthread_cond_t train_ready_to_load;
extern struct timespec start_time;

typedef struct data_package {
        pqueue_node* node;
        pthread_cond_t local_convar;
        pqueue* eastbound;
        pqueue* westbound;
        FILE* outptr;
    
} data_package;

typedef struct timespec timespec;

/** 
*@brief this function converts a timespec into a double amount of seconds
*@param time is a timespec pointer we wish to conver
*@return a double amount of seconds
*/
double timespec_to_seconds (timespec* time) {
    return ((double) time->tv_sec) + (((double) time->tv_nsec) / NANOSECOND_CONVERSION);
}

/** 
*@brief this function adds a train to a queue
*@param node is the train we wish to add
*@param data is a struct containing the queues we can add to
*/
void add_train_to_queue(pqueue_node* node, data_package* data) {
    pthread_mutex_lock(&queue);
    if (strcmp(node->direction_s, "East") == 0) {
        while (!data->eastbound->open) {
            pthread_cond_wait(&data->eastbound->cond, &data->eastbound->lock);
        }   
        enqueue(data->eastbound, node);
    }
    else if (strcmp(node->direction_s, "West") == 0) {
        
        while (!data->westbound->open) {
            pthread_cond_wait(&data->westbound->cond, &data->westbound->lock);
        }
        enqueue(data->westbound, node);
    }

    pthread_cond_signal(&train_added);
    pthread_mutex_unlock(&queue);

}

/** 
*@brief this function prints out the relative time of now to when the program started
*@param start is a timespec which contains data when the program started
*@param data is the train struct which contains details for the train
*@param state is an integer which controls which statement is printed: 1 -> ready to go, 2 -> on the track, 3 -> off the track
*@param outptr is the FILE* to the file we output to
*/
void print_relative_time(timespec* start, pqueue_node* data, int state, FILE* outptr) {
    timespec curr_time;
    clock_gettime(CLOCK_MONOTONIC, &curr_time);

    double curr_time_s = timespec_to_seconds(&curr_time);
    double start_time_s = timespec_to_seconds(start);

    double duration = curr_time_s - start_time_s;

    int d_hours = (int)duration / 3600;
    int d_minutes = ((int)duration - (d_hours * 3600)) / 60;
    int d_seconds = ((int)duration - d_hours*3600 - d_minutes*60);
    int d_miliseconds = (int)((duration - (d_hours*3600) - (d_minutes*60) - d_seconds)*10);
    switch (state)
    {
    case 1:
        fprintf(outptr, "%02d:%02d:%02d.%d Train %2d is ready to go %4s\n", d_hours, d_minutes, d_seconds, d_miliseconds, data->tid, data->direction_s);
        break;
    case 2:
        fprintf(outptr, "%02d:%02d:%02d.%d Train %2d is ON the main track going %4s\n", d_hours, d_minutes, d_seconds, d_miliseconds, data->tid, data->direction_s);
        break;
    case 3:
        fprintf(outptr, "%02d:%02d:%02d.%d Train %2d is OFF the main track after going %4s\n", d_hours, d_minutes, d_seconds, d_miliseconds, data->tid, data->direction_s);
        break;
    
    default:
        break;
    }
}

/** 
*@brief this function is the main function for the train thread
*@param start is a timespec which contains data when the program started
*/
void* train(void* t) {
    data_package* data = (data_package*)t;
    pqueue_node* node = ((data_package*)t)->node;

    pthread_mutex_lock(&start_timer);
	while (!ready_to_load) {
		pthread_cond_wait(&train_ready_to_load, &start_timer);
	}
	pthread_mutex_unlock(&start_timer);
    pthread_mutex_lock(&node->train_mutex);
    timespec loadtime;
    loadtime.tv_sec = node->load_time/10;
    loadtime.tv_nsec = ((node->load_time%10) * 100000000);
    pthread_mutex_unlock(&node->train_mutex);
    nanosleep(&loadtime, NULL);
    pthread_mutex_lock(&node->train_mutex);
    print_relative_time(&start_time, node, 1, data->outptr);
    pthread_mutex_unlock(&node->train_mutex);
    
    add_train_to_queue(node, (data_package*)t);

    pthread_mutex_lock(&node->train_mutex);
    while (!node->can_cross) {
        pthread_cond_wait(&node->start, &node->train_mutex);
    }
    pthread_mutex_unlock(&node->train_mutex);
    print_relative_time(&start_time, node, 2, data->outptr);

    pthread_mutex_lock(&track);

    timespec crosstime;
    crosstime.tv_sec = node->cross_time/10;
    crosstime.tv_nsec = ((node->cross_time%10) * 100000000);

    nanosleep(&crosstime, NULL);
    pthread_cond_signal(&crossed);
    node->crossed = 1;
    print_relative_time(&start_time, node, 3, data->outptr);
    pthread_mutex_unlock(&track);

    pthread_exit(NULL);

    destroy_node(node);
    free(t);
}

#endif