#include <stdlib.h>
#include <stdio.h>
#include "thesquid.h"

void UnitTestSquad() {
  Squad* squad = SquadCreate();
  if (squad == NULL) {
    TheSquidErr->_type = PBErrTypeUnitTestFailed;
    sprintf(TheSquidErr->_msg, "SquadCreate failed");
    PBErrCatch(TheSquidErr);
  }
  FILE* fp = fopen("unitTestSquad.json", "r");
  if (fp == NULL || SquadLoadSquidlets(squad, fp) == false) {
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

void UnitTestSquadCheckSquidlets() {
  const int nbSquidlet = 2;
  int squidletId = -1;
  int port[2] = {9000, 9001};
  pid_t pidSquidlet[2];
  char buffer[100];
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
    
    Squidlet* squidlet = SquidletCreateOnPort(0, port[squidletId]);
    if (squidlet == NULL) {
      printf("Failed to create the squidlet #%d\n", squidletId);
      printf("errno: %s\n", strerror(errno));
    }
    sprintf(buffer, "unitTestDummySquidlet%d.log", squidletId);
    FILE* stream = fopen(buffer, "w");
    SquidletSetStreamInfo(squidlet, stream);
    do {
      SquidletTaskRequest request = SquidletWaitRequest(squidlet);
      SquidletProcessRequest(squidlet, &request);
    } while (!Squidlet_CtrlC);
    SquidletFree(&squidlet);
    fclose(stream);
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
      fprintf(fp, 
        "{\"_name\":\"UnitTestDummy\"\"_ip\":\"%s\",\"_port\":\"%d\"}", 
        ip, port[iSquidlet]);
      if (iSquidlet < nbSquidlet - 1)
        fprintf(fp, ",");
    }
    fprintf(fp, "]}");
    fclose(fp);
    // Load the info about the squidlet from the config file
    fp = fopen("unitTestDummy.json", "r");
    SquadLoadSquidlets(squad, fp);
    fclose(fp);
    // Wait to be sure the squidlets are up and running
    sleep(2);
    // Check the squidlets
    bool res = SquadCheckSquidlets(squad, stdout);
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
    if (res) {
      printf("UnitTestSquadCheckSquidlets OK\n");
    } else {
      printf("UnitTestSquadCheckSquidlets NG\n");
    }
    fflush(stdout);
  }
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
  char* temperature = SquidletGetTemperature(squidlet);
  printf("squidlet temperature: %s\n", temperature);
  if (temperature != NULL) {
    free(temperature);
  }
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
  char buffer[100];
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
    
    Squidlet* squidlet = SquidletCreateOnPort(0, port[squidletId]);
    if (squidlet == NULL) {
      printf("Failed to create the squidlet #%d\n", squidletId);
      printf("errno: %s\n", strerror(errno));
    }
    sprintf(buffer, "unitTestDummySquidlet%d.log", squidletId);
    FILE* stream = fopen(buffer, "w");
    SquidletSetStreamInfo(squidlet, stream);
    printf("Squidlet #%d : ", squidletId);
    SquidletPrint(squidlet, stdout);
    printf("\n");
    do {
      SquidletTaskRequest request = SquidletWaitRequest(squidlet);
      SquidletProcessRequest(squidlet, &request);
    } while (!Squidlet_CtrlC);
    SquidletFree(&squidlet);
    fclose(stream);
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
#if BUILDARCH == 0
    // Turn on the TextOMeter
    SquadSetFlagTextOMeter(squad, true);
#endif
    // Automatically create the config file
    FILE* fp = fopen("unitTestDummy.json", "w");
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    struct hostent* host = gethostbyname(hostname); 
    char* ip = inet_ntoa(*((struct in_addr*)host->h_addr_list[0]));
    fprintf(fp, "{\"_squidlets\":[");
    for (int iSquidlet = 0; iSquidlet < nbSquidlet; ++iSquidlet) {
      fprintf(fp, 
        "{\"_name\":\"UnitTestDummy\"\"_ip\":\"%s\",\"_port\":\"%d\"}", 
        ip, port[iSquidlet]);
      if (iSquidlet < nbSquidlet - 1)
        fprintf(fp, ",");
    }
    fprintf(fp, "]}");
    fclose(fp);
    // Load the info about the squidlet from the config file
    fp = fopen("unitTestDummy.json", "r");
    SquadLoadSquidlets(squad, fp);
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
    bool flagStop = false;
    do {
      
      // Step the Squad
      GSet completedTasks = SquadStep(squad);
      //sleep(1);
      while (GSetNbElem(&completedTasks) > 0L) {
        SquidletTaskRequest* task = GSetPop(&completedTasks);
        printf("squad : ");
        SquidletTaskRequestPrint(task, stdout);
        if (strstr(task->_buffer, "\"success\":\"1\"") == NULL) {
          printf(" failed !!\n");
          flagStop = true;
        } else {
          printf(" succeeded\n");
        }
        SquidletTaskRequestFree(&task);
      }
      
    } while (SquadGetNbTaskToComplete(squad) > 0L && 
      time(NULL) - startTime <= 60 && !flagStop);
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

void UnitTestPovRay() {
  int ret = system("povray ./testPov.ini +OtestPovRef.tga +FT -D");
  (void)ret;
  const int nbSquidlet = 2;
  int squidletId = -1;
  int port[2] = {9000, 9001};
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
    
    Squidlet* squidlet = SquidletCreateOnPort(0, port[squidletId]);
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
    FILE* fp = fopen("unitTestPovRay.json", "w");
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    struct hostent* host = gethostbyname(hostname); 
    char* ip = inet_ntoa(*((struct in_addr*)host->h_addr_list[0]));
    fprintf(fp, "{\"_squidlets\":[");
    for (int iSquidlet = 0; iSquidlet < nbSquidlet; ++iSquidlet) {
      fprintf(fp, 
        "{\"_name\":\"UnitTestPovRay\"\"_ip\":\"%s\",\"_port\":\"%d\"}", 
        ip, port[iSquidlet]);
      if (iSquidlet < nbSquidlet - 1)
        fprintf(fp, ",");
    }
    fprintf(fp, "]}");
    fclose(fp);
    // Load the info about the squidlet from the config file
    fp = fopen("unitTestPovRay.json", "r");
    SquadLoadSquidlets(squad, fp);
    fclose(fp);
    // Wait to be sure the squidlets are up and running
    sleep(2);
    // Create the task
    time_t maxWait = 600;
    int id = 1;
    unsigned int sizeMinFragment = 100;
    unsigned int sizeMaxFragment = 1000;
    SquadAddTask_PovRay(squad, id, maxWait, "./testPov.ini", 
      sizeMinFragment, sizeMaxFragment);
    // Loop until all the tasks are completed or giveup after 60s
    time_t startTime = time(NULL);
    bool flagStop = false;
    do {
      sleep(1);
      // Step the Squad
      GSet completedTasks = SquadStep(squad);
      while (GSetNbElem(&completedTasks) > 0L) {
        SquidletTaskRequest* task = GSetPop(&completedTasks);
        printf("squad : ");
        SquidletTaskRequestPrint(task, stdout);
        if (strstr(task->_buffer, "\"success\":\"1\"") == NULL) {
          printf(" failed !!\n");
          flagStop = true;
        } else {
          printf(" succeeded\n");
        }
        SquidletTaskRequestFree(&task);
      }
      
    } while (SquadGetNbTaskToComplete(squad) > 0L && 
      time(NULL) - startTime <= 60 && !flagStop);
    // Kill the child process
    for (int iSquidlet = 0; iSquidlet < nbSquidlet; ++iSquidlet) {
      if (kill(pidSquidlet[iSquidlet], SIGINT) < 0) {
        printf("Couldn't kill squidlet %d\n", pidSquidlet[iSquidlet]);
      }
    }
    // Wait for the child to be killed
    sleep(2);
    // Compare the result to the reference
    GenBrush* result = GBCreateFromFile("./testPov.tga");
    GenBrush* ref = GBCreateFromFile("./testPovRef.tga");
    if (GBIsSameAs(result, ref) == false) {
      TheSquidErr->_type = PBErrTypeUnitTestFailed;
      sprintf(TheSquidErr->_msg, "UnitTestPovRay failed");
      TheSquidErr->_fatal = false;
      PBErrCatch(TheSquidErr);
      TheSquidErr->_fatal = true;
    } else {
      printf("UnitTestPovRay OK\n");
    }
    // Free memory
    SquadFree(&squad);
    GBFree(&result);
    GBFree(&ref);
  }
}

void UnitTestLoadTasks() {
  Squad* squad = SquadCreate();
  FILE* stream = fopen("./testLoadTasks.json","r");
  if (!SquadLoadTasks(squad, stream)) {
    TheSquidErr->_type = PBErrTypeUnitTestFailed;
    sprintf(TheSquidErr->_msg, "SquadLoadTasks failed");
    PBErrCatch(TheSquidErr);
  }
  fclose(stream);
  SquadFree(&squad);
  printf("UnitTestLoadTasks OK\n");
}

void UnitTestAll() {
  UnitTestSquad();
  UnitTestSquadCheckSquidlets();
  UnitTestSquidlet();
  UnitTestDummy();
  UnitTestPovRay();
  UnitTestLoadTasks();
  printf("UnitTestAll OK\n");
}

int main() {
  UnitTestAll();
  // Return success code
  return 0;
}

