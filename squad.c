// -------------- squidlet.c ---------------

// Include third party libraries
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Include own libraries
#include "thesquid.h"

// Main function for the Squad executable
int main(int argc, char** argv) {

  // Init the random generator
  srand(time(NULL));
  
  // Declare and initialise variables to process arguments
  char* tasksFilePath = NULL;
  char* squidletsFilePath = NULL;
  bool flagTextOMeter = false;
  unsigned int freq = 1;

  // Loop on the arguments to process the prior arguments
  for (int iArg = 0; iArg < argc; ++iArg) {

    // -freq <delay in second between step>
    if (strcmp(argv[iArg], "-freq") == 0 && iArg < argc - 1) {

      // Memorize a pointer to the path to the task file
      ++iArg;
      freq = atoi(argv[iArg]);

    }

    // -tasks <path to tasks file>
    if (strcmp(argv[iArg], "-tasks") == 0 && iArg < argc - 1) {

      // Memorize a pointer to the path to the task file
      ++iArg;
      tasksFilePath = argv[iArg];

    }

    // -squidlets <path to squidlets file>
    if (strcmp(argv[iArg], "-squidlets") == 0 && iArg < argc - 1) {

      // Memorize a pointer to the path to the squidlet file
      ++iArg;
      squidletsFilePath = argv[iArg];

    }

    // -verbose
    if (strcmp(argv[iArg], "-verbose") == 0) {

      // Set teh fag to display messages in a TextOMeter
      flagTextOMeter = true;

    }

    // -help
    if (strcmp(argv[iArg], "-help") == 0) {

      // Display the help message and quit
      printf("squad [-squidlets <path to squidlets config file>] ");
      printf("[-verbose] [-tasks <path to tasks file>] ");
      printf("[-freq <delay in second between step, default: 1>] ");
      printf("[-check] [-benchmark] [-help]\n");
      return 0;

    }
  }

  // Create the squad
  Squad* squad = SquadCreate();

  // If we couldn't create the squad
  if (squad == NULL) {

    fprintf(stderr, "Squad: Failed to create the squad\n");
    return 1;

  // Else, we could create the squad
  } else {

    printf("Squad : started\n");

  }

  // If the user has provided a squidlet configuration file
  if (squidletsFilePath != NULL) {

    // Open the squidlets file
    FILE* squidletsFile = fopen(squidletsFilePath, "r");
    
    // If we couldn't open the squidlets file
    if (squidletsFile == NULL) {

        // Print an error message
        fprintf(stderr, "Squad: Couldn't open the squidlets file: %s\n", 
          squidletsFilePath);
        fprintf(stderr, "errno: %s\n", strerror(errno));

        // Free memory
        SquadFree(&squad);

        // Stop here
        return 2;

    // Else, we could open the squidlets file
    } else {

      // Load the content of the squidlets file
      bool retLoadSquidlets = SquadLoadSquidlets(squad, squidletsFile);

      // If we couldn't oad the squidlets file
      if (retLoadSquidlets == false) {

        // Print an error message
        fprintf(stderr, 
          "Squad: Couldn't load the squidlets config file %s\n",
          squidletsFilePath);
        fprintf(stderr, "TheSquidErr: %s\n", TheSquidErr->_msg);
        
        // Free memory
        SquadFree(&squad);

        // Stop here
        return 3;

      }

      // Close the squidlets file
      fclose(squidletsFile);

    }
  }

  // Set the TextOMeter accordingly to the -verbose argument
  SquadSetFlagTextOMeter(squad, flagTextOMeter);

  // Loop on the arguments to process the posterior arguments
  for (int iArg = 0; iArg < argc; ++iArg) {

    // -check
    if (strcmp(argv[iArg], "-check") == 0) {

      // Check that the squidlets described in the squidlets file
      // are up and running
      bool resCheckSquidlets = SquadCheckSquidlets(squad, stdout);

      // If at least one of the squidlets is not responding
      if (resCheckSquidlets == false) {

        // Free memory
        SquadFree(&squad);

        // Stop here
        return 4;
        
      }

    }

    // -benchmark
    if (strcmp(argv[iArg], "-benchmark") == 0) {
      
      // Run the standard benchmark on the loaded squidlets
      SquadBenchmark(squad, stdout);

    }

  }

  // If the user has provided a tasks file
  if (tasksFilePath != NULL) {

    printf("Squad: Executing task file: %s\n", tasksFilePath);

    // Open the task file
    FILE* tasksFile = fopen(tasksFilePath, "r");
    
    // If we couldn't open the tasks file
    if (tasksFile == NULL) {

        // Print an error message
        fprintf(stderr, "Squad: Couldn't open the tasks file: %s\n", 
          tasksFilePath);
        fprintf(stderr, "errno: %s\n", strerror(errno));

        // Free memory
        SquadFree(&squad);

        // Stop here
        return 5;

    // Else, we could open the tasks file
    } else {
      
      // Load the task file
      bool retLoadTasks = SquadLoadTasks(squad, tasksFile);
      
      // If we couldn't load the tasks file
      if (retLoadTasks == false) {
        
        // Print an error message
        fprintf(stderr, "Squad: Couldn't load the tasks file %s\n",
          tasksFilePath);
        fprintf(stderr, "TheSquidErr: %s\n", TheSquidErr->_msg);
        
        // Free memory
        SquadFree(&squad);
        fclose(tasksFile);
        
        // Stop here
        return 6;
      }

      // Close the tasks file
      fclose(tasksFile);

      // Loop as long as there are task to complete
      while (SquadGetNbTaskToComplete(squad) > 0) {

        // Sleep between each step of the Squad
        sleep(freq);

        // Step the squad and get the completed tasks at this step
        GSet completedTasks = SquadStep(squad);
        
        // While there are completed tasks
        while (GSetNbElem(&completedTasks) > 0) {
          
          // Pop the completed task
          SquidletTaskRequest* task = GSetPop(&completedTasks);
          
          // Display the completed task
          printf("Squad : ");
          SquidletTaskRequestPrint(task, stdout);
          
          // If the task has succeeded
          if (SquidletTaskHasSucceeded(task)) {

            printf(" succeeded\n");

            // Free the task
            SquidletTaskRequestFree(&task);

          // Else, the task has failed
          } else {

            printf(" failed !!\n");

            // Put the task back into the set of tasks to complete
            SquadTryAgainTask(squad, task);

          }

        }

      }

    }

  }
  
  // Free memory
  SquadFree(&squad);

  printf("Squad : ended\n");

  // Return success code
  return 0;
}

