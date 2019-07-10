// -------------- squidlet.c ---------------

// Include third party libraries
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Include own libraries
#include "thesquid.h"

// Main function for the Squidlet executable
int main(int argc, char** argv) {

  // Declare and initialise variables to process arguments
  int port = -1;
  uint32_t ip = 0;
  char* outputFilePath = NULL;

  // Loop on the arguments to process the prior arguments
  for (int iArg = 0; iArg < argc; ++iArg) {
    
    // -ip <a.b.c.d>
    if (strcmp(argv[iArg], "-ip") == 0 && iArg < argc - 1) {

      // Decode the IP adress to which the Squidlet is attached
      ++iArg;
      unsigned int v[4] = {0, 0, 0 ,0};
      int ret = sscanf(argv[iArg], "%d.%d.%d.%d", 
        v, v + 1, v + 2, v + 3);

      // If we couldn't decode the IP
      if (ret == EOF) {
        fprintf(stderr, "Failed to decode the IP adress.\n");
        return 1;
      }

      // Encode the IP into a 4 bytes integer
      for (int i = 0; i < 4; ++i) {
        ((unsigned char*)(&ip))[i] = v[i];
      }

    }
    
    // -port <port>
    if (strcmp(argv[iArg], "-port") == 0 && iArg < argc - 1) {

      // Decode the value of the port on which the Squidlet is listening
      ++iArg;
      port = atoi(argv[iArg]);

    }

    // -stream <stdout | file path>
    if (strcmp(argv[iArg], "-stream") == 0 && iArg < argc - 1) {

      // Decode the output stream 
      ++iArg;
      outputFilePath = argv[iArg];

    }
    
    // -help
    if (strcmp(argv[iArg], "-help") == 0) {

      // Display the help message and quit
      printf("squidlet [-ip <a.b.c.d>] [-port <port>] ");
      printf("[-stream <stdout | file path>] [-temp] [-help]\n");
      return 0;

    }
  }
  
  // Create the squidlet
  Squidlet* squidlet = SquidletCreateOnPort(ip, port);

  // If we couldn't create the Squidlet
  if (squidlet == NULL) {
    fprintf(stderr, "Failed to create the squidlet\n");
    fprintf(stderr, "TheSquidErr: %s\n", TheSquidErr->_msg);
    fprintf(stderr, "errno: %s\n", strerror(errno));
    return 2;
  }

  // Display info about the Squidlet:
  // <pid> <hostname> <ip>:<port>
  printf("Squidlet : ");
  SquidletPrint(squidlet, stdout);
  printf("\n");
  
  // Set the output stream to stdout by default
  FILE* stream = stdout;
  
  // If the user provided an output stream
  if (outputFilePath != NULL) {
    
    // If the user requested the standard output stream
    if (strcmp(outputFilePath, "stdout") == 0) {
      
      // Set the output stream of the Squidlet to the standard output
      SquidletSetStreamInfo(squidlet, stdout);
    
    // else if the user requested a different stream
    } else if (outputFilePath != NULL) {
      
      // Open the requested stream
      stream = fopen(outputFilePath, "w");
      
      // If we couldn't open the requested stream
      if (stream == NULL) {
        fprintf(stderr, "Failed to open the file %s\n", outputFilePath);
        return 3;
      }
      
      // Set the output stream of the Squidlet to the requested stream
      SquidletSetStreamInfo(squidlet, stream);

    }

  // Else, the user hasn't requested any output stream
  } else {
    
    // Turn off the output stream of the Squidlet
    SquidletSetStreamInfo(squidlet, NULL);
    
  }

  // Loop on the arguments to process the posterior arguments
  for (int iArg = 0; iArg < argc; ++iArg) {

    // -temp(erature)
    if (strcmp(argv[iArg], "-temp") == 0) {

      // Get the temperature of the Squidlet
      float temperature = SquidletGetTemperature(squidlet);

      // Display the temperature
      fprintf(stream, "Squidlet : temperature: %f\n", temperature);

    }
  }
  
  // Set the handler for SIGPIPE
  signal(SIGPIPE, SquidletHandlerSigPipe);
  
  // Loop until the Squidlet is killed by Ctrl-C:
  // kill -INT <squidlet's pid>
  do {
    
    // Wait for a request
    SquidletTaskRequest request = SquidletWaitRequest(squidlet);
    
    // Process the received request
    SquidletProcessRequest(squidlet, &request);

  } while (!Squidlet_CtrlC);

  // Free memory
  SquidletFree(&squidlet);

  // Sayonara
  fprintf(stream, "Squidlet : ended\n");

  // Close the output stream if necessary
  if (stream != NULL && stream != stdout)
    fclose(stream);

  // Return success code
  return 0;
}

