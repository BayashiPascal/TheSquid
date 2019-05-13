#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "thesquid.h"

void UnitTestSquad() {
  Squad* squad = SquadCreate();
  if (squad == NULL) {
    TheSquidErr->_type = PBErrTypeUnitTestFailed;
    sprintf(TheSquidErr->_msg, "SquadCreate failed");
    PBErrCatch(TheSquidErr);
  }
  FILE* fp = fopen("unitTestSquad.json", "r");
  if (fp == NULL || SquadLoad(squad, fp) == false) {
    TheSquidErr->_type = PBErrTypeUnitTestFailed;
    sprintf(TheSquidErr->_msg, "SquadLoad failed");
    PBErrCatch(TheSquidErr);
  }
  fclose(fp);
  SquadFree(&squad);
  if (squad != NULL) {
    TheSquidErr->_type = PBErrTypeUnitTestFailed;
    sprintf(TheSquidErr->_msg, "SquadFree failed");
    PBErrCatch(TheSquidErr);
  }
  printf("UnitTestSquad OK\n");
}

void UnitTestSquidlet() {
  Squidlet* squidlet = SquidletCreate();
  if (squidlet == NULL) {
    TheSquidErr->_type = PBErrTypeUnitTestFailed;
    sprintf(TheSquidErr->_msg, "SquidletCreate failed");
    PBErrCatch(TheSquidErr);
  }
  SquidletPrint(squidlet, stdout);
  printf("\n");
  SquidletFree(&squidlet);
  if (squidlet != NULL) {
    TheSquidErr->_type = PBErrTypeUnitTestFailed;
    sprintf(TheSquidErr->_msg, "SquidletFree failed");
    PBErrCatch(TheSquidErr);
  }
  printf("UnitTestSquidlet OK\n");
}

void UnitTestDummy() {
  const int nbSquidlet = 2;
  int squidletId = -1;
  int port[2] = {9000, 9001};
  int nbRequest = 3;
  pid_t pidSquidlet[2];
  // Create the squidlet processes
  for (int iSquidlet = 0; iSquidlet < nbSquidlet; ++iSquidlet) {
    int pid = fork();
    if (pid == 0) {
      squidletId = iSquidlet;
      break;
    } else {
      pidSquidlet[iSquidlet] = pid;
    }
  }
  if (squidletId != -1) {
    
    // In a squidlet process
    
    Squidlet* squidlet = SquidletCreateOnPort(port[squidletId]);
    if (squidlet == NULL) {
      printf("Failed to create the squidlet #%d\n", squidletId);
      printf("errno: %s\n", strerror(errno));
    }
    printf("Squidlet #%d : ", squidletId);
    SquidletPrint(squidlet, stdout);
    printf("\n");
    do {
      SquidletTaskRequest request = SquidletWaitRequest(squidlet);
      SquidletProcessRequest(squidlet, &request);
    } while (!Squidlet_CtrlC);
    SquidletFree(&squidlet);
    printf("Squidlet #%d ended\n", squidletId);
    fflush(stdout);
    exit(0);
  } else {
    
    // In the squad process
    
    // Create the Squad
    Squad* squad = SquadCreate();
    if (squad == NULL) {
      printf("Failed to create the squad\n");
      printf("errno: %s\n", strerror(errno));
    }
    // Automatically create the config file
    FILE* fp = fopen("unitTestDummy.json", "w");
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    struct hostent* host = gethostbyname(hostname); 
    char* ip = inet_ntoa(*((struct in_addr*)host->h_addr_list[0]));
    fprintf(fp, "{\"_squidlets\":[");
    for (int iSquidlet = 0; iSquidlet < nbSquidlet; ++iSquidlet) {
      fprintf(fp, "{\"_ip\":\"%s\",\"_port\":\"%d\"}", 
        ip, port[iSquidlet]);
      if (iSquidlet < nbSquidlet - 1)
        fprintf(fp, ",");
    }
    fprintf(fp, "]}");
    fclose(fp);
    // Load the info about the squidlet from the config file
    fp = fopen("unitTestDummy.json", "r");
    SquadLoad(squad, fp);
    fclose(fp);
    // Wait to be sure the squidlets are up and running
    sleep(2);
    // Create all the tasks
    time_t maxWait = 5;
    for (unsigned long id = 0; 
      id < (unsigned long)(nbRequest * nbSquidlet); ++id) {
      SquadAddTask_Dummy(squad, id, maxWait);
    }
    // Loop until all the tasks are completed or give up after 10s
    time_t startTime = time(NULL);
    do {
      
      // Step the Squad
      GSet completedTasks = SquadStep(squad);
      sleep(1);
      while (GSetNbElem(&completedTasks) > 0L) {
        SquidletTaskRequest* task = GSetPop(&completedTasks);
        printf("squad : completed task #%ld\n", task->_id);
        SquidletTaskRequestFree(&task);
      }
      
    } while (SquadGetNbTaskToComplete(squad) > 0L && 
      time(NULL) - startTime <= 60);
    // Kill the child process
    for (int iSquidlet = 0; iSquidlet < nbSquidlet; ++iSquidlet) {
      if (kill(pidSquidlet[iSquidlet], SIGINT) < 0) {
        printf("Couldn't kill squidlet %d\n", pidSquidlet[iSquidlet]);
      }
    }
    // Wait for the child to be killed
    sleep(2);
    // Free memory
    SquadFree(&squad);
    printf("Squad ended\n");
    printf("UnitTestDummy OK\n");
    fflush(stdout);
  }
}

