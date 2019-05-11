#include <stdlib.h>
#include <stdio.h>
#include <time.h>
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

void UnitTestAll() {
  UnitTestSquad();
  UnitTestSquidlet();
  printf("UnitTestAll OK\n");
}

void UnitTestDummy() {
  const int nbSquidlet = 2;
  int squidletId = -1;
  int port[2] = {9000, 9001};
  int nbRequest = 3;
  // Create the squidlet processes
  for (int iSquidlet = 0; iSquidlet < nbSquidlet; ++iSquidlet) {
    if (fork() == 0) {
      squidletId = iSquidlet;
      break;
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
      printf("Squidlet #%d received task %d\n", squidletId, request._id);
      SquidletProcessRequest(squidlet, &request);
      --nbRequest;
    } while (nbRequest > 0);
    SquidletFree(&squidlet);
    printf("Squidlet #%d ended\n", squidletId);
    fflush(stdout);
  } else {
    
    // In the squad process
    
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
    fp = fopen("unitTestDummy.json", "r");
    SquadLoad(squad, fp);
    fclose(fp);
    // Wait to be sure the squidlet are up and running
    sleep(2);
    // Send the task request to the squid
    SquidletTaskRequest request = {._id = SquidletTaskID_Dummy};
    int data = 1;
    do {
      GSetIterForward iter = 
        GSetIterForwardCreateStatic(SquadSquidlets(squad));
      do {
        SquidletInfo* squidlet = GSetIterGet(&iter);
        bool ret = SquadSendTaskRequest(squad, &request, squidlet);
        if (!ret) {
          printf("Failed to send task request to %s:%d\n", 
            squidlet->_ip, squidlet->_port);
          printf("errno: %s\n", strerror(errno));
        } else {
          ret = SquadSendTaskData_Dummy(squad, squidlet, data);
          if (!ret) {
            printf("Failed to send data to %s:%d\n", squidlet->_ip, 
              squidlet->_port);
            printf("errno: %s\n", strerror(errno));
          }
        }
        ++data;
      } while (GSetIterStep(&iter));

      // Todo : get the result from the squidlet
      sleep(2);

      --nbRequest;
    } while (nbRequest > 0);
    SquadFree(&squad);
    printf("Squad ended\n");
    printf("UnitTestDummy OK\n");
    fflush(stdout);
  }
}

int main() {
  //UnitTestAll();
  UnitTestDummy();

  // Return success code
  return 0;
}

