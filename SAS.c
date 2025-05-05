#include <stdio.h>
#include <stdlib.h>
#define NUM_RACKS 5
#define MAX_TEMP 70.0


int main (int argc, char *argv[]){
    // printf("argc: %d \n", argc);

    // for (int i = 0; i < argc; i++){
    //     printf("argument #%d: %lf \n", i, (float)atof(argv[i]));
    // }


    // printf("\n");

    // ignore first argument, put all others into an array
    int output[NUM_RACKS] = {0};

    for (int i = 0; i < NUM_RACKS; i++){
        if ((float)atof(argv[i+1]) >= MAX_TEMP)
            output[i] = 1;
        else
            output[i] = 0;
    }

    for (int i = 0; i < argc-1; i++){
        printf("output #%d: %d \n", i, output[i]);
    }

    return 0;
}