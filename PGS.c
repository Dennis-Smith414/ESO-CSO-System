#include <bits/types/struct_timeval.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>
#define START_FAN_TEMP 45
#define STOP_FAN_TEMP 35
#define TURN_OFF_TEMP 70

int main (int argc, char *argv[]){
    if (argc != 4) {
        printf("You should not run this by itself; run main instead.\n");
        return 1;
    }

    int read_pipe = atoi(argv[1]);
    int write_pipe = atoi(argv[2]);
    int num_racks = atoi(argv[3]);

    // -1 = stop, 1 = start, 0 = no change
    int *fans = malloc(num_racks * sizeof *fans);
    int *power = malloc(num_racks * sizeof *power);

    for (int i = 0; i < num_racks; i++) {
        fans[i] = 0;
        power[i] = 1;
    }

    float *buffer = malloc(2 * num_racks * sizeof *buffer);
    int *write_buffer = malloc(2 * num_racks * sizeof *buffer);
    fd_set read_fds;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(read_pipe, &read_fds);
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        int select_return = select(read_pipe+1, &read_fds, NULL, NULL, &timeout);

        if (select_return == -1) {
            printf("error reading from pipe in PGS");
        } else if (select_return > 0) { // reading and writing
            ssize_t bytes_read = read(read_pipe, buffer, num_racks * sizeof *buffer);
            if (bytes_read > 0) {
                // like in NEM, 1 is turn on, -1 is turn off, 0 is do nothing
                for (int i = 0; i < num_racks; i++) {
                    if (buffer[i] >= TURN_OFF_TEMP) {
                        power[i] = 0;
                    } else if (buffer[i] >= START_FAN_TEMP) {
                        fans[i] = 1;
                    } else if (buffer[i] <= STOP_FAN_TEMP) {
                        fans[i] = -1;
                    }
                }

                for (int i = 0; i < num_racks; i++) {
                    write_buffer[i] = fans[i];
                    write_buffer[i+num_racks] = power[i];
                }
                usleep(20*1000); // NOTE: Added this delay due to issues with reading
                write(write_pipe, write_buffer, 2 * num_racks * sizeof *write_buffer);
            } else if (bytes_read == 0) {
                printf("PGS read pipe closed unexpectedly\n");
            } else {
                printf("Other PGS read pipe error\n");
            }
        }
    }

    return 0;
}