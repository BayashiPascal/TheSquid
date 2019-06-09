#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "thesquid.h"

int main(int argc, char** argv) {
  // Process arguments
  int port = -1;
  uint32_t ip = 0;
  char* outputFilePath = NULL;
  for (int iArg = 0; iArg < argc; ++iArg) {
    if (strcmp(argv[iArg], "-port") == 0) {
      ++iArg;
      port = atoi(argv[iArg]);
    }
    if (strcmp(argv[iArg], "-ip") == 0) {
      ++iArg;
      unsigned int v[4];
      sscanf(argv[iArg], "%d.%d.%d.%d", v, v + 1, v + 2, v + 3);
      for (int i = 0; i < 4; ++i) {
        ((unsigned char*)(&ip))[i] = v[i];
      }
    }
    if (strcmp(argv[iArg], "-verbose") == 0) {
      ++iArg;
      outputFilePath = argv[iArg];
    }
    if (strcmp(argv[iArg], "-help") == 0) {
      printf("squidlet [-verbose <stdout | file path>] ");
      printf("[-ip <a.b.c.d>] [-port <port>] [-temp] [-help]\n");
      exit(0);
    }
  }
  
  // Create the squidlet
  Squidlet* squidlet = SquidletCreateOnPort(ip, port);
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

  for (int iArg = 0; iArg < argc; ++iArg) {
    if (strcmp(argv[iArg], "-temp") == 0) {
      char* temperature = SquidletGetTemperature(squidlet);
      if (stream == NULL)
        printf("temperature: %s\n", temperature);
      else
        fprintf(stream, "temperature: %s\n", temperature);
      if (temperature != NULL)
        free(temperature);
    }
  }
  
  // Set the handler for SIGPIPE
  signal(SIGPIPE, SquidletHandlerSigPipe);
  
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

