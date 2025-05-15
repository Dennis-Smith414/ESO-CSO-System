#include <bits/types/struct_timeval.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>
#define MAX_TIME_OFF 35

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("You should not run this by itself; run main instead.\n");
        return 1;
    }

    int read_pipe = atoi(argv[1]);
    int write_pipe = atoi(argv[2]);
    int num_racks = atoi(argv[3]);

    int *time_off = malloc(num_racks * sizeof *time_off);
    int *inc_timer = malloc(num_racks * sizeof *inc_timer);

    for (int i = 0; i < num_racks; i++) {
        time_off[i] = 0;
        inc_timer = 0;
    }

    int *buffer = malloc(num_racks * sizeof *buffer);
    int *write_buffer = malloc(num_racks * sizeof *buffer);
    fd_set read_fds;
    time_t prev_time = time(NULL);

    while (1) {
        time_t cur_time = time(NULL);

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
                    write_buffer[i] = 0;
                    if (buffer[i] != 0) {
                        need_to_write = 1;
                        write_buffer[i] = buffer[i];
                        if (buffer[i] == 2)
                            inc_timer[i] = 1;
                    }
                }

                if (need_to_write) {
                    write(write_pipe, buffer, num_racks * sizeof *buffer);
                }
            } else if (bytes_read == 0) {
                printf("PES read pipe closed unexpectedly\n");
            } else {
                printf("Other PES read pipe error\n");
            }

        } else { //timed out: keep track of powered off racks
            int need_to_write = 0;

            for (int i = 0; i < num_racks; i++) {
                write_buffer[i] = 0;

                if (inc_timer[i]) {
                    time_off[i] += cur_time - prev_time;
                    if (time_off[i] >= MAX_TIME_OFF) {
                        need_to_write = 1;
                        write_buffer[i] = 1;
                        time_off[i] = 0;
                        inc_timer[i] = 0;
                        printf("Turning on rack no. %d\n", i+1);
                    }
                }
            }

            if (need_to_write) {
                write(write_pipe, write_buffer, num_racks * sizeof *write_buffer);
            }
        }
        
        prev_time = cur_time;
    }

    return 0;
}
