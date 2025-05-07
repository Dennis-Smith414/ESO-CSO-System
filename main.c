#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  srand(time(NULL));

  int num_racks = atoi(argv[1]);

  if (num_racks > 1024) {
    printf("too many racks");
  }


  // can't directly put num_racks as size of arrays
  bool fans[1024] = {0};
  int temps[1024] = {0};
  int times[1024] = {0};

  while (1) {
    for (int i = 0; i < num_racks; i++) {
      if (!fans[i]) {
        temps[i] += (rand() % 5) + 1;
      }
    }

    sleep(1);
  }

  return 0;
}
