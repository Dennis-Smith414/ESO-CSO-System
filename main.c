#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>

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

  // without this, children will run in the terminal and can't be ctrl+C-ed
  while (1) {
    sleep(1);
  }

  return 0;
}