#include <stdio.h>
#include <stdlib.h>

#include "mdata.h"
#include "train.h"

#define LINE_BUFFER 100

extern int cur_size;

/** 
*@brief this function parses an .txt file and creates a train for each line read in the proper format
*/
void parse_input_to_trains(char* filepath, int size, pthread_t tid[], FILE* outptr) {
    FILE* fptr = fopen(filepath, "r");

    if (fptr == NULL) {
        printf("ERROR: File pointer is null\n");
        exit(1);
    }

    char buf[LINE_BUFFER];
    size_t i = 0;

    while (fgets(buf, LINE_BUFFER, fptr)) {
        pqueue_node* new_node = parse_line_to_node(buf);
        data_package* data = (data_package*)malloc(sizeof(data_package));
        data->node = new_node;
        data->node->tid = i;
        data->westbound = east_to_west;
        data->eastbound = west_to_east;
        data->outptr = outptr;
        
        pthread_create(&tid[i], NULL, train, (void *) data);
        ++i;
        cur_size++;
    }   

    fclose(fptr);
}

/** 
*@brief this function determines the size of a file
*/
int determine_size(char* filepath) {
    FILE* fptr = fopen(filepath, "r");
    int c = 0;

    int lines = 0;

    if (fptr == NULL) return 0;
    lines++;

    /*
    Another way to fix the issue, yet it is quite different than my original attempt
    char tempDir;
    int tempLoad;
    int tempCross;
    while (fscanf(fptr, "%c %d %d\n", &tempDir, &tempLoad, &tempCross) == 3) {
        lines++;
    }
    */
    
    //NEW
    int prevc = '\n';
    for (c = getc(fptr); c != EOF; c = getc(fptr)) {
        if (c == '\n') lines ++;

        //NEW
        prevc = c;
    }

    //NEW
    if (prevc == '\n') lines --;

    fclose(fptr);
    return lines;

}