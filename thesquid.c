// ============ THESQUID.C ================

// ================= Include =================

#include "thesquid.h"
#if BUILDMODE == 0
  #include "thesquid-inline.c"
#endif

// -------------- SquidletInfo

// ================ Functions definition ====================

// ================ Functions implementation ====================

// Return a new SquidletInfo with IP 'ip' and port 'port'
SquidletInfo* SquidletInfoCreate(char* ip, int port) {
  // Allocate memory for the squidletInfo
  SquidletInfo* that = PBErrMalloc(TheSquidErr, sizeof(SquidletInfo));
  
  // Init properties
  that->_ip = strdup(ip);
  that->_port = port;

  // Return the new squidletInfo
  return that;
}

// Free the memory used by the SquidletInfo 'that'
void SquidletInfoFree(SquidletInfo** that) {
  // If the pointer is null there is nothing to do
  if (that == NULL || *that == NULL)
    return;

  // Free memory
  free((*that)->_ip);
  free(*that);
  *that = NULL;
}

// -------------- Squad

// ================ Functions definition ====================

// Decode the JSON info of a Squad
bool SquadDecodeAsJSON(Squad* that, JSONNode* json);

// ================ Functions implementation ====================

// Return a new Squad
Squad* SquadCreate(void) {
  // Allocate memory for the squad
  Squad* that = PBErrMalloc(TheSquidErr, sizeof(Squad));
  
  // Open the socket
  that->_fd = socket(AF_INET, SOCK_STREAM, 0);
  
  // If we couldn't open the socket
  if (that->_fd == -1) {
    // Free memory and return null
    free(that);
    return NULL;
  }

  // Init properties
  that->_squidlets = GSetCreateStatic();

  // Return the new squad
  return that;
}

// Free the memory used by the Squad 'that'
void SquadFree(Squad** that) {
  // If the pointer is null there is nothing to do
  if (that == NULL || *that == NULL)
    return;

  // Close the socket
  close((*that)->_fd);

  // Free memory
  while (GSetNbElem(SquadSquidlets(*that)) > 0) {
    SquidletInfo* squidletInfo = GSetPop((GSet*)SquadSquidlets(*that));
    SquidletInfoFree(&squidletInfo);
  }
  free(*that);
  *that = NULL;
}

// Load the Squad info from the 'stream' into the 'that'
// Return true if it could load the info, else false
bool SquadLoad(Squad* const that, FILE* const stream) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
  if (stream == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'stream' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Discard the current squidlets info if any
  while (GSetNbElem(SquadSquidlets(that)) > 0) {
    SquidletInfo* squidletInfo = GSetPop((GSet*)SquadSquidlets(that));
    SquidletInfoFree(&squidletInfo);
  }

  // Declare a json to load the encoded data
  JSONNode* json = JSONCreate();

  // Load the whole encoded data
  if (JSONLoad(json, stream) == false) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "JSONLoad failed");
    PBErrCatch(JSONErr);
  }

  // Decode the data from the JSON
  if (!SquadDecodeAsJSON(that, json)) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "SquadDecodeAsJSON failed");
    PBErrCatch(JSONErr);
  }

  // Free the memory used by the JSON
  JSONFree(&json);
  
  // Return the succes code
  return true;
}

// Decode the JSON info of a Squad
bool SquadDecodeAsJSON(Squad* that, JSONNode* json) {

  // Get the property _squidlets from the JSON
  JSONNode* prop = JSONProperty(json, "_squidlets");
  if (prop == NULL) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "SquadDecodeAsJSON failed");
    return false;
  }

  // Get the number of squidlets
  int nbSquidlet = JSONGetNbValue(prop);
  
  // Loop on squidlets
  for (int iSquidlet = 0; iSquidlet < nbSquidlet; ++iSquidlet) {
    // Get the JSON node for this squidlet
    JSONNode* propSquidlet = JSONValue(prop, iSquidlet);
    
    // Get the property _ip of the squidlet
    JSONNode* propIp = JSONProperty(propSquidlet, "_ip");
    if (propIp == NULL) {
      JSONErr->_type = PBErrTypeUnitTestFailed;
      sprintf(JSONErr->_msg, "SquadDecodeAsJSON failed");
      return false;
    }

    // Get the property _port of the squidlet
    JSONNode* propPort = JSONProperty(propSquidlet, "_port");
    if (propPort == NULL) {
      JSONErr->_type = PBErrTypeUnitTestFailed;
      sprintf(JSONErr->_msg, "SquadDecodeAsJSON failed");
      return false;
    }

    // Create a SquidletInfo
    char* ip = JSONLabel(JSONValue(propIp, 0));
    int port = atoi(JSONLabel(JSONValue(propPort, 0)));
    SquidletInfo* squidletInfo = SquidletInfoCreate(ip, port);
    
    // Add the the squidlet to the set of squidlets
    GSetAppend((GSet*)SquadSquidlets(that), squidletInfo);
  }

  // Return the success code
  return true;
}

// Send the task request 'request' from the Squad 'that' to its
// Squidlet 'squidlet'
// Return true if the request could be sent, false else
bool SquadSendTaskRequest(Squad* const that, 
  const SquidletTaskRequest* const request, 
  SquidletInfo* const squidlet) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
  if (request == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'request' is null");
    PBErrCatch(TheSquidErr);
  }
  if (squidlet == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'squidlet' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // If the requested task is null
  if (request->_id == SquidletTaskID_Null) {
    // Nothing to do
    return true;
  }
  
  // Create a socket
  short fd = socket(AF_INET, SOCK_STREAM, 0);
  
  // If we couldn't create the socket
  if (fd == -1)
    return false;
  
  // Create the data for the connection to the squidlet
  struct sockaddr_in remote = {0};
  remote.sin_addr.s_addr = inet_addr(squidlet->_ip);
  remote.sin_family = AF_INET;
  remote.sin_port = htons(squidlet->_port);

  // Connect to the squidlet
  if (connect(fd, (struct sockaddr*)&remote, 
    sizeof(struct sockaddr_in)) == -1) {
    // If the connection failed
    return false;
  }

  // Declare a variable to memorize the timeout and set it to 1 second
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;  
  if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, 
    (char*)&tv, sizeof(tv)) == -1) {
    // If we couldn't set the timeout
    return false;
  }

  // Send the task request
  int flags = 0;
  if (send(fd, request, sizeof(SquidletTaskRequest), flags) == -1) {
    // If we couldn't set the timeout
    return false;
  }

  // Return the success code
  return true;
}