void UnitTestBenchmark() {
  printf("-- Benchmark --\n");
  printf("Execution on local device:\n");
  printf("nbLoopPerTask\tnbBytePayload\tnbTaskComp\ttimeMsPerTask\n");

  for (size_t sizePayload = 100; sizePayload <= 10000000; 
    sizePayload *= 10) {
    char* buffer = PBErrMalloc(TheSquidErr, sizePayload);
    buffer[0] = 1;
    for (size_t i = 1; i < sizePayload - 1; ++i)
      buffer[i] = 'a' + i % 26;
    buffer[sizePayload - 1] = 0;
    struct timeval stop, start;
    gettimeofday(&start, NULL);
    unsigned long nb = 0;
    do {
      TheSquidBenchmark(buffer);
      ++nb;
      gettimeofday(&stop, NULL);
    } while (stop.tv_sec - start.tv_sec < 10);
    unsigned long deltams = (stop.tv_sec - start.tv_sec) * 1000000 + 
      stop.tv_usec - start.tv_usec;
    float timePerTaskMs = (float) deltams / (float)nb;
    printf("001\t%08u\t%07lu\t%011.2f\n", sizePayload, nb, timePerTaskMs);
    free(buffer);
  }

  printf("Execution on TheSquid:\n");

  // Create the Squad
  Squad* squad = SquadCreate();
  if (squad == NULL) {
    printf("Failed to create the squad\n");
    printf("errno: %s\n", strerror(errno));
  }
  // Load the info about the squidlet from the config file
  FILE* fp = fopen("unitTestBenchmark.json", "r");
  SquadLoad(squad, fp);
  fclose(fp);
  // Loop on payload size
  time_t maxWait = 100;
  unsigned int id = 0;
  for (size_t sizePayload = 100; sizePayload <= 10000000; 
    sizePayload *= 10) {
    // Loop on nbLoop
    for (int nbLoop = 1; nbLoop <= 32; nbLoop *= 2) {

      // Loop until all the tasks are completed or give up after 10s
      struct timeval stop, start;
      gettimeofday(&start, NULL);
      unsigned long nb = 0;
      do {
        
        // Create benchmark tasks
        for (int i = SquadGetNbSquidlets(squad); i--;)
          SquadAddTask_Benchmark(squad, id++, maxWait, nbLoop, 
            sizePayload);

        // Step the Squad
        GSet completedTasks = SquadStep(squad);
        nb += GSetNbElem(&completedTasks);
        while (GSetNbElem(&completedTasks) > 0L) {
          SquidletTaskRequest* task = GSetPop(&completedTasks);
          SquidletTaskRequestFree(&task);
        }
        
        gettimeofday(&stop, NULL);
      } while (stop.tv_sec - start.tv_sec < 10);
      unsigned long deltams = (stop.tv_sec - start.tv_sec) * 1000000 + 
        stop.tv_usec - start.tv_usec;
      float timePerTaskMs = (float) deltams / (float)nb;
      printf("%03d\t%08u\t%07lu\t%011.2f\n", nbLoop, sizePayload, nb, 
        timePerTaskMs);
    }
  }
  // Free memory
  SquadFree(&squad);
  printf("Squad ended\n");

  printf("UnitTestBenchmark OK\n");
}

void UnitTestAll() {
  UnitTestSquad();
  UnitTestSquidlet();
  UnitTestDummy();
  //UnitTestBenchmark();
  printf("UnitTestAll OK\n");
}

int main() {
  UnitTestAll();
  // Return success code
  return 0;
}

