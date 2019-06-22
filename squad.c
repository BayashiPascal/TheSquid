#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "thesquid.h"

int main(int argc, char** argv) {
  // Process arguments
  char* tasksFilePath = NULL;
  char* squidletsFilePath = NULL;
  bool flagTextOMeter = false;
  for (int iArg = 0; iArg < argc; ++iArg) {
    if (strcmp(argv[iArg], "-tasks") == 0) {
      ++iArg;
      tasksFilePath = argv[iArg];
    }
    if (strcmp(argv[iArg], "-squidlets") == 0) {
      ++iArg;
      squidletsFilePath = argv[iArg];
    }
    if (strcmp(argv[iArg], "-verbose") == 0) {
      flagTextOMeter = true;
    }
    if (strcmp(argv[iArg], "-help") == 0) {
      printf("squad [-verbose] [-tasks <path to tasks file>] ");
      printf("[-squidlets <path to squidlets config file>] ");
      printf("[-check] [-help]\n");
      exit(0);
    }
  }
  if (squidletsFilePath == NULL) {
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

  // Load the squidlets config file
  FILE* squidletsFile = fopen(squidletsFilePath, "r");
  if (squidletsFile == NULL) {
      printf("Squad: Couldn't open the squidlets file: %s\n", 
        squidletsFilePath);
  } else {
    if (SquadLoadSquidlets(squad, squidletsFile) == false) {
      printf("Squad: Couldn't load the squidlets config file %s\n",
        squidletsFilePath);
      printf("TheSquidErr: %s\n", TheSquidErr->_msg);
      printf("errno: %s\n", strerror(errno));
      SquadFree(&squad);
      exit(1);
    }
    fclose(squidletsFile);
  }
  // Set the TextOMeter
  SquadSetFlagTextOMeter(squad, flagTextOMeter);
  // Execute the requested actions
  for (int iArg = 0; iArg < argc; ++iArg) {
    if (strcmp(argv[iArg], "-check") == 0) {
      bool res = SquadCheckSquidlets(squad, stdout);
      (void)res;
    }
  }
  if (tasksFilePath != NULL) {
    printf("Squad: Executing task file: %s\n", tasksFilePath);
    FILE* stream = fopen(tasksFilePath, "r");
    if (!SquadLoadTasks(squad, stream)) {
      printf("Squad: Couldn't load the tasks file %s\n",
        tasksFilePath);
      printf("TheSquidErr: %s\n", TheSquidErr->_msg);
      printf("errno: %s\n", strerror(errno));
      SquadFree(&squad);
      exit(1);
    }
    fclose(stream);
    // Loop until all the tasks are completed
    do {
      // Wait 1s between each step of the Squad
      sleep(1);
      // Display completed tasks
      GSet completedTasks = SquadStep(squad);
      while (GSetNbElem(&completedTasks) > 0L) {
        SquidletTaskRequest* task = GSetPop(&completedTasks);
        printf("Squad : ");
        SquidletTaskRequestPrint(task, stdout);
        if (strstr(task->_buffer, "\"success\":\"1\"") == NULL) {
          printf(" failed !!\n");
        } else {
          printf(" succeeded\n");
        }
        SquidletTaskRequestFree(&task);
      }
    } while (SquadGetNbTaskToComplete(squad) > 0L);
  }
  
  // Free memory
  SquadFree(&squad);
  printf("Squad : ended\n");
  fflush(stdout);

  // Return success code
  return 0;
}

