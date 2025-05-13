#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#define MAX_TIME_OFF 35

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("You should not run this by itself; run main instead.\n");
        return 1;
    }

    int read_pipe = atoi(argv[1]);
    int write_pipe = atoi(argv[2]);
    int num_racks = atoi(argv[3]);

    int *fans = malloc(num_racks * sizeof *fans);
    int *power = malloc(num_racks * sizeof *power);
    int *time_off = malloc(num_racks * sizeof *time_off);

    for (int i = 0; i < num_racks; i++) {
        fans[i] = 0;
        power[i] = 1;
        time_off[i] = 0;
    }

    int *buffer = malloc(2 * num_racks * sizeof *buffer);
    int *write_buffer = malloc(2 * num_racks * sizeof *buffer);
    fd_set read_fds;
    time_t prev_time = time(NULL);

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(read_pipe, &read_fds);
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        int select_return = select(read_pipe+1, &read_fds, NULL, NULL, &timeout);

        if (select_return == -1) {
            printf("error reading from pipe in PES");
        } else if (select_return > 0) { // reading
            ssize_t bytes_read = read(read_pipe, buffer, num_racks * sizeof *buffer);
            if (bytes_read > 0) {
                int need_to_write = 0;

                for (int i = 0; i < num_racks; i++) {
                    fans[i] = buffer[i];
                    if (fans[i] != 0) {
                        need_to_write = 1;
                        if (fans[i] == -1) printf("Turning off fan no. %d\n", i+1);
                        else if (fans[i] == 1) printf("Turning on fan no. %d\n", i+1);
                    }

                    int oldPower = power[i];
                    power[i] = buffer[i+num_racks];
                    if (oldPower != power[i]) {
                        need_to_write = 1;
                        if (power[i]) {
                            printf("Turning on rack no. %d\n", i+1);
                            time_off[i] = 0;
                        }
                        else printf("Turning off rack no. %d\n", i+1);
                    }
                }

                if (need_to_write) {
                    usleep(20*1000); // NOTE: Added this delay due to issues with reading
                    write(write_pipe, buffer, 2 * num_racks * sizeof *buffer);
                }
            } else if (bytes_read == 0) {
                printf("PES read pipe closed unexpectedly\n");
            } else {
                printf("Other PES read pipe error\n");
            }

        } else { //timed out: keep track of powered off racks
            time_t cur_time = time(NULL);
            int need_to_write = 0;

            for (int i = 0; i < num_racks; i++) {
                if (!power[i]) {
                    time_off[i] += cur_time - prev_time;
                    if (time_off[i] >= MAX_TIME_OFF) {
                        need_to_write = 1;
                        power[i] = 1;
                        time_off[i] = 0;
                        printf("Turning on rack no. %d\n", i);
                    }
                }
            }
            prev_time = cur_time;

            if (need_to_write) {
                for (int i = 0; i < num_racks; i++) {
                    write_buffer[i] = 0; //don't change fans, only turn on
                    write_buffer[i+num_racks] = power[i];
                }
                usleep(20*1000); // NOTE: Added this delay due to issues with reading
                write(write_pipe, write_buffer, 2 * num_racks * sizeof *write_buffer);
            }
        }
    }

    return 0;
}
