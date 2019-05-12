#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "thesquid.h"

int main(int argc, char** argv) {
  // Process arguments
  int port = -1;
  for (int iArg = 0; iArg < argc; ++iArg) {
    if (strcmp(argv[iArg], "-port") == 0) {
      ++iArg;
      port = atoi(argv[iArg]);
    }
  }
  
  // Create the squidlet
  Squidlet* squidlet = SquidletCreateOnPort(port);
  if (squidlet == NULL) {
    printf("Failed to create the squidlet\n");
    printf("errno: %s\n", strerror(errno));
  }
  printf("Squidlet : ");
  SquidletPrint(squidlet, stdout);
  printf("\n");

  // Loop until it's killed
  do {
    SquidletTaskRequest request = SquidletWaitRequest(squidlet);
    SquidletProcessRequest(squidlet, &request);
  } while (!Squidlet_CtrlC);
  SquidletFree(&squidlet);
  printf("Squidlet : ended\n");
  fflush(stdout);

  // Return success code
  return 0;
}

