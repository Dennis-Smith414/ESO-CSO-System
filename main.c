#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>
//These can be removed just for testing
// #define NUM_RACKS 10
#define MAX_RACKS 256
#define COOLING_THRESHOLD 70
#define MAX_TEMP 90
#define MIN_TEMP 20
#define COOLING_CYCLES 20
int NEM_SAS_PIPE[2];
int SAS_PGS_PIPE[2];
int PGS_PES_PIPE[2];
int PES_NEM_PIPE[2];
int NEM_OUTPUT[MAX_RACKS] = {0};
int SAS_OUTPUT[MAX_RACKS] = {0};
int PGS_OUTPUT[MAX_RACKS] = {0};
int PES_OUTPUT[MAX_RACKS] = {0};
void NEM(int NUM_RACKS){
  printf("[NEM] Starting with %d racks\n", NUM_RACKS);
  // Check if thread 1, then close these:
  close(NEM_SAS_PIPE[0]); // Close read end of NEM --> SAS pipe
  close(SAS_PGS_PIPE[0]); // Close read end of SAS --> PGS pipe
  close(PGS_PES_PIPE[0]); // Close read end of PGS --> PES pipe
  close(PES_NEM_PIPE[1]); // Close write end of PES --> NEM pipe
  // Thread 1: Noisy Enterprise Model:
  float racks[MAX_RACKS];
  int fan_status[MAX_RACKS] = {0}; // 1 = fan is on, 0 = fan is off
  int fan_time_on[MAX_RACKS] = {0}; // Fan can be on for COOLING_CYCLES loops
  while (1){
    printf("[NEM] Waiting for fan status from PES\n");
    read(PES_NEM_PIPE[0], fan_status, sizeof(fan_status));
    printf("[NEM] Received fan status from PES\n");
    
    for (int i = 0; i < NUM_RACKS; i++){
      float temperature;
      // Fan is off:
      if (fan_status[i] == 0 || fan_time_on[i] > COOLING_CYCLES){
        temperature = ((float)rand()/(float)RAND_MAX)*(MAX_TEMP-MIN_TEMP) + MIN_TEMP;
        fan_time_on[i] = 0;
      }
      // Fan is on:
      else if (fan_status[i] == 1 && fan_time_on[i] <= COOLING_CYCLES){
        temperature = ((float)rand()/(float)RAND_MAX)*(COOLING_THRESHOLD-MIN_TEMP) + MIN_TEMP;
        ++fan_time_on[i];
      }
      racks[i] = temperature;
      NEM_OUTPUT[i] = racks[i];
      printf("[NEM] Rack %d: Temp=%.2f, Fan=%s, Cycles=%d\n", 
             i, temperature, fan_status[i] ? "ON" : "OFF", fan_time_on[i]);
    }
    printf("[NEM] Sending temperatures to SAS\n");
    write(NEM_SAS_PIPE[1], NEM_OUTPUT, sizeof(int)*MAX_RACKS);
    sleep(1);
  }
}
void SAS(int NUM_RACKS){
  printf("[SAS] Starting with %d racks\n", NUM_RACKS);
  // Check if thread 2, then close these:
  close(NEM_SAS_PIPE[1]); // Close write end of NEM --> SAS pipe
  close(SAS_PGS_PIPE[0]); // Close read end of SAS --> PGS pipe
  close(PGS_PES_PIPE[0]); // Close read end of PGS --> PES pipe
  close(PES_NEM_PIPE[0]); // Close read end of PES --> NEM pipe
  // Thread 2: Situation Assessment Service:
  while(1){
    float buff[MAX_RACKS];
    printf("[SAS] Waiting for temperatures from NEM\n");
    read(NEM_SAS_PIPE[0], buff, sizeof(buff));
    printf("[SAS] Received temperatures from NEM\n");
    
    for (int i = 0; i < NUM_RACKS; i++){
      if (buff[i] >= COOLING_THRESHOLD) {
        SAS_OUTPUT[i] = 1;
        printf("[SAS] Rack %d: Temp=%.2f - NEEDS COOLING\n", i, buff[i]);
      }
      else {
        SAS_OUTPUT[i] = 0;
        printf("[SAS] Rack %d: Temp=%.2f - OK\n", i, buff[i]);
      }
    }
    printf("[SAS] Sending cooling needs to PGS\n");
    write(SAS_PGS_PIPE[1], SAS_OUTPUT, sizeof(int)*MAX_RACKS);
    sleep(1);
  }
}
void PGS(int NUM_RACKS){
  printf("[PGS] Starting with %d racks\n", NUM_RACKS);
  // Check if thread 3, then close these:
  close(NEM_SAS_PIPE[0]); // Close read end of NEM --> SAS pipe
  close(NEM_SAS_PIPE[1]); // Close write end of NEM --> SAS pipe
  close(SAS_PGS_PIPE[1]); // Close write end of SAS --> PGS pipe
  close(PGS_PES_PIPE[0]); // Close read end of PGS --> PES pipe
  close(PES_NEM_PIPE[0]); // Close read end of PES --> NEM pipe
  // Thread 3: Plan Generation Service
  while(1){
    printf("[PGS] Waiting for cooling needs from SAS\n");
    read(SAS_PGS_PIPE[0], PGS_OUTPUT, sizeof(PGS_OUTPUT));
    printf("[PGS] Received cooling needs from SAS\n");
    
    for (int i = 0; i < NUM_RACKS; i++){
      printf("[PGS] Rack %d: Cooling plan = %s\n", i, PGS_OUTPUT[i] ? "ACTIVATE FAN" : "NO ACTION");
    }
    
    printf("[PGS] Sending cooling plan to PES\n");
    write(PGS_PES_PIPE[1], PGS_OUTPUT, sizeof(int)*MAX_RACKS);
    sleep(1);
  }
}
void PES(int NUM_RACKS){
  printf("[PES] Starting with %d racks\n", NUM_RACKS);
  // Check if thread 4, then close these:
  close(NEM_SAS_PIPE[0]); // Close read end of NEM --> SAS pipe
  close(NEM_SAS_PIPE[1]); // Close write end of NEM --> SAS pipe
  close(SAS_PGS_PIPE[0]); // Close read end of SAS --> PGS pipe
  close(SAS_PGS_PIPE[1]); // Close write end of SAS --> PGS pipe
  close(PGS_PES_PIPE[1]); // Close write end of PGS --> PES pipe
  // Thread 4: Plan Execution Service
  while(1){
    printf("[PES] Waiting for cooling plan from PGS\n");
    read(PGS_PES_PIPE[0], PES_OUTPUT, sizeof(PES_OUTPUT));
    printf("[PES] Received cooling plan from PGS\n");
    
    for (int i = 0; i < NUM_RACKS; i++){
      printf("[PES] Rack %d: Executing plan = %s\n", i, PES_OUTPUT[i] ? "TURN ON FAN" : "KEEP FAN OFF");
    }
    
    printf("[PES] Sending fan status to NEM\n");
    write(PES_NEM_PIPE[1], PES_OUTPUT, sizeof(int)*MAX_RACKS);
    sleep(1);
  }
}
int main (int argc, char *argv[]) {
  int NUM_RACKS = 1;
  if (argc > 1){
    NUM_RACKS = (int)atof(argv[1]);
  }
  printf("[MAIN] Starting with %d racks\n", NUM_RACKS);
  
  // Seed the random number generator
  srand(time(NULL));
  
  if (pipe(NEM_SAS_PIPE)){
    printf("[MAIN] Failed to create NEM_SAS_PIPE\n");
    return 1;
  }
  printf("[MAIN] Created NEM_SAS_PIPE\n");
  
  if (pipe(SAS_PGS_PIPE)){
    printf("[MAIN] Failed to create SAS_PGS_PIPE\n");
    return 1;
  }
  printf("[MAIN] Created SAS_PGS_PIPE\n");
  
  if (pipe(PGS_PES_PIPE)){
    printf("[MAIN] Failed to create PGS_PES_PIPE\n");
    return 1;
  }
  printf("[MAIN] Created PGS_PES_PIPE\n");
  
  if (pipe(PES_NEM_PIPE)){
    printf("[MAIN] Failed to create PES_NEM_PIPE\n");
    return 1;
  }
  printf("[MAIN] Created PES_NEM_PIPE\n");
  
  // Thread 1:
  pid_t run_NEM;
  printf("[MAIN] Forking NEM process\n");
  run_NEM = fork();
  if (run_NEM == 0) {
    NEM(NUM_RACKS);
    exit(0);
  }
  else if (run_NEM < 0){
    printf("[MAIN] Noisy Enterprise Model forked incorrectly.\n");
    return 1;
  }
  else {
    printf("[MAIN] NEM process started with PID %d\n", run_NEM);
    
    // Thread 2:
    pid_t run_SAS;
    printf("[MAIN] Forking SAS process\n");
    run_SAS = fork();
    if (run_SAS == 0) {
      SAS(NUM_RACKS);
      exit(0);
    }
    else if (run_SAS < 0){
      printf("[MAIN] Situation Assessment Service forked incorrectly.\n");
      return 1;
    }
    else {
      printf("[MAIN] SAS process started with PID %d\n", run_SAS);
      
      // Thread 3:
      pid_t run_PGS;
      printf("[MAIN] Forking PGS process\n");
      run_PGS = fork();
      if (run_PGS == 0) {
        PGS(NUM_RACKS);
        exit(0);
      }
      else if (run_PGS < 0){
        printf("[MAIN] Plan Generation Service forked incorrectly.\n");
        return 1;
      }
      else {
        printf("[MAIN] PGS process started with PID %d\n", run_PGS);
        
        // Thread 4:
        pid_t run_PES;
        printf("[MAIN] Forking PES process\n");
        run_PES = fork();
        if (run_PES == 0) {
          PES(NUM_RACKS);
          exit(0);
        }
        else if (run_PES < 0){
          printf("[MAIN] Plan Execution Service forked incorrectly.\n");
          return 1;
        }
        else {
          printf("[MAIN] PES process started with PID %d\n", run_PES);
          printf("[MAIN] All processes started. Parent process will wait.\n");
          
          // Close all pipe ends in parent process since we don't use them
          close(NEM_SAS_PIPE[0]);
          close(NEM_SAS_PIPE[1]);
          close(SAS_PGS_PIPE[0]);
          close(SAS_PGS_PIPE[1]);
          close(PGS_PES_PIPE[0]);
          close(PGS_PES_PIPE[1]);
          close(PES_NEM_PIPE[0]);
          close(PES_NEM_PIPE[1]);
          
          // Wait for child processes
          waitpid(run_NEM, NULL, 0);
          waitpid(run_SAS, NULL, 0);
          waitpid(run_PGS, NULL, 0);
          waitpid(run_PES, NULL, 0);
        }
      }
    }
  }
  return 0;
}
