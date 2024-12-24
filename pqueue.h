#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_PARSE_BUFFER 10

typedef struct pqueue {

    struct pqueue_node* root;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int num_elements;
    int open;

} pqueue;

typedef struct pqueue_node {

    struct pqueue_node* next;
    struct pqueue_node* prev;

    pthread_cond_t start;
    pthread_mutex_t train_mutex;

    int tid;
    char direction;
    char direction_s[4];
    int priority;
    int load_time;
    int cross_time;

    int can_cross;
    int crossed;


} pqueue_node;

/** 
*@brief this function creates a pqueue;
*/
pqueue* create_pqueue () {
    pqueue* new_pqueue = (pqueue*)malloc(sizeof(pqueue));
    new_pqueue->root = NULL;
    new_pqueue->num_elements = 0;
    new_pqueue->open = 1;
    pthread_mutex_init(&(new_pqueue->lock), NULL);
    return new_pqueue;
}

/** 
*@brief this function adds a node to a queue bases on the specifications listed in P2
*@param direction a char to represent direction
*@param priority a int to represent priority, 1->High priority 0->Low priority
*@param load_time a int to represent load time
*@param cross_time a int to represent cross time
*@return node the node we wish to add to said queue
*/
pqueue_node* create_pqueue_node(char direction, int priority, int load_time, int cross_time) {
    pqueue_node* new_pqueue_node = (pqueue_node*)malloc(sizeof(pqueue_node));
    new_pqueue_node->direction = direction;
    new_pqueue_node->priority = priority;
    new_pqueue_node->load_time = load_time;
    new_pqueue_node->cross_time = cross_time;

    new_pqueue_node->can_cross = 0;
    new_pqueue_node->crossed = 0;

    if (direction == 101 || direction == 69) strcpy(new_pqueue_node->direction_s, "East");
    if (direction == 119 || direction == 87) strcpy(new_pqueue_node->direction_s, "West");

    pthread_cond_init(&new_pqueue_node->start, NULL);
    pthread_mutex_init(&new_pqueue_node->train_mutex, NULL);

    return new_pqueue_node;
}
/** 
*@brief this function creates a node from a string
*@param input a string to parse
*@return a node constructed from the parsed input
*/
pqueue_node* parse_line_to_node(char* input) {
    char direction;
    int load_time;
    int cross_time;

    char* cur_tok;

    cur_tok = strtok(input, " \n");
    direction = cur_tok[0];
    cur_tok = strtok(NULL, " \n");
    load_time = atoi(cur_tok);
    cur_tok = strtok(NULL, " \n");
    cross_time = atoi(cur_tok);

    int priority = 0;
    if ((int)direction < 97) priority = 1;

    return create_pqueue_node(direction, priority, load_time, cross_time);

}
/** 
*@brief this function adds a node to a queue bases on the specifications listed in P2
*@param queue the queue we wish to enqueue
*@param node the node we wish to add to said queue
*/
void enqueue(pqueue* queue, pqueue_node* node) {    
    pthread_mutex_lock(&queue->lock);
    pthread_mutex_lock(&node->train_mutex);
    queue->open = 0;
    if (queue->root == NULL) {
        queue->root = node;
    }
    else {
        pqueue_node* curr = queue->root;

        if (curr->next == NULL) {
            if (queue->root->priority == 0 && node->priority == 1) {
                node->next = queue->root;
                queue->root->prev = node;
                queue->root = node;
            }
            else if (queue->root->priority == 1 && node->priority == 0) {
                queue->root->next = node;
                node->prev = queue->root;
            }
            else if (queue->root->load_time == node->load_time) {
                if (queue->root->tid > node->tid) {
                    node->next = queue->root;
                    queue->root->prev = node;
                    queue->root = node;
                }
                else {
                    queue->root->next = node;
                    node->prev = queue->root;
                }
            }
            else {
                queue->root->next = node;
                node->prev = queue->root;
            }
        }
        else if (queue->root->priority < node->priority) {
            node->next = queue->root;
            queue->root->prev = node;
            queue->root = node;
        }
        else if (queue->root->priority == node->priority && queue->root->load_time == node->load_time && queue->root->tid > node->tid) {
            node->next = queue->root;
            queue->root->prev = node;
            queue->root = node;
        }
        else {
            while (curr->next != NULL) {
                if (curr->next->priority < node->priority) break;
                if (curr->next->priority == node->priority && curr->next->load_time == node->load_time) {
                    while (curr->next != NULL && curr->next->priority == node->priority && curr->next->load_time == node->load_time && curr->next->tid < node->tid) {
                        curr = curr->next;
                    }
                    break;
                }
                curr = curr->next;
            }
            node->next = curr->next;
            curr->next = node;
            node->prev = curr;
        }
    }

    queue->open = 1;
    queue->num_elements++;
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&node->train_mutex);
    pthread_mutex_unlock(&queue->lock);
}

/** 
*@brief this function returns a node from the front of the queue
*@param queue the queue we wish to pop
*@return a node from the top of the queue
*/
pqueue_node* dequeue(pqueue* queue) {
    if (queue->root == NULL) {
        return NULL;
    }

    queue->num_elements--;
    pqueue_node* return_node = queue->root;
    queue->root = return_node->next;
    if (queue->root != NULL) queue->root->prev = NULL;

    return return_node;
}
/** 
*@brief this function returns the priority of the top element in the queue
*@param queue the queue we wish to peek
*@return the priority of the top element
*/
int peek_priority(pqueue* queue) {
    if (queue->root == NULL) return -1;
    return queue->root->priority;
}

/** 
*@brief this function returns a train from the west to east queue, modifies corresponding data
*@param node a node we want to destroy
*/
void destroy_node(pqueue_node* node) {
    pthread_cond_destroy(&node->start);
    pthread_mutex_destroy(&node->train_mutex);
    free(node->direction_s);
    free(node);
}

/** 
*@brief this function prints the tid of each element in a queue, for debugging purposes
*/
void print_queue(pqueue* queue) {
    pqueue_node* cur = queue->root;
    while (cur != NULL) {
        printf("%d ", cur->tid);
        cur = cur->next;
    }
    printf("\n");
}

/** 
*@brief this function frees a particular queue from the memory
*@param queue the queue we wish to free
*/
void free_pqueue(pqueue* queue) {
    pqueue_node* cur = queue->root;
    free(queue);

    while (cur != NULL) {
        free(cur->direction_s);
        cur = cur->next;
        free(cur);
    }
}