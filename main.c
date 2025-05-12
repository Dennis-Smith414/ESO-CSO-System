#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>

//These can be removed just for testing
// #define NUM_RACKS 10
#define MAX_RACKS 256
#define COOLING_THRESHOLD 70
#define MAX_TEMP 90
#define MIN_TEMP 20
#define COOLING_CYCLES 20
int NEM_OUTPUT[MAX_RACKS] = {0};
int SAS_OUTPUT[MAX_RACKS] = {0};
int PGS_OUTPUT[MAX_RACKS] = {0};
int PES_OUTPUT[MAX_RACKS] = {0};

// void NEM(int NUM_RACKS){
//   // Check if thread 1, then close these:
//   close(NEM_SAS_PIPE[0]); // Close read end of NEM --> SAS pipe
//   close(SAS_PGS_PIPE[0]); // Close read end of SAS --> PGS pipe
//   close(PGS_PES_PIPE[0]); // Close read end of PGS --> PES pipe
//   // Thread 1: Noisy Enterprise Model:
//   float racks[MAX_RACKS];
//   int fan_status[MAX_RACKS] = {0}; // 1 = fan is on, 0 = fan is off
//   int fan_time_on[MAX_RACKS] = {0}; // Fan can be on for COOLING_CYCLES loops
//   while (1){
//     read(PES_NEM_PIPE[0], fan_status, sizeof(fan_status));

//     for (int i = 0; i < NUM_RACKS; i++){
//       float temperature;
//       // Fan is off:
//       if (fan_status[i] == 0 || fan_time_on[i] > COOLING_CYCLES){
//         temperature = ((float)rand()/(float)RAND_MAX)*(MAX_TEMP-MIN_TEMP) + MIN_TEMP;
//         fan_time_on[i] = 0;
//       }
//       // Fan is on:
//       else if (fan_status[i] == 1 && fan_time_on[i] <= COOLING_CYCLES){
//         temperature = ((float)rand()/(float)RAND_MAX)*(COOLING_THRESHOLD-MIN_TEMP) + MIN_TEMP;
//         ++fan_time_on[i];
//       }
//       racks[i] = temperature;
//       NEM_OUTPUT[i] = racks[i];
//     }
//     write(NEM_SAS_PIPE[1], NEM_OUTPUT, sizeof(int)*MAX_RACKS);
//     sleep(1);
//   }
// }

// void SAS(int NUM_RACKS){
//   // Check if thread 2, then close these:
//   close(SAS_PGS_PIPE[0]); // Close read end of SAS --> PGS pipe
//   close(PGS_PES_PIPE[0]); // Close read end of PGS --> PES pipe
//   close(PES_NEM_PIPE[0]); // Close read end of PES --> NEM pipe
//   // Thread 2: Situation Assessment Service:
//   while(1){
//     float buff[MAX_RACKS];
//     read(NEM_SAS_PIPE[0], buff, sizeof(buff));

//     for (int i = 0; i < NUM_RACKS; i++){
//       if (buff[i] >= COOLING_THRESHOLD)
//         SAS_OUTPUT[i] = 1;
//       else
//         SAS_OUTPUT[i] = 0;
//     }
//     write(SAS_PGS_PIPE[1], SAS_OUTPUT, sizeof(int)*MAX_RACKS);
//     sleep(1);
//   }
// }

// void PGS(){
//   // Check if thread 3, then close these:
//   close(NEM_SAS_PIPE[0]); // Close read end of NEM --> SAS pipe
//   close(PGS_PES_PIPE[0]); // Close read end of PGS --> PES pipe
//   close(PES_NEM_PIPE[0]); // Close read end of PES --> NEM pipe
//   // Thread 3: Plan Generation Service
//   while(1){
//     read(SAS_PGS_PIPE[0], PGS_OUTPUT, sizeof(PGS_OUTPUT));
//     write(PGS_PES_PIPE[1], PGS_OUTPUT, sizeof(int)*MAX_RACKS);
//     sleep(1);
//   }
// }

// void PES(){
//   // Check if thread 4, then close these:
//   close(NEM_SAS_PIPE[0]); // Close read end of NEM --> SAS pipe
//   close(SAS_PGS_PIPE[0]); // Close read end of SAS --> PGS pipe
//   close(PES_NEM_PIPE[0]); // Close read end of PES --> NEM pipe
//   // Thread 4: Plan Execution Service
//   while(1){
//     read(PGS_PES_PIPE[0], PES_OUTPUT, sizeof(PES_OUTPUT));
//     write(PES_NEM_PIPE[1], PGS_OUTPUT, sizeof(int)*MAX_RACKS);
//     sleep(1);
//   }
// }

