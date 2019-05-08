#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "thesquid.h"

void UnitTestSquad() {

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

int main() {
  UnitTestAll();

  // Return success code
  return 0;
}

