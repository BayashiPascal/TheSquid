#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "thesquid.h"

int main(int argc, char** argv) {
  // Process arguments
  int port = -1;
  char* outputFilePath = NULL;
  for (int iArg = 0; iArg < argc; ++iArg) {
    if (strcmp(argv[iArg], "-port") == 0) {
      ++iArg;
      port = atoi(argv[iArg]);
    }
    if (strcmp(argv[iArg], "-verbose") == 0) {
      ++iArg;
      outputFilePath = argv[iArg];
    }
    if (strcmp(argv[iArg], "-help") == 0) {
      printf("squidlet [-verbose <stdout | file path>] ");
      printf("[-port <port>] [-help]\n");
      exit(0);
    }
  }
  
  // Create the squidlet
  Squidlet* squidlet = SquidletCreateOnPort(port);
  if (squidlet == NULL) {
    printf("Failed to create the squidlet\n");
    printf("TheSquidErr: %s\n", TheSquidErr->_msg);
    printf("errno: %s\n", strerror(errno));
    exit(1);
  }
  printf("Squidlet : ");
  SquidletPrint(squidlet, stdout);
  printf("\n");

  // Set the output stream
  FILE* stream = NULL;
  if (outputFilePath != NULL) {
    if (strcmp(outputFilePath, "stdout") == 0) {
      SquidletSetStreamInfo(squidlet, stdout);
    } else if (outputFilePath != NULL) {
      stream = fopen(outputFilePath, "w");
      if (stream == NULL) {
        printf("Failed to open the file %s\n", outputFilePath);
        exit(1);
      }
      SquidletSetStreamInfo(squidlet, stream);
    } else {
      SquidletSetStreamInfo(squidlet, NULL);
    }
  } else {
    SquidletSetStreamInfo(squidlet, NULL);
  }
  
  // Loop until it receives Ctrl-C
  do {
    SquidletTaskRequest request = SquidletWaitRequest(squidlet);
    SquidletProcessRequest(squidlet, &request);
  } while (!Squidlet_CtrlC);

  // Free memory
  SquidletFree(&squidlet);
  printf("Squidlet : ended\n");
  fflush(stdout);
  if (stream != NULL)
    fclose(stream);

  // Return success code
  return 0;
}

