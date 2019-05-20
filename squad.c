#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "thesquid.h"

int main(int argc, char** argv) {
  // Process arguments
  char* tasksFilePath = NULL;
  char* squadFilePath = NULL;
  bool flagTextOMeter = false;
  for (int iArg = 0; iArg < argc; ++iArg) {
    if (strcmp(argv[iArg], "-tasks") == 0) {
      ++iArg;
      tasksFilePath = argv[iArg];
    }
    if (strcmp(argv[iArg], "-squad") == 0) {
      ++iArg;
      squadFilePath = argv[iArg];
    }
    if (strcmp(argv[iArg], "-verbose") == 0) {
      flagTextOMeter = true;
    }
    if (strcmp(argv[iArg], "-help") == 0) {
      printf("squad [-verbose] [-tasks <path to tasks file>] ");
      printf("[-squad <path to squad config file>] [-help]\n");
      exit(0);
    }
  }
  if (tasksFilePath == NULL) {
    printf("Squad: No tasks file provided\n");
    exit(1);
  }
  if (squadFilePath == NULL) {
    printf("Squad: No squad config file provided\n");
    exit(1);
  }

  // Create the squad
  Squad* squad = SquadCreate();
  if (squad == NULL) {
    printf("Failed to create the squad\n");
    exit(1);
  }
  printf("Squad : started\n");
  fflush(stdout);

  // Load the squad config file
  if (SquadLoad(squad, squadFilePath) == false) {
    printf("Squad: Couldn't load the squad config file %s\n",
      squadFilePath);
    printf("TheSquidErr: %s\n", TheSquidErr->_msg);
    printf("errno: %s\n", strerror(errno));
    SquadFree(&squad);
    exit(1);
  }
  
  // Set the TextOMeter
  SquadSetFlagTextOMeter(squad, flagTextOMeter);

  // Free memory
  SquadFree(&squad);
  printf("Squad : ended\n");
  fflush(stdout);

  // Return success code
  return 0;
}