// -------------- Squidlet

// ================ Functions definition ====================

// ================ Functions implementation ====================

// Return a new Squidlet listening to the port 'port'
// If 'port' equals -1 select automatically one available
Squidlet* SquidletCreateOnPort(int port) {
  // Allocate memory for the squidlet
  Squidlet* that = PBErrMalloc(TheSquidErr, sizeof(Squidlet));
  
  // Open the socket
  that->_fd = socket(AF_INET, SOCK_STREAM, 0);
  
  // If we couldn't open the socket
  if (that->_fd == -1) {
    // Free memory and return null
    free(that);
    return NULL;
  }

  // Get the hostname
  if (gethostname(that->_hostname, sizeof(that->_hostname)) == -1) {
    // If we couldn't get the hostname, free memory and 
    // return null
    free(that);
    return NULL;
  }
  
  // Get the info about the host
  that->_host = gethostbyname(that->_hostname); 
  if (that->_host == NULL) {
    // If we couldn't get the host info, free memory and 
    // return null
    free(that);
    return NULL;
  }
  
  // Init the port and socket info
  if (port != -1)
    that->_port = port;
  else
    that->_port = THESQUID_PORTMIN;
  memset(&(that->_sock), 0, sizeof(struct sockaddr_in));
  that->_sock.sin_family = AF_INET;
  that->_sock.sin_addr.s_addr = *(uint32_t*)(that->_host->h_addr_list[0]);
  that->_sock.sin_port = htons(that->_port);
  
  // Bind the socket on the first available port
  if (port == -1) {
    while (bind(that->_fd, (struct sockaddr *)&(that->_sock), 
      sizeof(struct sockaddr_in)) == -1 && 
      that->_port < THESQUID_PORTMAX) {
      ++(that->_port);
    }
    if (that->_port == THESQUID_PORTMAX) {
      // If we couldn't bind the socket, close it, free memory and 
      // return null
      close(that->_fd);
      free(that);
      return NULL;
    }
  } else {
    if (bind(that->_fd, (struct sockaddr *)&(that->_sock), 
      sizeof(struct sockaddr_in)) == -1) {
      // If we couldn't bind the socket, close it, free memory and 
      // return null
      close(that->_fd);
      free(that);
      return NULL;
    }
  }
  
  // Start listening through the socket
  if (listen(that->_fd, THESQUID_NBMAXPENDINGCONN) == -1) {
    // If we can't listen through the socket, close it, free memory and 
    // return null
    close(that->_fd);
    free(that);
    return NULL;
  }

  // Init the PID
  that->_pid = getpid();

  // Return the new squidlet
  return that;
}

// Free the memory used by the Squidlet 'that'
void SquidletFree(Squidlet** that) {
  // If the pointer is null there is nothing to do
  if (that == NULL || *that == NULL)
    return;

  // Close the socket
  close((*that)->_fd);

  // Free memory
  free(*that);
  *that = NULL;
}

// Print the PID, Hostname, IP and Port of the Squidlet 'that' on the 
// 'stream'
// Example: 100 localhost 0.0.0.0 3000
void SquidletPrint(const Squidlet* const that, FILE* const stream) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
  if (stream == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'stream' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Print the info on the stream
  fprintf(stream, "%d %s %s %d", SquidletGetPID(that), 
    SquidletHostname(that), SquidletIP(that), SquidletGetPort(that));
}

// Wait for a task request to be received by the Squidlet 'that'
// Return the received task request
SquidletTaskRequest SquidletWaitRequest(Squidlet* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Declare a buffer to receive the payload of the connection
  SquidletTaskRequest taskRequest;
  taskRequest._id = SquidletTaskID_Null;

  // Declare a variable to memorize the info about the incoming connection
  struct sockaddr_in incomingSock;
  socklen_t incomingSockSize = sizeof(incomingSock);
  
  // Accept a connection
  int connection = accept(that->_fd, (struct sockaddr *)&incomingSock,
    &incomingSockSize);
  
  // If the connection was accepted
  if (connection >= 0) {

    // Receive the payload
    int flags = 0;
    int sizePayload = recv(connection, &taskRequest, 
      sizeof(SquidletTaskRequest), flags);

    // If we couldn't received the payload
    if(sizePayload != sizeof(SquidletTaskRequest)) {
      taskRequest._id = SquidletTaskID_Null;
    }
  }

  // Return the received task request
  return taskRequest;
}

// Process the task request 'request' with the Squidlet 'that'
void SquidletProcessRequest(Squidlet* const that, 
  const SquidletTaskRequest* const request) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
  if (request == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'request' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Switch according to the request id
  switch (request->_id) {
    case SquidletTaskID_Dummy:
      SquidletProcessRequest_Dummy(that);
      break;
    default:
      break;
  }
}

// Process a dummy task request with the Squidlet 'that'
// This task only sleep for 2 seconds and serve only unit test purpose
void SquidletProcessRequest_Dummy(Squidlet* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Sleep for 2 seconds
  sleep(2);
}  
