#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>

//These can be removed just for testing
// #define NUM_RACKS 10
#define MAX_RACKS 256
#define COOLING_THRESHOLD 30
#define MAX_TEMP 50 
#define MIN_TEMP 20

int NEM_SAS_PIPE[2];
int SAS_PGS_PIPE[2];
int PGS_PES_PIPE[2];
int PES_NEM_PIPE[2];
int NEM_OUTPUT[MAX_RACKS] = {0};
int SAS_OUTPUT[MAX_RACKS] = {0};
int PGS_OUTPUT[MAX_RACKS] = {0};
int PES_OUTPUT[MAX_RACKS] = {0};

void NEM(int NUM_RACKS){
  // Check if thread 1, then close these:
  close(NEM_SAS_PIPE[0]); // Close read end of NEM --> SAS pipe
  close(SAS_PGS_PIPE[0]); // Close read end of SAS --> PGS pipe
  close(PGS_PES_PIPE[0]); // Close read end of PGS --> PES pipe
  // Thread 1: Noisy Enterprise Model:
  while (1){
    printf("ur mom \n");
  }
}

void SAS(int NUM_RACKS){
  // Check if thread 2, then close these:
  close(SAS_PGS_PIPE[0]); // Close read end of SAS --> PGS pipe
  close(PGS_PES_PIPE[0]); // Close read end of PGS --> PES pipe
  close(PES_NEM_PIPE[0]); // Close read end of PES --> NEM pipe
  // Thread 2: Situation Assessment Service:
  while(1){
    float buff[MAX_RACKS];
    read(NEM_SAS_PIPE[0], buff, sizeof(buff));

    for (int i = 0; i < NUM_RACKS; i++){
      if (buff[i] >= COOLING_THRESHOLD)
        SAS_OUTPUT[i] = 1;
      else
        SAS_OUTPUT[i] = 0;
    }
    write(SAS_PGS_PIPE[1], SAS_OUTPUT, sizeof(int)*MAX_RACKS);
    sleep(1);
  }
}

void PGS(){
  // Check if thread 3, then close these:
  close(NEM_SAS_PIPE[0]); // Close read end of NEM --> SAS pipe
  close(PGS_PES_PIPE[0]); // Close read end of PGS --> PES pipe
  close(PES_NEM_PIPE[0]); // Close read end of PES --> NEM pipe
  // Thread 3: Plan Generation Service
  while(1){
    read(SAS_PGS_PIPE[0], PGS_OUTPUT, sizeof(PGS_OUTPUT));
    write(PGS_PES_PIPE[1], PGS_OUTPUT, sizeof(int)*MAX_RACKS);
    sleep(1);
  }
}

void PES(){
  // Check if thread 4, then close these:
  close(NEM_SAS_PIPE[0]); // Close read end of NEM --> SAS pipe
  close(SAS_PGS_PIPE[0]); // Close read end of SAS --> PGS pipe
  close(PES_NEM_PIPE[0]); // Close read end of PES --> NEM pipe
  // Thread 4: Plan Execution Service
  while(1){
    read(PGS_PES_PIPE[0], PES_OUTPUT, sizeof(PES_OUTPUT));
    write(PES_NEM_PIPE[1], PGS_OUTPUT, sizeof(int)*MAX_RACKS);
    sleep(1);
  }
}

int main (int argc, char *argv[]) {

  int NUM_RACKS = 1;
  if (argc > 1){
    NUM_RACKS = (int)atof(argv[1]);
  }

  if (pipe(NEM_SAS_PIPE)){
    printf("Failed to create NEM_SAS_PIPE\n");
    return 1;
  }
  if (pipe(SAS_PGS_PIPE)){
    printf("Failed to create SAS_PGS_PIPE\n");
    return 1;
  }
  if (pipe(PGS_PES_PIPE)){
    printf("Failed to create PGS_PES_PIPE\n");
    return 1;
  }
  if (pipe(PES_NEM_PIPE)){
    printf("Failed to create PES_NEM_PIPE\n");
    return 1;
  }

  /*
  close(NEM_SAS_PIPE[0]); // Close read end of NEM --> SAS pipe
  close(SAS_PGS_PIPE[0]); // Close read end of SAS --> PGS pipe
  close(PGS_PES_PIPE[0]); // Close read end of PGS --> PES pipe
  close(PES_NEM_PIPE[0]); // Close read end of PES --> NEM pipe
  */

  // Thread 1:
  pid_t run_NEM;
  run_NEM = fork();
  if (run_NEM == 0) NEM(NUM_RACKS);
  else if (run_NEM < 0){
    printf("Noisy Enterprise Model forked incorrectly.\n");
    return 1;
  }
  else {
    // Thread 2:
    pid_t run_SAS;
    run_SAS = fork();
    if (run_SAS == 0) SAS(NUM_RACKS);
    else if (run_SAS < 0){
      printf("Situation Assessment Service forked incorrectly.\n");
      return 1;
    }
    else {
      // Thread 3:
      pid_t run_PGS;
      run_PGS = fork();
      if (run_PGS == 0) PGS();
      else if (run_PGS < 0){
        printf("Plan Generation Service forked incorrectly.\n");
        return 1;
      }
      else {
        // Thread 4:
        pid_t run_PES;
        run_PES = fork();
        if (run_PES == 0) PES();
        else if (run_PES < 0){
          printf("Plan Execution Service forked incorrectly.\n");
          return 1;
        }
      }
    }
  }

  return 0;
}