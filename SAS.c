#include <bits/types/struct_timeval.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>
#define AVG_WINDOW 4

int main (int argc, char *argv[]){
    if (argc != 4) {
        printf("You should not run this by itself; run main instead.\n");
        return 1;
    }

    int read_pipe = atoi(argv[1]);
    int write_pipe = atoi(argv[2]);
    int num_racks = atoi(argv[3]);

    int current_iter = 0;
    int *all_temps = malloc(num_racks * AVG_WINDOW * sizeof *all_temps); //flattened 2d array
    float *avg_temps = malloc(num_racks * sizeof *avg_temps);

    int *buffer = malloc(num_racks * sizeof *buffer);
    fd_set read_fds;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(read_pipe, &read_fds);
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        int select_return = select(read_pipe+1, &read_fds, NULL, NULL, &timeout);

        if (select_return == -1) {
            printf("error reading from pipe in SAS");
        } else if (select_return > 0) { // reading and writing
            ssize_t bytes_read = read(read_pipe, buffer, num_racks * sizeof *buffer);
            if (bytes_read > 0) {
                if (current_iter != AVG_WINDOW) { // not yet read the full window
                    for (int i = 0; i < num_racks; i++) {
                        all_temps[i*AVG_WINDOW + current_iter] = buffer[i]; 
                    }
                    current_iter++;
                } else { // already read the full window, so must replace oldest value
                    for (int i = 0; i < num_racks; i++) {
                        for (int j = AVG_WINDOW-1; j >= 0; j--) {
                            if (j != 0) all_temps[i*AVG_WINDOW + j] = all_temps[i*AVG_WINDOW + j - 1];
                            else all_temps[i*AVG_WINDOW] = buffer[i];
                        }
                    }
                }
                
                // calculate average
                for (int i = 0; i < num_racks; i++) {
                    int avg = all_temps[i*AVG_WINDOW];
                    for (int j = 1; j < current_iter; j++) {
                        avg += all_temps[i*AVG_WINDOW + j];
                    }

                    double real_avg = (double)(avg) / (double)(current_iter);
                    avg_temps[i] = real_avg;
                    printf("Sent: %f\n", real_avg);
                }
                //usleep(20*1000); // NOTE: Added this delay due to issues with reading
                write(write_pipe, avg_temps, num_racks * sizeof *avg_temps);
            } else if (bytes_read == 0) {
                printf("SAS read pipe closed unexpectedly\n");
            } else {
                printf("Other SAS read pipe error\n");
            }
        } // do nothing on timeout, just wait again
    }

    return 0;
}