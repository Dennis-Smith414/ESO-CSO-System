#include <stdio.h>
#include <stdlib.h>
#define NUM_RACKS 5 // Number of server racks, this will be an argument provided to main
#define MAX_TEMP 70.0 // The maximum tolerable temperature before our system deems a server rack is too hot

int main (int argc, char *argv[]){
    // For checking argv:
    // printf("argc: %d \n", argc);

    // for (int i = 0; i < argc; i++){
    //     printf("argument #%d: %d \n", i, (int)atof(argv[i]));
    // }
    // printf("\n");

    // ignore first argument, put all others into an array
    // Initialize the SAS_output, all values are 0 at first
    int PGS_output[NUM_RACKS] = {0};

    for (int i = 1; i < argc; i++){
        PGS_output[i-1] = (int)atof(argv[i]);
    }

    // Loop through PGS_output to check the validity:
    // (this will be replaced with a loop that loads PGS_output 
    // onto a pipe for the next thread to pick up)
    for (int i = 0; i < argc-1; i++){
        printf("output #%d: %d \n", i, PGS_output[i]);
    }

    return 0;
}