#include <bits/types/struct_timeval.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>
#define MAX_WAIT 3
#define ROOM_TEMP 20

int main (int argc, char *argv[]) {
    if (argc != 4) {
        printf("You should not run this by itself; run main instead.\n");
        return 1;
    }

    int read_pipe = atoi(argv[1]);
    int write_pipe = atoi(argv[2]);
    int num_racks = atoi(argv[3]);

    // fans and power are booleans
    int *rack_temps = malloc(num_racks * sizeof *rack_temps);
    int *fans = malloc(num_racks * sizeof *fans);
    int *power = malloc(num_racks * sizeof *power);
    for (int i = 0; i < num_racks; i++) {
        rack_temps[i] = ROOM_TEMP;
        fans[i] = 0;
        power[i] = 1;
    }

    srand(time(NULL));
    fd_set read_fds;
    time_t start_time = time(NULL);
    int *buffer = malloc(2 * num_racks * sizeof *buffer);

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(read_pipe, &read_fds);
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        int select_return = select(read_pipe+1, &read_fds, NULL, NULL, &timeout); // +1 is weird requirement of select

        if (select_return == -1) {
            printf("error reading from pipe in NEM");
        } else if (select_return == 0) { // timeout
            time_t cur_time = time(NULL);
            if (cur_time - start_time >= MAX_WAIT) {
                start_time = cur_time;

                // cooling/heating simulation
                for (int i = 0; i < num_racks; i++) {
                    if (!power[i]) {
                        int temp = (rand() % 3) + 1;
                        rack_temps[i] -= temp;
                        if (rack_temps[i] < ROOM_TEMP) rack_temps[i] = ROOM_TEMP;
                    }

                    else if (power[i] && fans[i]) {
                        int temp = rand() % 3;
                        if (rand() % 10 == 0) temp = -1; //small chance to increase temp even if fan is on
                        rack_temps[i] -= temp;
                        if (rack_temps[i] < ROOM_TEMP) rack_temps[i] = ROOM_TEMP;
                    }

                    else if (power[i] && !fans[i]) {
                        int temp = rand() % 3;
                        rack_temps[i] += temp;
                    }
                }
            }

            write(write_pipe, rack_temps, num_racks * sizeof *rack_temps);
        } else { // In this case there is something to read. Data format is array twice as long as num_racks,
                 // first section is fans and second is power
                 // fans can be 1, -1 or 0 (turn on, turn off, no change)
            
            if (FD_ISSET(read_pipe, &read_fds)) {
                ssize_t bytes_read = read(read_pipe, buffer, num_racks * 2 * sizeof *buffer);
                if (bytes_read > 0) {
                    for (int i = 0; i < num_racks; i++) {
                        int started_off = !power[i];

                        if (buffer[i] == 1) {
                            fans[i] = 1;
                        } else if (buffer[i] == -1) {
                            fans[i] = 0;
                        }
                        power[i] = buffer[i+num_racks];
                        if (started_off && power[i]) fans[i] = 0; // when powered back on, fan starts off
                    }
                } else if (bytes_read == 0) {
                    printf("NEM read pipe closed unexpectedly\n");
                } else {
                    printf("Other NEM read pipe error\n");
                }
            }
        }
    }

    return 0;
}
