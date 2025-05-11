#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main (int argc, char *argv[]) {
    if (argc != 4) {
        printf("You should not run this by itself; run main instead.\n");
        return 1;
    }

    int read_pipe = atoi(argv[1]);
    int write_pipe = atoi(argv[2]);
    int num_racks = atoi(argv[3]);

    int *racks = malloc(num_racks * sizeof(racks));

    srand(time(NULL));

    while (1) {
        int rng = rand() % 15;
        printf("random = %d\n", rng);
        sleep(3);
    }
    // printf("argc: %d \n", argc);

    // for (int i = 0; i < argc; i++){
    //     printf("argument #%d: %lf \n", i, (float)atof(argv[i]));
    // }


    // printf("\n");

    // // ignore first argument, put all others into an array
    // int output[length] = {0};

    // for (int i = 0; i < length; i++){
    //     if ((float)atof(argv[i+1]) >= 70.0)
    //         output[i] = 1;
    //     else
    //         output[i] = 0;
    // }

    // for (int i = 0; i < argc-1; i++){
    //     printf("output #%d: %d \n", i, output[i]);
    // }

    return 0;
}
