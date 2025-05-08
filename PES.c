#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>

//These can be removed just for testing
#define NUM_RACKS 10
#define COOLING_THRESHOLD 30
#define MAX_TEMP 50 
#define MIN_TEMP 20 

//These can be removed in final implementation
// Shared arrays for rack status and temperatures
int rack_status[NUM_RACKS]; // 1 for hot, 0 for cool
int rack_temps[NUM_RACKS];  // Simulated temperatures for each rack
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int run = 1;

// Simulate cooling 
void simulate_cooling(int rack_id) {
    printf("Cooling rack %d (Initial Temp: %d°C)\n", rack_id, rack_temps[rack_id]);
    
    // Simulate fan speed increase (This can be changed)
    int fan_speed = (rack_temps[rack_id] - COOLING_THRESHOLD) / 2;
    if (fan_speed < 1) fan_speed = 1;
    if (fan_speed > 10) fan_speed = 10;
    printf("Adjusting fan speed for rack %d to level %d\n", rack_id, fan_speed);
    
    // Simulate temperature drop over time
    // Three steps of temp decrease 5 degress each time
    for (int i = 0; i < 3; i++) { 

        rack_temps[rack_id] -= 5; 
        if (rack_temps[rack_id] < MIN_TEMP) rack_temps[rack_id] = MIN_TEMP;
        printf("Rack %d Temp after cooling step %d: %d°C\n", rack_id, i + 1, rack_temps[rack_id]);
        sleep(1); //Time for the cooling
    }
    
    // Mark as cooled
    rack_status[rack_id] = 0;
    printf("Rack %d cooled successfully. Final Temp: %d°C\n", rack_id, rack_temps[rack_id]);
}

void* plan_execution_service(void* arg) {
  int fd;
  int rack_status[NUM_RACKS];
  
  if ((fd = open(PIPE, O_RDONLY)) < 0) {
    perror("Failed to open the read pipe");
    exit(1);
  }

    while (run) {
        if (read(fd, rack_status, NUM_RACKS * sizeof(int)) < 0) {
          perror("Failed to read from pipe");
          break;
        }
        
        for (int i = 0; i < NUM_RACKS; i++) {
            pthread_mutex_lock(&mutex);
            if (rack_status[i] == 1) { 
                simulate_cooling(i);
            }
            pthread_mutex_unlock(&mutex);
        }
        sleep(1); // Check every second
    }
    return NULL;
}

// Test case 1: Single hot rack
void test_case_1() {
    printf("\n=== Test Case 1: Single Hot Rack ===\n");
    for (int i = 0; i < NUM_RACKS; i++) {
        rack_status[i] = 0;
        rack_temps[i] = MIN_TEMP;
    }
    rack_status[2] = 1; 
    rack_temps[2] = 45;
    sleep(5); 
}

// Test case 2: Multiple hot racks
void test_case_2() {
    printf("\n=== Test Case 2: Multiple Hot Racks ===\n");
    for (int i = 0; i < NUM_RACKS; i++) {
        rack_status[i] = 0;
        rack_temps[i] = MIN_TEMP;
    }
    rack_status[1] = 1; rack_temps[1] = 40; 
    rack_status[4] = 1; rack_temps[4] = 50;
    rack_status[7] = 1; rack_temps[7] = 35;
    sleep(10); 
}

// Test case 3: No hot racks
void test_case_3() {
    printf("\n=== Test Case 3: No Hot Racks ===\n");
    for (int i = 0; i < NUM_RACKS; i++) {
        rack_status[i] = 0;
        rack_temps[i] = MIN_TEMP;
    }
    sleep(3);
}

int main() {
    pthread_t pes_thread;
    
    // Initialize random seed for temperature simulation
    srand(time(NULL));
    
    // Init arrays
    for (int i = 0; i < NUM_RACKS; i++) {
        rack_status[i] = 0;
        rack_temps[i] = MIN_TEMP;
    }
    
    // call thread 4
    if (pthread_create(&pes_thread, NULL, plan_execution_service, NULL) != 0) {
        printf("Failed to create Thread 4\n");
        return 1;
    }
    
    //test cases
    test_case_1();
    test_case_2();
    test_case_3();
    
    run = 0;
    
    // Wait for Thread 4 to finish
    pthread_join(pes_thread, NULL);
    
    // Cleanup
    pthread_mutex_destroy(&mutex);
    return 0;
}
