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

    int *rack_temps = malloc(num_racks * sizeof *rack_temps);
    int *rack_states = malloc(num_racks * sizeof *rack_states); //0 = power off; 1 = power on, fan off; 2 = power on, fan on
    int *broken_fans = malloc(num_racks * sizeof *broken_fans);
    
    srand(time(NULL));
    for (int i = 0; i < num_racks; i++) {
        rack_temps[i] = ROOM_TEMP;
        rack_states[i] = 1;
        broken_fans[i] = rand() % 20 == 0;
        // unrealistically high break rate, but it demonstrates the turn off logic
    }

    fd_set read_fds;
    time_t start_time = time(NULL);
    int *buffer = malloc(num_racks * sizeof *buffer);

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
                    if (rack_states[i] == 0) {
                        int temp = (rand() % 3) + 1;
                        rack_temps[i] -= temp;
                        if (rack_temps[i] < ROOM_TEMP)
                            rack_temps[i] = ROOM_TEMP;
                    } else if (rack_states[i] == 1) {
                        int temp = rand() % 3;
                        rack_temps[i] += temp;
                    }else if (rack_states[i] == 2 && !broken_fans[i]) {
                        int temp = rand() % 3;
                        if (rand() % 10 == 0) //small chance to increase temp even if fan is on
                            temp = -1;
                        rack_temps[i] -= temp;
                        if (rack_temps[i] < ROOM_TEMP)
                            rack_temps[i] = ROOM_TEMP;
                    }
                }
                
                printf("\nTEMPS\n");
                for (int i = 0; i < num_racks; i++) {
                    printf("Rack %d: %d C\n", i+1, rack_temps[i]);
                }
                printf("\nFANS\n");
                for (int i = 0; i < num_racks; i++) {
                    if (broken_fans[i])
                        printf("Rack %d's fan is broken\n", i+1);
                    else if (rack_states[i] == 2)
                        printf("Rack %d's fan is on\n", i+1);
                    else
                        printf("Rack %d's fan is off\n", i+1);
                }
                printf("\nPOWER\n");
                for (int i = 0; i < num_racks; i++) {
                    if (rack_states[i] == 0)
                        printf("Rack %d is powered off\n", i+1);
                    else
                        printf("Rack %d is powered on\n", i+1);
                }
                printf("\n");
                write(write_pipe, rack_temps, num_racks * sizeof *rack_temps);
            }
        } else { // In this case there is something to read. Read data is interpreted as follows:
                 // 0 = no change; 1 = turn on power; 2 = turn off power; 3 = turn on fan; 4 = turn off fan
            
            if (FD_ISSET(read_pipe, &read_fds)) {
                ssize_t bytes_read = read(read_pipe, buffer, num_racks * sizeof *buffer);
                if (bytes_read > 0) {
                    for (int i = 0; i < num_racks; i++) {
                        if (buffer[i] == 1 && rack_states[i] == 0)
                            rack_states[i] = 1;
                        else if (buffer[i] == 2 && rack_states[i] != 0)
                            rack_states[i] = 0;
                        else if (buffer[i] == 3 && rack_states[i] == 1 && !broken_fans[i])
                            rack_states[i] = 2;
                        else if (buffer[i] == 4 && rack_states[i] == 2 && !broken_fans[i])
                            rack_states[i] = 1;
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
