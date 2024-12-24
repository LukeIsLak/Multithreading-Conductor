#ifndef MDATA
#define MDATA

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "pqueue.h"

pthread_mutex_t start_timer;
pthread_cond_t train_ready_to_load;

pthread_mutex_t station;
pthread_cond_t train_added;

pthread_cond_t crossed;

pthread_mutex_t track;
pthread_mutex_t queue;

int ready_to_load = 0;

extern pqueue* west_to_east;
extern pqueue* east_to_west;

/*dir 1 impllies direction is Westbound, dir 0 implies direction is Eastbound*/
int last_dir = -1;
int cur_dir = -1;


/** 
*@brief this function creates and initializes all pqueues and mutexes
*/
void create_mutexes() {
    west_to_east = create_pqueue();
    east_to_west = create_pqueue();

    pthread_mutex_init(&west_to_east->lock, NULL);
    pthread_mutex_init(&east_to_west->lock, NULL);
    pthread_mutex_init(&track, NULL);
    pthread_mutex_init(&station, NULL);
    pthread_mutex_init(&start_timer, NULL);
    pthread_mutex_init(&queue, NULL);

    pthread_cond_init(&train_ready_to_load, NULL);
    pthread_cond_init(&west_to_east->cond, NULL);
    pthread_cond_init(&east_to_west->cond, NULL);
    pthread_cond_init(&train_added, NULL);
    pthread_cond_init(&crossed, NULL);
}

/** 
*@brief this function destroys all mutexes
*/
void destroy_mutexes() {
    pthread_mutex_destroy(&east_to_west->lock);
    pthread_mutex_destroy(&west_to_east->lock);
    pthread_mutex_destroy(&track);
    pthread_mutex_destroy(&station);
    pthread_mutex_destroy(&start_timer);
    pthread_mutex_destroy(&queue);

    pthread_cond_destroy(&train_ready_to_load);
    pthread_cond_destroy(&west_to_east->cond);
    pthread_cond_destroy(&east_to_west->cond);
    pthread_cond_destroy(&train_added);
    pthread_cond_destroy(&crossed);
}

#endif