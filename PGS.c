#include <stdio.h>
#include <stdlib.h>
#define NUM_RACKS 5 // Number of server racks, this will be an argument provided to main
#define MAX_TEMP 70.0 // The maximum tolerable temperature before our system deems a server rack is too hot

int main (int argc, char *argv[]){
    // For checking argv:
    printf("argc: %d \n", argc);

    for (int i = 0; i < argc; i++){
        printf("argument #%d: %lf \n", i, (float)atof(argv[i]));
    }
    printf("\n");

    // ignore first argument, put all others into an array
    // Initialize the SAS_output, all values are 0 at first
    int SAS_output[NUM_RACKS] = {0};

    // Iterate through argv (the input to SAS)
    // check if it's above or equal to max temp: it's too hot SAS_output[i] = 1, if it's fine SAS_output[i] = 0 
    for (int i = 0; i < NUM_RACKS; i++){
        if ((float)atof(argv[i+1]) >= MAX_TEMP)
            SAS_output[i] = 1;
        else
            SAS_output[i] = 0;
    }

    // Loop through SAS_output to check the validity:
    // (this will be replaced with a loop that loads SAS_output 
    // onto a pipe for the next thread to pick up)
    for (int i = 0; i < argc-1; i++){
        printf("output #%d: %d \n", i, SAS_output[i]);
    }

    return 0;
}