int main (int argc, char *argv[]) {
  if (argc != 2) {
    printf("Please specify number of racks\n");
    return 1;
  }

  int num_racks = atoi(argv[1]);
  if (num_racks == 0) {
    printf("Improper formatting on number of racks\n");
    return 1;
  }
  
  if (num_racks > MAX_RACKS) {
    printf("Too many racks");
    return 1;
  }

  int NEM_SAS_PIPE[2];
  int SAS_PGS_PIPE[2];
  int PGS_PES_PIPE[2];
  int PES_NEM_PIPE[2];

  if (pipe(NEM_SAS_PIPE) || pipe(SAS_PGS_PIPE) || pipe(PGS_PES_PIPE) || pipe(PES_NEM_PIPE)) {
    printf("One of the pipes failed\n");
    return 1;
  }

  // first argument to each of these execl's is read pipe, second is write pipe,
  // third is number of racks
  pid_t pid = fork();
  if (pid < 0) {
    printf("Error on first fork\n");
    return 1;
  } else if (pid == 0) {
    char read_arg[16];
    char write_arg[16];
    char num_racks_arg[16];
    snprintf(read_arg, 16, "%d", PES_NEM_PIPE[0]);
    snprintf(write_arg, 16, "%d", NEM_SAS_PIPE[1]);
    snprintf(num_racks_arg, 16, "%d",num_racks);
    execl("./NEM", "NEM", read_arg, write_arg, num_racks_arg, NULL);

    printf("First execl returned unexpectedly\n");
    return 1;
  } else {
    pid = fork();
    if (pid < 0) {
      printf("Error on second fork\n");
      return 1;
    } else if (pid == 0) {
      char read_arg[16];
      char write_arg[16];
      char num_racks_arg[16];
      snprintf(read_arg, 16, "%d", NEM_SAS_PIPE[0]);
      snprintf(write_arg, 16, "%d", SAS_PGS_PIPE[1]);
      snprintf(num_racks_arg, 16, "%d",num_racks);
      execl("./SAS", "SAS", read_arg, write_arg, num_racks_arg, NULL);

      printf("Second execl returned unexpectedly\n");
      return 1;
    } else {
      pid = fork();
      if (pid < 0) {
        printf("Error on third fork\n");
        return 1;
      } else if (pid == 0) {
        char read_arg[16];
        char write_arg[16];
        char num_racks_arg[16];
        snprintf(read_arg, 16, "%d", SAS_PGS_PIPE[0]);
        snprintf(write_arg, 16, "%d", PGS_PES_PIPE[1]);
        snprintf(num_racks_arg, 16, "%d",num_racks);
        execl("./PGS", "PGS", read_arg, write_arg, num_racks_arg, NULL);

        printf("Third execl returned unexpectedly\n");
        return 1;
      } else {
        pid = fork();
        if (pid < 0) {
          printf("Error on fouth fork\n");
          return 1;
        } else if (pid == 0) {
          char read_arg[16];
          char write_arg[16];
          char num_racks_arg[16];
          snprintf(read_arg, 16, "%d", PGS_PES_PIPE[0]);
          snprintf(write_arg, 16, "%d", PES_NEM_PIPE[1]);
          snprintf(num_racks_arg, 16, "%d",num_racks);
          execl("./PES", "PES", read_arg, write_arg, num_racks_arg, NULL);

          printf("Fourth execl returned unexpectedly\n");
          return 1;
        }
      }
    }
  }
  // int NUM_RACKS = 1;
  // if (argc > 1){
  //   NUM_RACKS = (int)atof(argv[1]);
  // }

  // if (pipe(NEM_SAS_PIPE)){
  //   printf("Failed to create NEM_SAS_PIPE\n");
  //   return 1;
  // }
  // if (pipe(SAS_PGS_PIPE)){
  //   printf("Failed to create SAS_PGS_PIPE\n");
  //   return 1;
  // }
  // if (pipe(PGS_PES_PIPE)){
  //   printf("Failed to create PGS_PES_PIPE\n");
  //   return 1;
  // }
  // if (pipe(PES_NEM_PIPE)){
  //   printf("Failed to create PES_NEM_PIPE\n");
  //   return 1;
  // }

  // /*
  // close(NEM_SAS_PIPE[0]); // Close read end of NEM --> SAS pipe
  // close(SAS_PGS_PIPE[0]); // Close read end of SAS --> PGS pipe
  // close(PGS_PES_PIPE[0]); // Close read end of PGS --> PES pipe
  // close(PES_NEM_PIPE[0]); // Close read end of PES --> NEM pipe
  // */

  // // Thread 1:
  // pid_t run_NEM;
  // run_NEM = fork();
  // if (run_NEM == 0) NEM(NUM_RACKS);
  // else if (run_NEM < 0){
  //   printf("Noisy Enterprise Model forked incorrectly.\n");
  //   return 1;
  // }
  // else {
  //   // Thread 2:
  //   pid_t run_SAS;
  //   run_SAS = fork();
  //   if (run_SAS == 0) SAS(NUM_RACKS);
  //   else if (run_SAS < 0){
  //     printf("Situation Assessment Service forked incorrectly.\n");
  //     return 1;
  //   }
  //   else {
  //     // Thread 3:
  //     pid_t run_PGS;
  //     run_PGS = fork();
  //     if (run_PGS == 0) PGS();
  //     else if (run_PGS < 0){
  //       printf("Plan Generation Service forked incorrectly.\n");
  //       return 1;
  //     }
  //     else {
  //       // Thread 4:
  //       pid_t run_PES;
  //       run_PES = fork();
  //       if (run_PES == 0) PES();
  //       else if (run_PES < 0){
  //         printf("Plan Execution Service forked incorrectly.\n");
  //         return 1;
  //       }
  //     }
  //   }
  // }

  // without this, children will run in the terminal and can't be ctrl+C-ed
  while (1) {
    sleep(1);
  }

  return 0;
}