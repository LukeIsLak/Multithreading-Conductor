#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#include "mdata.h"
#include "train.h"
#include "file.h"

pqueue* east_to_west;
pqueue* west_to_east;
timespec start_time = {0};

void create_mutexes();
void destroy_mutexes();
void start_trains();
void parse_input_to_trains(char* filepath, int size, pthread_t tid[], FILE* outptr);
int determine_size(char* filepath);

pqueue_node* dequeue_eastbound();
pqueue_node* dequeue_westbound();
pqueue_node* train_logic();

int cur_size;
int recieved_signal = 0;

int main (int argc, char* argv[]) {

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    if (argc != 2) {
        printf("ERROR: Only expecing file call and filename as arguements\n");
        exit(1);
    }

    FILE* outptr = fopen("output.txt", "w");
    if (outptr == NULL) {
        printf("ERROR: Could not open output.txt\n");
        exit(1);
    } 

    create_mutexes();
    int size = determine_size(argv[1]);
    pthread_t tid[size];

    parse_input_to_trains(argv[1], size, tid, outptr);

    while (cur_size != size);

    start_trains();

    int i = 0;
    while(i < size) {
        pthread_mutex_lock(&queue);

        while (east_to_west->num_elements == 0 && west_to_east->num_elements == 0) {
            recieved_signal = 1;
            pthread_cond_wait(&train_added, &queue);
        }
        pthread_mutex_unlock(&queue);
        while (recieved_signal) {
            timespec wait = {0};
            wait.tv_nsec = 6000;
            nanosleep(&wait, NULL);
            recieved_signal = 0;
        }
        pthread_mutex_lock(&queue);
        pthread_mutex_lock(&station);
        pthread_mutex_lock(&west_to_east->lock);
        pthread_mutex_lock(&east_to_west->lock);
        pqueue_node* train_to_load = train_logic();
        pthread_mutex_lock(&train_to_load->train_mutex);
        pthread_cond_signal(&train_to_load->start); //ISSUE HERE
        train_to_load->can_cross = 1;
        pthread_mutex_unlock(&queue);
        pthread_mutex_unlock(&train_to_load->train_mutex);
        pthread_mutex_unlock(&east_to_west->lock);
        pthread_mutex_unlock(&west_to_east->lock);
        pthread_mutex_lock(&track);
        if (train_to_load != NULL) {
            while (!train_to_load->crossed) {
                pthread_cond_wait(&crossed, &track);
            }
        }
        pthread_mutex_unlock(&track);
        pthread_mutex_unlock(&station);
        ++i;
    }

    for(size_t i = 0; i < size; ++i) {
		pthread_join(tid[i], NULL);
	}

    destroy_mutexes();
    return 0;
}
/** 
*@brief this function signals to all threads to start loading
*/
void start_trains() {
    pthread_mutex_lock(&start_timer);
	ready_to_load = 1;
	pthread_cond_broadcast(&train_ready_to_load);
	pthread_mutex_unlock(&start_timer);
}

/** 
*@brief this function returns a train from the west to east queue, modifies corresponding data
*@return a train from the top of the queue
*/
pqueue_node* dequeue_eastbound() {
    pqueue_node* return_node;
    return_node = dequeue(west_to_east);
    last_dir = cur_dir;
    cur_dir = 0;
    return return_node;
}

/** 
*@brief this function returns a train from the east to west queue, modifies corresponding data
*@return a train from the top of the queue
*/
pqueue_node* dequeue_westbound() {
    pqueue_node* return_node;
    return_node = dequeue(east_to_west);
    last_dir = cur_dir;
    cur_dir = 1;
    return return_node;
}

/** 
*@brief this function determines which train to take out of the two queues based on our implementation from P2
*@return the train to 
*/
pqueue_node* train_logic() {
    pqueue_node* return_node;
    if (west_to_east->num_elements == 0 && east_to_west->num_elements > 0) return_node = dequeue_westbound();
    else if (west_to_east->num_elements > 0 && east_to_west->num_elements == 0) return_node = dequeue_eastbound();
    else if (cur_dir != -1 && last_dir == cur_dir) {
        if (cur_dir) return_node = dequeue_eastbound();
        else return_node = dequeue_westbound();
    }
    else if (peek_priority(west_to_east) == 1 && peek_priority(east_to_west) == 0) return_node = dequeue_eastbound();
    else if (peek_priority(west_to_east) == 0 && peek_priority(east_to_west) == 1) return_node = dequeue_westbound();
    else if (cur_dir == -1) {
        return_node = dequeue_westbound();
    }
    else {
        if (west_to_east->root->load_time == east_to_west->root->load_time) {
            if (cur_dir == -1) {
                return_node = dequeue_westbound();
            }
            if (west_to_east->root->tid < east_to_west->root->tid) {
                return_node = dequeue_eastbound();
            }
            else {
                return_node = dequeue_westbound();
            }
        }
        else {
            if (cur_dir) return_node = dequeue_eastbound();
            else return_node = dequeue_westbound();
        }
    }
    return return_node;
}