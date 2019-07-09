// ============ THESQUID.C ================

// ================= Include =================

#include "thesquid.h"
#if BUILDMODE == 0
  #include "thesquid-inline.c"
#endif

// ================ global variables ====================

const char* squidletTaskTypeStr[] = {
  "Null", "Dummy", "Benchmark", "PovRay"  
};

// ================ Functions declaration ====================

// Function to receive in blocking mode 'nb' bytes of data from
// the socket 'sock' and store them into 'buffer' (which must be big 
// enough). Give up after 'timeout' seconds.
// Return true if we could read all the requested byte, false else
bool SocketRecv(
  short* sock, 
  unsigned long nb, 
  char* buffer, 
  const time_t timeout);

// -------------- SquidletInfo

// ================ Functions implementation ====================

// Return a new SquidletInfo describing a Squidlet whose name is 
// 'name', and whose attached to the address 'ip':'port'
SquidletInfo* SquidletInfoCreate(
  const char* const name, 
  const char* const ip, 
    const int const port) {
#if BUILDMODE == 0
  if (name == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'name' is null");
    PBErrCatch(TheSquidErr);
  }
  if (ip == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'ip' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Allocate memory for the squidletInfo
  SquidletInfo* that = PBErrMalloc(TheSquidErr, sizeof(SquidletInfo));
  
  // Init properties
  that->_name = strdup(name);
  that->_ip = strdup(ip);
  that->_port = port;
  that->_sock = -1;

  // Return the new squidletInfo
  return that;
}

// Free the memory used by the SquidletInfo 'that'
void SquidletInfoFree(
  SquidletInfo** that) {
  // If the pointer is null there is nothing to do
  if (that == NULL || *that == NULL)
    return;

  // Close the socket if it's opened
  if ((*that)->_sock != -1)
    close((*that)->_sock);
    
  // Free memory
  free((*that)->_name);
  free((*that)->_ip);
  free(*that);
  *that = NULL;
}

// Print the SquidletInfo 'that' on the file 'stream'
void SquidletInfoPrint(
  const SquidletInfo* const that, 
                FILE* const stream) {
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
  fprintf(stream, "%s(%s:%d)", that->_name, that->_ip, that->_port);
}

// -------------- SquidletTaskRequest

// ================ Functions implementation ====================

// Return a new SquidletTaskRequest for a task of type 'type'
// The task is identified by its 'id'. It holds a copy of 'data', a 
// string in JSON format
SquidletTaskRequest* SquidletTaskRequestCreate(
   SquidletTaskType type, 
      unsigned long id, 
      unsigned long subId, 
  const char* const data, 
       const time_t maxWait) {
#if BUILDMODE == 0
  if (data == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'data' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Allocate memory for the new SquidletTaskRequest
  SquidletTaskRequest* that = PBErrMalloc(TheSquidErr, 
    sizeof(SquidletTaskRequest));
  
  // Init properties
  that->_id = id;
  that->_subId = subId;
  that->_type = type;
  that->_data = strdup(data);
  that->_bufferResult = NULL;
  that->_maxWaitTime = maxWait;
  
  // Return the new SquidletTaskRequest
  return that;
}

// Free the memory used by the SquidletTaskRequest 'that'
void SquidletTaskRequestFree(
  SquidletTaskRequest** that) {
  // If the pointer is null there is nothing to do
  if (that == NULL || *that == NULL)
    return;
  
  // Free memory
  if ((*that)->_bufferResult != NULL)
    free((*that)->_bufferResult);
  free((*that)->_data);
  free(*that);
  *that = NULL;  
}

// Print the SquidletTaskRequest 'that' on the file 'stream'
void SquidletTaskRequestPrint(
  const SquidletTaskRequest* const that, 
                       FILE* const stream) {
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
  // Get a truncated version of the data
  char truncData[150];
  truncData[149] = '\0';
  strncpy(truncData, that->_data, 149);

  // Print the info on the stream 
  fprintf(stream, "%s(#%lu-%lu) %s", 
    squidletTaskTypeStr[that->_type], that->_id, 
    that->_subId, truncData);
}

// -------------- SquadRunningTask

// ================ Functions implementation ====================

// Return a new SquadRunningTask with the 'request' and 'squidlet'
SquadRunningTask* SquadRunningTaskCreate(
  SquidletTaskRequest* const request, 
         SquidletInfo* const squidlet) {
#if BUILDMODE == 0
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
  // Allocate memory for the new SquadRunningTask
  SquadRunningTask* that = PBErrMalloc(TheSquidErr, 
    sizeof(SquadRunningTask));
  
  // Init properties
  that->_request = request;
  that->_squidlet = squidlet;
  that->_startTime = time(NULL);
  
  // Return the new SquadRunningTask
  return that;
}

// Free the memory used by the SquadRunningTask 'that'
void SquadRunningTaskFree(
  SquadRunningTask** that) {
  // If the pointer is null there is nothing to do
  if (that == NULL || *that == NULL)
    return;
  
  // Free memory
  SquidletInfoFree(&((*that)->_squidlet));
  SquidletTaskRequestFree(&((*that)->_request));
  free(*that);
  *that = NULL;
}

// Print the SquadRunningTask 'that' on the file 'stream'
void SquadRunningTaskPrint(
  const SquadRunningTask* const that, 
                    FILE* const stream) {
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
  fprintf(stream, "[");
  SquidletTaskRequestPrint(that->_request, stream);
  fprintf(stream, "]/[");
  SquidletInfoPrint(that->_squidlet, stream);
  fprintf(stream, "]");
}

// -------------- Squad

// ================ Functions declaration ====================

// Decode the JSON info of a Squad
bool SquadDecodeAsJSON(
  Squad* that, 
  JSONNode* json);

// Refresh the content of the TextOMeter attached to the 
// Squad 'that'
void SquadUpdateTextOMeter(
  const Squad* const that);

// Add one line to the history of messages for the TextOMeter
// 'msg' must be less than 
// SQUAD_TXTOMETER_LENGTHLINEHISTORY - 10 characters long
void SquadPushHistory(
  Squad* const that, 
  char* msg);

// Request the execution of a task on a squidlet for the squad 'that'
// Return true if the request was successfull, fals else
bool SquadSendTaskOnSquidlet(
  Squad* const that, 
  SquidletInfo* const squidlet, 
  SquidletTaskRequest* const task);
  
// ================ Functions implementation ====================

// Return a new Squad
Squad* SquadCreate(void) {
  // Allocate memory for the squad
  Squad* that = PBErrMalloc(TheSquidErr, sizeof(Squad));
  
  // Open the TCP/IP socket
  // AF_INET: IPv4 Internet protocols
  // SOCK_STREAM: Provides sequenced, reliable, two-way, 
  // connection-based byte streams.
  int defaultProtocol = 0;
  that->_fd = socket(AF_INET, SOCK_STREAM, defaultProtocol);
  
  // If we couldn't open the socket
  if (that->_fd == -1) {

    // Free memory and return null
    free(that);
    return NULL;
  }

  // Init properties
  that->_squidlets = GSetCreateStatic();
  that->_tasks = GSetCreateStatic();
  that->_runningTasks = GSetCreateStatic();
  that->_flagTextOMeter = false;
  that->_textOMeter = NULL;
  for (int i = 0; i < SQUAD_TXTOMETER_NBLINEHISTORY; ++i) {
    that->_history[i][0] = '\n';
    that->_history[i][1] = '\0';
  }
  that->_countLineHistory = 0;

  // Return the new squad
  return that;
}

// Free the memory used by the Squad 'that'
void SquadFree(
  Squad** that) {
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
  while (GSetNbElem(SquadTasks(*that)) > 0) {
    SquidletTaskRequest* task = GSetPop((GSet*)SquadTasks(*that));
    SquidletTaskRequestFree(&task);
  }
  while (GSetNbElem(SquadRunningTasks(*that)) > 0) {
    SquadRunningTask* task = GSetPop((GSet*)SquadRunningTasks(*that));
    SquadRunningTaskFree(&task);
  }
  if ((*that)->_textOMeter != NULL) {
    TextOMeterFree(&((*that)->_textOMeter));
  }
  free(*that);
  *that = NULL;
}

// Load a list of tasks stored in json format from the file 'stream'
// and add them to the set of tasks of the Squad 'that'
// Return true if the tasks could be loaded, else false
// Example of list of tasks:
// {"tasks":[
//   {"SquidletTaskType":"1", "id":"1", "maxWait":"1"},  
//   {"SquidletTaskType":"2", "id":"1", "maxWait":"1", 
//    "nb":"1", "payloadSize":"1"},
//   {"SquidletTaskType":"3", "id":"1", "maxWait":"1", 
//    "ini":"./testPov.ini", "sizeMinFragment":"100", 
//    "sizeMaxFragment":"1000"}
// ]}
bool SquadLoadTasks(
  Squad* const that, 
   FILE* const stream) {
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
  // Declare a json to load the encoded data
  JSONNode* json = JSONCreate();

  // Load the whole encoded data
  if (JSONLoad(json, stream) == false) {
    TheSquidErr->_type = PBErrTypeIOError;
    sprintf(TheSquidErr->_msg, "JSONLoad failed");
    JSONFree(&json);
    return false;
  }
  
  // Get the list of tasks
  JSONNode* propTasks = JSONProperty(json, "tasks");
  if (propTasks == NULL) {
    TheSquidErr->_type = PBErrTypeInvalidData;
    sprintf(TheSquidErr->_msg, "tasks not found");
    JSONFree(&json);
    return false;
  }
  
  // Get the number of tasks
  unsigned long nbTasks = JSONGetNbValue(propTasks);
  
  // Loop on tasks
  for (unsigned long iTask = 0; iTask < nbTasks; ++iTask) {

    // Get the task
    JSONNode* propTask = JSONValue(propTasks, iTask);
    
    // Get the type of task, id and time out
    JSONNode* propType = JSONProperty(propTask, "SquidletTaskType");
    if (propType == NULL) {
      TheSquidErr->_type = PBErrTypeInvalidData;
      sprintf(TheSquidErr->_msg, "'SquidletTaskType' not found");
      JSONFree(&json);
      return false;
    }
    JSONNode* propId = JSONProperty(propTask, "id");
    if (propId == NULL) {
      TheSquidErr->_type = PBErrTypeInvalidData;
      sprintf(TheSquidErr->_msg, "'id' not found");
      JSONFree(&json);
      return false;
    }
    JSONNode* propMaxWait = JSONProperty(propTask, "maxWait");
    if (propMaxWait == NULL) {
      TheSquidErr->_type = PBErrTypeInvalidData;
      sprintf(TheSquidErr->_msg, "'maxWait' not found");
      JSONFree(&json);
      return false;
    }
    
    // Convert values
    int type = atoi(JSONLabel(JSONValue(propType, 0)));
    unsigned long id = atol(JSONLabel(JSONValue(propId, 0)));
    time_t maxWait = atoi(JSONLabel(JSONValue(propMaxWait, 0)));

    // Switch according to the type of task and add the corresponding 
    // task
    JSONNode* prop = NULL;
    switch(type) {
      
      // Dummy task
      case SquidletTaskType_Dummy:
        
        // Add the task
        SquadAddTask_Dummy(that, id, maxWait);
        break;
      
      // Benchmark task
      case SquidletTaskType_Benchmark:
        
        // Get the extra arguments
        prop = JSONProperty(propTask, "nb");
        if (prop == NULL) {
          TheSquidErr->_type = PBErrTypeInvalidData;
          sprintf(TheSquidErr->_msg, "'nb' not found");
          JSONFree(&json);
          return false;
        }
        int nb = atoi(JSONLabel(JSONValue(prop, 0)));
        prop = JSONProperty(propTask, "payloadSize");
        if (prop == NULL) {
          TheSquidErr->_type = PBErrTypeInvalidData;
          sprintf(TheSquidErr->_msg, "'payloadSize' not found");
          JSONFree(&json);
          return false;
        }
        size_t payloadSize = atol(JSONLabel(JSONValue(prop, 0)));

        // Add the task
        SquadAddTask_Benchmark(that, id, maxWait, nb, payloadSize);
        break;

      // POV-Ray task
      case SquidletTaskType_PovRay:

        // Get the extra arguments
        prop = JSONProperty(propTask, "ini");
        if (prop == NULL) {
          TheSquidErr->_type = PBErrTypeInvalidData;
          sprintf(TheSquidErr->_msg, "ini not found");
          JSONFree(&json);
          return false;
        }
        char* ini = strdup(JSONLabel(JSONValue(prop, 0)));
        prop = JSONProperty(propTask, "sizeMinFragment");
        if (prop == NULL) {
          TheSquidErr->_type = PBErrTypeInvalidData;
          sprintf(TheSquidErr->_msg, "sizeMinFragment not found");
          JSONFree(&json);
          return false;
        }
        int sizeMinFragment = atoi(JSONLabel(JSONValue(prop, 0)));
        prop = JSONProperty(propTask, "sizeMaxFragment");
        if (prop == NULL) {
          TheSquidErr->_type = PBErrTypeInvalidData;
          sprintf(TheSquidErr->_msg, "sizeMaxFragment not found");
          JSONFree(&json);
          return false;
        }
        int sizeMaxFragment = atoi(JSONLabel(JSONValue(prop, 0)));

        // Add the task
        SquadAddTask_PovRay(that, id, maxWait, ini, 
          sizeMinFragment, sizeMaxFragment);

        // Free memory
        free(ini);
        break;

      // Invalid task type
      default:
        // Set the error message
        TheSquidErr->_type = PBErrTypeInvalidData;
        sprintf(TheSquidErr->_msg, "invalid task type (%d)", type);

        // Free memory
        JSONFree(&json);
        
        // Return the error code
        return false;
    }
  }

  // Free the memory used by the JSON
  JSONFree(&json);
  
  // Return the success code
  return true;
  
}

// Load the Squidlet info from the file 'stream' into the Squad 'that'
// Return true if it could load the info, else false
bool SquadLoadSquidlets(
  Squad* const that, 
   FILE* const stream) {
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
    TheSquidErr->_type = PBErrTypeIOError;
    sprintf(TheSquidErr->_msg, "JSONLoad failed");
    JSONFree(&json);
    return false;
  }

  // Decode the data from the JSON
  if (!SquadDecodeAsJSON(that, json)) {
    JSONFree(&json);
    return false;
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
    TheSquidErr->_type = PBErrTypeInvalidData;
    sprintf(TheSquidErr->_msg, "'_squidlets' not found");
    return false;
  }

  // Get the number of squidlets
  int nbSquidlet = JSONGetNbValue(prop);
  
  // Loop on squidlets
  for (int iSquidlet = 0; iSquidlet < nbSquidlet; ++iSquidlet) {
    // Get the JSON node for this squidlet
    JSONNode* propSquidlet = JSONValue(prop, iSquidlet);
    
    // Get the property _name of the squidlet
    JSONNode* propName = JSONProperty(propSquidlet, "_name");
    if (propName == NULL) {
      TheSquidErr->_type = PBErrTypeInvalidData;
      sprintf(TheSquidErr->_msg, "'_name' not found for squidlet %d",
        iSquidlet);
      return false;
    }

    // Get the property _ip of the squidlet
    JSONNode* propIp = JSONProperty(propSquidlet, "_ip");
    if (propIp == NULL) {
      TheSquidErr->_type = PBErrTypeInvalidData;
      sprintf(TheSquidErr->_msg, "'_ip' not found for squidlet %d",
        iSquidlet);
      return false;
    }

    // Get the property _port of the squidlet
    JSONNode* propPort = JSONProperty(propSquidlet, "_port");
    if (propPort == NULL) {
      TheSquidErr->_type = PBErrTypeInvalidData;
      sprintf(TheSquidErr->_msg, "'_port' not found for squidlet %d",
        iSquidlet);
      return false;
    }

    // Create a SquidletInfo
    char* name = JSONLabel(JSONValue(propName, 0));
    char* ip = JSONLabel(JSONValue(propIp, 0));
    int port = atoi(JSONLabel(JSONValue(propPort, 0)));
    SquidletInfo* squidletInfo = SquidletInfoCreate(name, ip, port);
    
    // Add the the squidlet to the set of squidlets
    GSetAppend((GSet*)SquadSquidlets(that), squidletInfo);
  }

  // Return the success code
  return true;
}

// Send the task request 'request' from the Squad 'that' to its
// Squidlet 'squidlet'
// Create a socket, open a connection, ask the squidlet if it can 
// execute the task and wait for its reply
// Return true if the request has been accepted by the squidlet, 
// false else
bool SquadSendTaskRequest(
                      Squad* const that, 
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
  if (request->_type == SquidletTaskType_Null) {

    // Nothing to do
    return true;
  }
  
  // Close the socket if it was opened
  if (squidlet->_sock != -1) {
    close(squidlet->_sock);
    squidlet->_sock = -1;
  }
  
  // Create a socket
  int protocol = 0;
  squidlet->_sock = socket(AF_INET, SOCK_STREAM, protocol);
  
  // If we couldn't create the socket
  if (squidlet->_sock == -1) {

    // Display info in the TextOMeter if necessary
    if (SquadGetFlagTextOMeter(that) == true) {
      char lineHistory[SQUAD_TXTOMETER_LENGTHLINEHISTORY - 10];
      sprintf(lineHistory, "can't create socket to squidlet:");
      SquadPushHistory(that, lineHistory);
      FILE* streamBufferHistory = fmemopen(lineHistory, 
        SQUAD_TXTOMETER_LENGTHLINEHISTORY - 10, "w");
      SquidletInfoPrint(squidlet, streamBufferHistory);
      fclose(streamBufferHistory);
      SquadPushHistory(that, lineHistory);
    }
  
    // Return the failure code
    return false;
  }
  
  // Create the data for the connection to the squidlet from its
  // ip and port
  struct sockaddr_in remote = {0};
  remote.sin_addr.s_addr = inet_addr(squidlet->_ip);
  remote.sin_family = AF_INET;
  remote.sin_port = htons(squidlet->_port);

  // Connect to the squidlet
  int retConnect = connect(squidlet->_sock, (struct sockaddr*)&remote, 
    sizeof(struct sockaddr_in));

  // If the connection failed
  if (retConnect == -1) {

    // Close the socket
    close(squidlet->_sock);
    squidlet->_sock = -1;

    // Display info in the TextOMeter if necessary
    if (SquadGetFlagTextOMeter(that) == true) {

      char lineHistory[SQUAD_TXTOMETER_LENGTHLINEHISTORY - 10];
      sprintf(lineHistory, "can't connect to squidlet:");
      SquadPushHistory(that, lineHistory);
      FILE* streamBufferHistory = fmemopen(lineHistory, 
        SQUAD_TXTOMETER_LENGTHLINEHISTORY - 10, "w");
      SquidletInfoPrint(squidlet, streamBufferHistory);
      fclose(streamBufferHistory);
      SquadPushHistory(that, lineHistory);
    }

    // Return the failure code
    return false;
  }

  // Display info in the TextOMeter if necessary
  if (SquadGetFlagTextOMeter(that) == true) {

    char lineHistory[SQUAD_TXTOMETER_LENGTHLINEHISTORY - 10];
    sprintf(lineHistory, "connected to squidlet:");
    SquadPushHistory(that, lineHistory);
    FILE* streamBufferHistory = fmemopen(lineHistory, 
      SQUAD_TXTOMETER_LENGTHLINEHISTORY - 10, "w");
    SquidletInfoPrint(squidlet, streamBufferHistory);
    fclose(streamBufferHistory);
    SquadPushHistory(that, lineHistory);
  }

  // Set the timeout of the socket for sending and receiving to 1s
  // and allow the reuse of address
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  int retSnd = setsockopt(squidlet->_sock, SOL_SOCKET, SO_SNDTIMEO, 
    (char*)&tv, sizeof(tv));
  int retRcv = setsockopt(squidlet->_sock, SOL_SOCKET, SO_RCVTIMEO, 
    (char*)&tv, sizeof(tv));
  int reuse = 1;
  int retReuse = setsockopt(squidlet->_sock, SOL_SOCKET, SO_REUSEADDR,
    &reuse,sizeof(int));
    
  // If we couldn't configure the socket
  if (retSnd == -1 || retRcv == -1 || retReuse == -1) {

    // Close the socket
    close(squidlet->_sock);
    squidlet->_sock = -1;

    // Display info in the TextOMeter if necessary
    if (SquadGetFlagTextOMeter(that) == true) {

      char lineHistory[SQUAD_TXTOMETER_LENGTHLINEHISTORY - 10];
      sprintf(lineHistory, "failed to configure socket to squidlet:");
      SquadPushHistory(that, lineHistory);
      FILE* streamBufferHistory = fmemopen(lineHistory, 
        SQUAD_TXTOMETER_LENGTHLINEHISTORY - 10, "w");
      SquidletInfoPrint(squidlet, streamBufferHistory);
      fclose(streamBufferHistory);
      SquadPushHistory(that, lineHistory);
    }

    // Return the failure code
    return false;
  }

  // Send the task request
  int flags = 0;
  int retSend = send(squidlet->_sock, 
    request, sizeof(SquidletTaskRequest), flags);
    
  // If we couldn't send the request
  if (retSend == -1) {

    // Close the socket
    close(squidlet->_sock);
    squidlet->_sock = -1;

    // Display info in the TextOMeter if necessary
    if (SquadGetFlagTextOMeter(that) == true) {

      char lineHistory[SQUAD_TXTOMETER_LENGTHLINEHISTORY - 10];
      sprintf(lineHistory, "failed to send the request to squidlet:");
      SquadPushHistory(that, lineHistory);
      FILE* streamBufferHistory = fmemopen(lineHistory, 
        SQUAD_TXTOMETER_LENGTHLINEHISTORY - 10, "w");
      SquidletInfoPrint(squidlet, streamBufferHistory);
      fclose(streamBufferHistory);
      SquadPushHistory(that, lineHistory);
    }

    // Return the failure code
    return false;
  }

  // Wait for the reply from the squidlet up to 5s
  char reply = THESQUID_TASKREFUSED;
  time_t maxWait = 5; 
  bool retRecv = SocketRecv(
    &(squidlet->_sock), sizeof(reply), &reply, maxWait);

  // If we couldn't receive the reply or the reply timed out or
  // the squidlet refused the task
  if (retRecv == false || reply == THESQUID_TASKREFUSED) {

    // Close the socket
    close(squidlet->_sock);
    squidlet->_sock = -1;

    // Display info in the TextOMeter if necessary
    if (SquadGetFlagTextOMeter(that) == true) {

      char lineHistory[SQUAD_TXTOMETER_LENGTHLINEHISTORY - 10];
      sprintf(lineHistory, "task refused by squidlet:");
      SquadPushHistory(that, lineHistory);
      FILE* streamBufferHistory = fmemopen(lineHistory, 
        SQUAD_TXTOMETER_LENGTHLINEHISTORY - 10, "w");
      SquidletInfoPrint(squidlet, streamBufferHistory);
      fclose(streamBufferHistory);
      SquadPushHistory(that, lineHistory);
    }

    // Return the failure code
    return false;
  }

  // If we reach here the task has been accepted
  // Display info in the TextOMeter if necessary
  if (SquadGetFlagTextOMeter(that) == true) {
      char lineHistory[SQUAD_TXTOMETER_LENGTHLINEHISTORY - 10];
      sprintf(lineHistory, "task:");
      SquadPushHistory(that, lineHistory);
      FILE* streamBufferHistory = fmemopen(lineHistory, 
        SQUAD_TXTOMETER_LENGTHLINEHISTORY - 10, "w");
      SquidletTaskRequestPrint(request, streamBufferHistory);
      fclose(streamBufferHistory);
      SquadPushHistory(that, lineHistory);

      sprintf(lineHistory, "accepted by squidlet:");
      SquadPushHistory(that, lineHistory);
      streamBufferHistory = fmemopen(lineHistory, 
        SQUAD_TXTOMETER_LENGTHLINEHISTORY - 10, "w");
      SquidletInfoPrint(squidlet, streamBufferHistory);
      fclose(streamBufferHistory);
      SquadPushHistory(that, lineHistory);
  }

  // Return the success code
  return true;
}

// Add a dummy task uniquely identified by its 'id' to the list of 
// task to execute by the squad 'that'
// The task will have a maximum of 'maxWait' seconds to complete from 
// the time it's accepted by the squidlet or it will be considered
// as failed
void SquadAddTask_Dummy(
         Squad* const that, 
  const unsigned long id,
         const time_t maxWait) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Prepare the data as JSON
  // Hopefully the id will never have more than 90 digits
  char buffer[100];
  memset(buffer, 0, 100);
  sprintf(buffer, "{\"v\":\"%d\"}", (int)id);
  unsigned long subid = 0;

  // Create the new task
  SquidletTaskRequest* task = SquidletTaskRequestCreate(
    SquidletTaskType_Dummy, id, subid, buffer, maxWait);
  
  // Add the new task to the set of task to execute
  GSetAppend((GSet*)SquadTasks(that), task);
}

// Add a benchamrk task uniquely identified by its 'id' to the list of 
// task to execute by the squad 'that'
// The task will have a maximum of 'maxWait' seconds to complete from 
// the time it's accepted by the squidlet or it will be considered
// as failed
// Artificially set the size of the data for the task to 'payloadSize' 
// bytes
// The benchmark function used is sorting 'nb' times a set of numbers
// cf TheSquidBenchmark()
void SquadAddTask_Benchmark(
         Squad* const that, 
  const unsigned long id,
         const time_t maxWait, 
   const unsigned int nb, 
   const unsigned int payloadSize) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Create a dummy buffer made of the alphabet character
  // The length is tricked by the requested payloadSize
  char* data = PBErrMalloc(TheSquidErr, payloadSize + 1);
  memset(data, ' ', payloadSize);
  data[payloadSize] = '\0';

  // Convert the arguments into strings to get their length
  char bufferNb[100];
  sprintf(bufferNb, "%d", nb);
  int bufferNbLen = strlen(bufferNb);
  char bufferId[100];
  sprintf(bufferId, "%ld", id);
  int bufferIdLen = strlen(bufferId);

  // Get the length of the json data without values
  int jsonFormatLen = strlen("{\"id\":\"\",\"nb\":\"\",\"v\":\"\"}");

  // Get the length of the json data with values
  int bufferLength = 
    jsonFormatLen + bufferIdLen + bufferNbLen + strlen(data) + 1;

  // Allocate memory
  char* buffer = PBErrMalloc(TheSquidErr, bufferLength);

  // Create the JSON data
  sprintf(buffer, "{\"id\":\"%s\",\"nb\":\"%s\",\"v\":\"%s\"}", 
    bufferId, bufferNb, data);
  free(data);

  // Create the new task
  unsigned long subid = 0;
  SquidletTaskRequest* task = SquidletTaskRequestCreate(
    SquidletTaskType_Benchmark, id, subid, buffer, maxWait);
  free(buffer);
  
  // Add the new task to the set of task to execute
  GSetAppend((GSet*)SquadTasks(that), task);
}

// Add a POV-Ray task uniquely identified by its 'id' to the list of 
// task to execute by the squad 'that'
// The task will have a maximum of 'maxWait' seconds to complete from 
// the time it's accepted by the squidlet or it will be considered
// as failed
// The POV-Ray task is described by the POV-Ray ini file 'ini' which 
// must include at least the following options:
// Input_File_Name=<script.pov>
// Width=<width> 
// Height=<height>
// Output_File_Name=<image.tga>
// The output format of the image must be TGA
// Video are not supported
// The image is splitted into as many squared fragments as 
// SquadGetNbSquidlets(that)^2, but the size of the fragment is clipped
// to [sizeMinFragment, sizeMaxFragment]
// The size of the fragments are corrected to fit the size of the image
// if it's not a squared image
// Fragments are rendered on the squidlets in random order
// The final image is updated each time a fragment has been
// rendered
// The random generator must have been initialised before calling this 
// function
void SquadAddTask_PovRay(
         Squad* const that, 
  const unsigned long id,
         const time_t maxWait, 
    const char* const ini, 
   const unsigned int sizeMinFragment,
   const unsigned int sizeMaxFragment) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
  if (ini == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'ini' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Init variables to memorize the dimensions and path of the final 
  // output image
  unsigned long width = 0;
  unsigned long height = 0;
  char* outImgPath = NULL;

  // Open the ini file
  FILE* fp = fopen(ini, "r");
  
  // If we couldn't open the ini file
  if (fp == NULL) {
    
    // Report the error
    TheSquidErr->_type = PBErrTypeInvalidArg;
    sprintf(TheSquidErr->_msg, "Can't open %s", ini);
    PBErrCatch(TheSquidErr);

  // Else, we could open the ini file
  } else {

    // Read the ini file line by line
    char oneLine[1024];
    while(fgets(oneLine, 1024, fp)) {
      
      // If we are on the line defining the width
      if (strstr(oneLine, "Width=")) {
        
        // Decode the width
        sscanf(oneLine + 6, "%lu", &width);

      // If we are on the line defining the height
      } else if (strstr(oneLine, "Height=")) {
        
        // Decode the height
        sscanf(oneLine + 7, "%lu", &height);

      // If we are on the line defining the path to the output file
      // and there is actually a value for the path
      } else if (strstr(oneLine, "Output_File_Name=") &&
        strlen(oneLine) > 17) {
        
        // Copy the path
        outImgPath = strdup(oneLine + 17);

        // Remove the return line
        outImgPath[strlen(outImgPath) - 1] = '\0';

        // Make sure the output file doesn't exists
        char* cmd = PBErrMalloc(TheSquidErr, sizeof(char) * 
          (strlen("rm -f ") + strlen(outImgPath) + 1));
        sprintf(cmd, "rm -f %s", outImgPath);
        int ret = system(cmd);
        free(cmd);
        (void)ret;
      }
    }
    fclose(fp);
  }
  
  // Check arguments
  if (width == 0 || height == 0 || outImgPath == NULL) {
    TheSquidErr->_type = PBErrTypeInvalidArg;
    sprintf(TheSquidErr->_msg, "Can't decode arguments from %s", ini);
    PBErrCatch(TheSquidErr);
  }

  // Get the size of one fragment
  unsigned long nbSquidlets = SquadGetNbSquidlets(that);
  if (nbSquidlets == 0)
    nbSquidlets = 1;
  unsigned long sizeFrag[2];
  sizeFrag[0] = 
    MAX(sizeMinFragment, MIN(sizeMaxFragment, width / nbSquidlets));
  sizeFrag[1] = 
    MAX(sizeMinFragment, MIN(sizeMaxFragment, width / nbSquidlets));
  
  // Get the nb of fragments
  unsigned long nbFrag[2];
  nbFrag[0] = width / sizeFrag[0];
  nbFrag[1] = height / sizeFrag[1];
  if (sizeFrag[0] * nbFrag[0] < width)
    ++(nbFrag[0]);
  if (sizeFrag[1] * nbFrag[1] < height)
    ++(nbFrag[1]);
  
  // Create a temporary GSet where to add the tasks to be able to 
  // shuffle it independantly of the eventual other task in the Squad
  GSet set = GSetCreateStatic();
  
  // Create the tasks for each fragment
  for (unsigned long i = 0; i < nbFrag[0]; ++i) {
    for (unsigned long j = 0; j < nbFrag[1]; ++j) {
      // Get the id of the task
      unsigned long taskId = i * nbFrag[1] + j;
      // Get the coordinates of the fragment
      // Pov-Ray starts counting at 1, so the top left is (1,1)
      unsigned long top = j * sizeFrag[1] + 1;
      unsigned long left = i * sizeFrag[0] + 1;
      unsigned long bottom = (j + 1) * sizeFrag[1] + 1;
      if (bottom > height)
        bottom = height;
      unsigned long right = (i + 1) * sizeFrag[0] + 1;
      if (right > width)
        right = width;
      // Get the name of the output file for this fragment
      int len = strlen(outImgPath);
      char* tga = PBErrMalloc(TheSquidErr, len + 6);
      memset(tga, 0, len + 6);
      strcpy(tga, outImgPath);
      sprintf(tga + len - 4, "-%05lu.tga", taskId);
      // Prepare the data as JSON
      char buffer[1024];
      memset(buffer, 0, 1024);
      sprintf(buffer, 
        "{\"id\":\"%lu\",\"subid\":\"%lu\",\"ini\":\"%s\","
        "\"tga\":\"%s\",\"top\":\"%lu\",\"left\":\"%lu\","
        "\"bottom\":\"%lu\",\"right\":\"%lu\",\"width\":\"%lu\","
        "\"height\":\"%lu\",\"outTga\":\"%s\"}", 
        id, taskId, ini, tga, top, left, bottom, right, width, height,
        outImgPath);
      // Add the new task to the set of task to execute
      SquidletTaskRequest* task = SquidletTaskRequestCreate(
        SquidletTaskType_PovRay, id, taskId, buffer, maxWait);
      GSetAppend(&set, task);
      // Free memory
      free(tga);
    }
  }
  GSetShuffle(&set);
  GSetAppendSet((GSet*)SquadTasks(that), &set);
  GSetFlush(&set);
  
  // Free memory
  free(outImgPath);

}

// Send the data associated to the task request 'task' from the Squad 
// 'that' to the Squidlet 'squidlet'
// First, send the size in byte of the data, then send the data
// Return true if the data could be sent, false else
// The size of the data must be less than 1024 bytes
bool SquadSendTaskData(
                Squad* const that, 
         SquidletInfo* const squidlet, 
  SquidletTaskRequest* const task) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Declare some variables to process the lines of history
  char lineHistory[200];
  char bufferHistory[100];
  FILE* streamBufferHistory = NULL;

  // Send the task data size
  int flags = 0;
  size_t len = strlen(task->_data);
  if (send(squidlet->_sock, 
    (char*)&len, sizeof(size_t), flags) == -1) {
    // If we couldn't send the data size

    if (SquadGetFlagTextOMeter(that) == true) {
      sprintf(lineHistory, 
        "couldn't send task data size %d", len);
      SquadPushHistory(that, lineHistory);
    }

    return false;
  } else {

    if (SquadGetFlagTextOMeter(that) == true) {
      streamBufferHistory = fmemopen(bufferHistory, 100, "w");
      SquidletInfoPrint(squidlet, streamBufferHistory);
      fclose(streamBufferHistory);
      sprintf(lineHistory, 
        "sent task data size %d to (%s)", len, 
        bufferHistory);
      SquadPushHistory(that, lineHistory);
    }
  }

  // Send the task data
  if (send(squidlet->_sock, task->_data, len, flags) == -1) {
    // If we couldn't send the data

    if (SquadGetFlagTextOMeter(that) == true)
      SquadPushHistory(that, "couldn't send task data");

    return false;
  } else {
    if (SquadGetFlagTextOMeter(that) == true)
      SquadPushHistory(that, "sent task data");
  }

  // Return the success code
  return true;
}

// Try to receive the result from the running task 'runningTask'
// If the result is ready it is stored in the _bufferResult of the 
// SquidletTaskRequest of the 'runningTask'
// If the size of the result data is not ready and couldn't be received 
// give up immediately
// If the size of the result data has been received, wait for 
// (5 + sizeData / 100) seconds maximum to receive the data
// First receive the size of the result data, then send an 
// acknowledgement signal to the squidlet for the size of data,
// then receive the data, and finally send an acknowledgement to the 
// squidlet for the data
// Return true if it could receive the result data, false else
bool SquadReceiveTaskResult(
             Squad* const that, 
  SquadRunningTask* const runningTask) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
  if (runningTask == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'squidlet' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Declare a variable to memorize if we have received the result
  bool receivedFlag = false;

  // Declare a variable to memorize the size in byte of the result data
  size_t sizeResultData = 0;

  // Helper variables
  SquidletInfo* squidlet = runningTask->_squidlet;
  SquidletTaskRequest* task = runningTask->_request;

  // Declare some variables to process the lines of history
  char lineHistory[200];
  char bufferHistory[100];
  FILE* streamBufferHistory = NULL;
  
  // Make sure the buffer to receive the task is empty
  if (task->_bufferResult != NULL) {
    free(task->_bufferResult);
    task->_bufferResult = NULL;
  }
  
  // Try to receive the size of the reply from the squidlet
  // and give up immediately
  if (SocketRecv(&(squidlet->_sock), sizeof(size_t), 
    (char*)&sizeResultData, 0)) {

    // If the result is ready
    if (sizeResultData > 0) {

      if (SquadGetFlagTextOMeter(that) == true) {
        streamBufferHistory = fmemopen(bufferHistory, 100, "w");
        SquidletInfoPrint(squidlet, streamBufferHistory);
        fclose(streamBufferHistory);
        sprintf(lineHistory, 
          "received the size of result from (%s)", 
          bufferHistory);
        SquadPushHistory(that, lineHistory);
      }

      // Send the acknowledgement of received size of result
      char ack = 1;
      int flags = 0;
      (void)send(squidlet->_sock, &ack, sizeof(char), flags);

      if (SquadGetFlagTextOMeter(that) == true) {
        streamBufferHistory = fmemopen(bufferHistory, 100, "w");
        SquidletInfoPrint(squidlet, streamBufferHistory);
        fclose(streamBufferHistory);
        sprintf(lineHistory, 
          "send ack of received size of result data to (%s)", 
          bufferHistory);
        SquadPushHistory(that, lineHistory);
      }

      // Allocate memory for the result data
      task->_bufferResult = PBErrMalloc(TheSquidErr, sizeResultData + 1);
      memset(task->_bufferResult, 0, sizeResultData + 1);
      
      // Wait to receive the result data with a time limit proportional
      // to the size of result data
      int timeOut = 5 + (int)round((float)sizeResultData / 100.0);
      if (!SocketRecv(&(squidlet->_sock), sizeResultData, 
        task->_bufferResult, timeOut)) {
        // If we coudln't received the result data
        free(task->_bufferResult);
        task->_bufferResult = NULL;

        if (SquadGetFlagTextOMeter(that) == true) {
          streamBufferHistory = fmemopen(bufferHistory, 100, "w");
          SquidletInfoPrint(squidlet, streamBufferHistory);
          fclose(streamBufferHistory);
          sprintf(lineHistory, 
            "couldn't received result data from (%s)", 
            bufferHistory);
          SquadPushHistory(that, lineHistory);
          sprintf(lineHistory, "waited for %ds", 
            timeOut);
          SquadPushHistory(that, lineHistory);
        }

      } else {
        receivedFlag = true;
        
        // Send the acknowledgement of received result
        (void)send(squidlet->_sock, &ack, 1, flags);
        
        if (SquadGetFlagTextOMeter(that) == true) {
          streamBufferHistory = fmemopen(bufferHistory, 100, "w");
          SquidletInfoPrint(squidlet, streamBufferHistory);
          fclose(streamBufferHistory);
          sprintf(lineHistory, 
            "received result data from (%s)", 
            bufferHistory);
          SquadPushHistory(that, lineHistory);
          sprintf(lineHistory, "size result data %d", 
            sizeResultData);
          SquadPushHistory(that, lineHistory);
        }

      }
    } else {

      if (SquadGetFlagTextOMeter(that) == true) {
        streamBufferHistory = fmemopen(bufferHistory, 100, "w");
        SquidletInfoPrint(squidlet, streamBufferHistory);
        fclose(streamBufferHistory);
        sprintf(lineHistory, 
          "received a null size of result from (%s)", 
          bufferHistory);
        SquadPushHistory(that, lineHistory);
      }

    }
  }
  
  // Return the result
  return receivedFlag;
}

// Request the execution of a task on a squidlet for the squad 'that'
// Return true if the request was successfull, fals else
bool SquadSendTaskOnSquidlet(Squad* const that, 
  SquidletInfo* const squidlet, SquidletTaskRequest* const task) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
  if (squidlet == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'squidlet' is null");
    PBErrCatch(TheSquidErr);
  }
  if (task == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'task' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Declare some variables to process the lines of history
  char lineHistory[200];
  char bufferHistory[100];
  FILE* streamBufferHistory = NULL;
  // Request the execution of the task by the squidlet
  bool ret = SquadSendTaskRequest(that, task, squidlet);
  if (ret) {
    // If the squidlet accepted to execute the task
    if (SquadSendTaskData(that, squidlet, task)) {
      // Create a new running task and add it to the set
      SquadRunningTask* runningTask = 
        SquadRunningTaskCreate(task, squidlet);

      if (SquadGetFlagTextOMeter(that) == true) {
        streamBufferHistory = fmemopen(bufferHistory, 100, "w");
        SquadRunningTaskPrint(runningTask, streamBufferHistory);
        fclose(streamBufferHistory);
        sprintf(lineHistory, "task %s running", 
          bufferHistory);
        SquadPushHistory(that, lineHistory);
      }

      GSetAppend((GSet*)SquadRunningTasks(that), runningTask);
    } else {

      if (SquadGetFlagTextOMeter(that) == true) {
        streamBufferHistory = fmemopen(bufferHistory, 100, "w");
        SquidletInfoPrint(squidlet, streamBufferHistory);
        fclose(streamBufferHistory);
        sprintf(lineHistory, "couldn't send data to %s", 
          bufferHistory);
        SquadPushHistory(that, lineHistory);
      }
      ret = false;
    }
  } else {

    if (SquadGetFlagTextOMeter(that) == true) {
      streamBufferHistory = fmemopen(bufferHistory, 100, "w");
      SquidletInfoPrint(squidlet, streamBufferHistory);
      fclose(streamBufferHistory);
      sprintf(lineHistory, "task refused by %s", 
        bufferHistory);
      SquadPushHistory(that, lineHistory);
    }

  }
  // Return the result
  return ret;
}

// Step the Squad 'that', i.e. tries to affect the remaining tasks to 
// available Squidlet and check for completion of running task.
// Return a GSet of completed SquidletTaskRequest at this step
// Non blocking, if there is no task to compute or no squidlet 
// available, and no task completed, do nothing and return an empty set
GSet SquadStep(
  Squad* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Create the set of completed tasks
  GSet completedTasks = GSetCreateStatic();

  // Declare some variables to process the lines of history
  char lineHistory[200];
  char bufferHistory[100];
  FILE* streamBufferHistory = NULL;
  
  // If there are running tasks
  if (SquadGetNbRunningTasks(that) > 0L) {
    // Loop on running tasks
    GSetIterForward iter = 
      GSetIterForwardCreateStatic((GSet*)SquadRunningTasks(that));
    bool flag = false;
    do {
      flag = false;
      // Get the running tasks
      SquadRunningTask* runningTask = GSetIterGet(&iter);
      // Check if the task is complete
      if (SquadReceiveTaskResult(that, runningTask)) {

        if (SquadGetFlagTextOMeter(that) == true) {
          streamBufferHistory = fmemopen(bufferHistory, 100, "w");
          SquadRunningTaskPrint(runningTask, streamBufferHistory);
          fclose(streamBufferHistory);
          sprintf(lineHistory, "task %s complete", 
            bufferHistory);
          SquadPushHistory(that, lineHistory);
        }

        // Put back the squidlet in the set of squidlets
        GSetAppend((GSet*)SquadSquidlets(that), runningTask->_squidlet);
        // Add the task to the set of completed tasks
        GSetAppend(&completedTasks, runningTask->_request);
        // Post process the completed task
        SquadProcessCompletedTask(that, runningTask->_request);
        // Free memory
        runningTask->_request = NULL;
        runningTask->_squidlet = NULL;
        SquadRunningTaskFree(&runningTask);
        // Remove the task from the running tasks
        flag = GSetIterRemoveElem(&iter);
      // Else if we've been waiting too long for this task to complete
      } else if (time(NULL) - runningTask->_startTime > 
        runningTask->_request->_maxWaitTime) {

        if (SquadGetFlagTextOMeter(that) == true) {
          streamBufferHistory = fmemopen(bufferHistory, 100, "w");
          SquadRunningTaskPrint(runningTask, streamBufferHistory);
          fclose(streamBufferHistory);
          sprintf(lineHistory, "task %s gave up", 
            bufferHistory);
          SquadPushHistory(that, lineHistory);
        }

        // Put back the squidlet in the set of squidlets
        GSetAppend((GSet*)SquadSquidlets(that), runningTask->_squidlet);
        // Put back the task to the set of tasks
        SquadTryAgainTask(that, runningTask->_request);
        // Free memory
        runningTask->_request = NULL;
        runningTask->_squidlet = NULL;
        SquadRunningTaskFree(&runningTask);
        // Remove the task from the running tasks
        flag = GSetIterRemoveElem(&iter);
      } else {

        if (SquadGetFlagTextOMeter(that) == true) {
          streamBufferHistory = fmemopen(bufferHistory, 100, "w");
          SquadRunningTaskPrint(runningTask, streamBufferHistory);
          fclose(streamBufferHistory);
          sprintf(lineHistory, "task %s still running", 
            bufferHistory);
          SquadPushHistory(that, lineHistory);
        }

      }
    } while (flag || GSetIterStep(&iter));
  }
  
  // If there are tasks to execute and available squidlet
  if (SquadGetNbRemainingTasks(that) > 0L && SquadGetNbSquidlets(that) > 0L) {
    // Loop on squidlets
    GSetIterForward iter = 
      GSetIterForwardCreateStatic((GSet*)SquadSquidlets(that));
    bool flag = false;
    do {
      flag = false;
      // Get the squidlet
      SquidletInfo* squidlet = GSetIterGet(&iter);
      // Get the next task
      SquidletTaskRequest* task = GSetPop((GSet*)SquadTasks(that));
      // If there is a task
      if (task != NULL) {
        // Request the task on the squidlet
        bool ret = SquadSendTaskOnSquidlet(that, squidlet, task);
        // If the squidlet refused the task or the data couldn't be sent
        if (!ret) {
          // Put back the task in the set
          GSetPush((GSet*)SquadTasks(that), task);
        } else {
          // Remove the squidlet from the available squidlet
          flag = GSetIterRemoveElem(&iter);
        }
      }
    } while (flag || GSetIterStep(&iter));
  }
  
  // Update the TextOMeter if necessary
  if (SquadGetFlagTextOMeter(that) == true) {
    SquadUpdateTextOMeter(that);
  }
   
  // Return the set of completed tasks
  return completedTasks;
}

// Process the completed 'task' with the Squad 'that' after its 
// reception in SquadStep()
void SquadProcessCompletedTask(
                Squad* const that, 
  SquidletTaskRequest* const task) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
  if (task == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'task' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Call the appropriate function based on the type of the task
  switch (task->_type) {
    case SquidletTaskType_Dummy:
      break;
    case SquidletTaskType_Benchmark:
      break;
    case SquidletTaskType_PovRay:
      SquadProcessCompletedTask_PovRay(that, task);
      break;
    default:
      break;
  }
}

// Process the completed Pov-Ray 'task' with the Squad 'that'
void SquadProcessCompletedTask_PovRay(
                Squad* const that, 
  SquidletTaskRequest* const task) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
  if (task == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'task' is null");
    PBErrCatch(TheSquidErr);
  }
#endif

  // Decode the JSON data from the request and the completed task
  JSONNode* jsonRequest = JSONCreate();
  JSONNode* jsonResult = JSONCreate();

  // If we could decode the JSON
  if (JSONLoadFromStr(jsonResult, task->_bufferResult) && 
    JSONLoadFromStr(jsonRequest, task->_data)) {

    // Get the necessary properties
    JSONNode* propResultImg = JSONProperty(jsonRequest, "outTga");
    JSONNode* propWidth = JSONProperty(jsonRequest, "width");
    JSONNode* propHeight = JSONProperty(jsonRequest, "height");
    JSONNode* propTga = JSONProperty(jsonResult, "tga");
    JSONNode* propTop = JSONProperty(jsonResult, "top");
    JSONNode* propLeft = JSONProperty(jsonResult, "left");
    JSONNode* propRight = JSONProperty(jsonResult, "right");
    JSONNode* propBottom = JSONProperty(jsonResult, "bottom");

    // If the necessary properties were present
    if (propTga != NULL && propTop != NULL && propLeft != NULL &&
      propRight != NULL && propBottom != NULL && propResultImg != NULL) {
      // Load the result image
      GenBrush* resultImg = 
        GBCreateFromFile(JSONLabel(JSONValue(propResultImg, 0)));

      // If the result image doesn't exists
      if (resultImg == NULL) {
        // Create the result image
        VecShort2D dim = VecShortCreateStatic2D();
        VecSet(&dim, 0, atoi(JSONLabel(JSONValue(propWidth, 0))));
        VecSet(&dim, 1, atoi(JSONLabel(JSONValue(propHeight, 0))));
        resultImg = GBCreateImage(&dim);
        GBSetFileName(resultImg, JSONLabel(JSONValue(propResultImg, 0)));
      }

      // Load the fragment
      GenBrush* fragment = 
        GBCreateFromFile(JSONLabel(JSONValue(propTga, 0)));

      // If we could load the fragment
      if (fragment != NULL) {
        // Crop the relevant portion of the image
        // Pov-Ray has its coordinate system origin at the top left of 
        // the image, while GenBrush has its own at the bottom left
        // Pov-Ray starts counting at 1, so the top left is (1,1)
        VecShort2D dim = VecShortCreateStatic2D();
        VecSet(&dim, 0,
          atoi(JSONLabel(JSONValue(propRight, 0))) -
          atoi(JSONLabel(JSONValue(propLeft, 0))) + 1);
        VecSet(&dim, 1, 
          atoi(JSONLabel(JSONValue(propBottom, 0))) -
          atoi(JSONLabel(JSONValue(propTop, 0))) + 1);
        VecShort2D posLR = VecShortCreateStatic2D();
        VecSet(&posLR, 0, atoi(JSONLabel(JSONValue(propLeft, 0))) - 1);
        VecSet(&posLR, 1, 
          atoi(JSONLabel(JSONValue(propHeight, 0))) -
          atoi(JSONLabel(JSONValue(propBottom, 0))));
        // Add the fragment to the result image
        VecShort2D pos = VecShortCreateStatic2D();
        do {
          VecShort2D posFinal = VecGetOp(&pos, 1, &posLR, 1);
          GBPixel pix = GBGetFinalPixel(fragment, &posFinal);
          GBSetFinalPixel(resultImg, &posFinal, &pix);
        } while (VecStep(&pos, &dim));
        
        // Save the result image
        GBRender(resultImg);

        // Free memory
        GBFree(&fragment);
      } else {
        if (SquadGetFlagTextOMeter(that) == true) {
          char lineHistory[200];
          sprintf(lineHistory, "Couldn't read the fragment (%s)", 
            GenBrushErr->_msg);
          SquadPushHistory(that, lineHistory);
        }
      }

      // Free memory
      GBFree(&resultImg);

      // Delete the fragment
      char cmd[500];
      sprintf(cmd, "rm %s", JSONLabel(JSONValue(propTga, 0)));
      int ret = system(cmd);
      (void)ret;

    } else {
      if (SquadGetFlagTextOMeter(that) == true) {
        char lineHistory[200];
        sprintf(lineHistory, 
          "Can't preprocess the Pov-Ray task (invalid data)");
        SquadPushHistory(that, lineHistory);
      }
    }

    // Free memory
    if (jsonResult != NULL)
      JSONFree(&jsonResult);
    if (jsonRequest != NULL)
      JSONFree(&jsonRequest);
  }
}

// Set the flag memorizing if the TextOMeter is displayed for
// the Squad 'that' to 'flag'
void SquadSetFlagTextOMeter(
  Squad* const that, 
  const bool flag) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // If the requested flag is different from the current flag;
  if (that->_flagTextOMeter != flag) {
    if (flag && that->_textOMeter == NULL) {
      char title[] = "Squad";
      int width = SQUAD_TXTOMETER_LENGTHLINEHISTORY + 1;
      int height = SQUAD_TXTOMETER_NBLINEHISTORY + 
        SQUAD_TXTOMETER_NBTASKDISPLAYED + 4;
      that->_textOMeter = TextOMeterCreate(title, width, height);
    }
    if (!flag && that->_textOMeter != NULL) {
      TextOMeterFree(&(that->_textOMeter));
    }
    that->_flagTextOMeter = flag;
  }
}

// Add one line to the history of messages for the TextOMeter
// 'msg' must be less than 
// SQUAD_TXTOMETER_LENGTHLINEHISTORY - 10 characters long
void SquadPushHistory(Squad* const that, char* msg) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
  if (msg == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'msg' is null");
    PBErrCatch(TheSquidErr);
  }
  if (strlen(msg) >= SQUAD_TXTOMETER_LENGTHLINEHISTORY - 10) {
    TheSquidErr->_type = PBErrTypeInvalidArg;
    sprintf(TheSquidErr->_msg, "'msg' is too long (%d<100)",
      strlen(msg));
    PBErrCatch(TheSquidErr);
  }
#endif
  for (int iLine = 0; iLine < SQUAD_TXTOMETER_NBLINEHISTORY - 1; 
    ++iLine) {
    strcpy(that->_history[iLine], that->_history[iLine + 1]);
  }
  ++(that->_countLineHistory);
  sprintf(that->_history[SQUAD_TXTOMETER_NBLINEHISTORY - 1], 
    "[%06u] %s\n", that->_countLineHistory, msg);
  SquadUpdateTextOMeter(that);
}

// Refresh the content of the TextOMeter attached to the 
// Squad 'that'
void SquadUpdateTextOMeter(const Squad* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
  if (that->_textOMeter == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that->_textOMeter' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Clear the TextOMeter
  TextOMeterClear(that->_textOMeter);
  // .........................
  char buffer[200];
  sprintf(buffer, SQUAD_TXTOMETER_FORMAT1, 
    SquadGetNbRunningTasks(that), SquadGetNbRemainingTasks(that),
    SquadGetNbSquidlets(that));
  TextOMeterPrint(that->_textOMeter, buffer);
  for (int iLine = 0; iLine < SQUAD_TXTOMETER_NBLINEHISTORY; ++iLine) {
    TextOMeterPrint(that->_textOMeter, that->_history[iLine]);
  }
  sprintf(buffer, SQUAD_TXTOMETER_TASKHEADER);
  TextOMeterPrint(that->_textOMeter, buffer);
  int iLine = 0;
  if (SquadGetNbRunningTasks(that) > 0) {
    GSetIterForward iter = GSetIterForwardCreateStatic(
      (GSet*)SquadRunningTasks(that));
    do {
      SquadRunningTask* task = GSetIterGet(&iter);
      if (task != NULL) {
        char bufferTask[100];
        FILE* stream = fmemopen(bufferTask, 100, "w");
        SquadRunningTaskPrint(task, stream);
        fclose(stream);
        sprintf(buffer, SQUAD_TXTOMETER_FORMATRUNNING, bufferTask);
      } else {
        buffer[0] = '\0';
      }
      TextOMeterPrint(that->_textOMeter, buffer);
      ++iLine;
    } while (GSetIterStep(&iter) && 
      iLine < SQUAD_TXTOMETER_NBTASKDISPLAYED);
  }
  if (SquadGetNbRemainingTasks(that) > 0 &&
    iLine < SQUAD_TXTOMETER_NBTASKDISPLAYED) {
    GSetIterForward iter = GSetIterForwardCreateStatic(
      (GSet*)SquadTasks(that));
    do {
      SquidletTaskRequest* task = GSetIterGet(&iter);
      if (task != NULL) {
        char bufferTask[100];
        FILE* stream = fmemopen(bufferTask, 100, "w");
        SquidletTaskRequestPrint(task, stream);
        fclose(stream);
        sprintf(buffer, SQUAD_TXTOMETER_FORMATQUEUED, bufferTask);
      } else {
        buffer[0] = '\0';
      }
      TextOMeterPrint(that->_textOMeter, buffer);
      ++iLine;
    } while (GSetIterStep(&iter) && 
      iLine < SQUAD_TXTOMETER_NBTASKDISPLAYED - 1);
  }
  if (iLine == SQUAD_TXTOMETER_NBTASKDISPLAYED - 1) {
    sprintf(buffer, "...\n");
    TextOMeterPrint(that->_textOMeter, buffer);
  } else {
    sprintf(buffer, "\n");
    for (; iLine < SQUAD_TXTOMETER_NBTASKDISPLAYED; ++iLine) {
      TextOMeterPrint(that->_textOMeter, buffer);
    }
  }
  sprintf(buffer, "\n");
  TextOMeterPrint(that->_textOMeter, buffer);
  // Flush the content of the TextOMeter
  TextOMeterFlush(that->_textOMeter);
}

// Check all the squidlets of the Squad 'that' by processing a dummy 
// task and display information about each one on the file 'stream'
// Return true if all the tasks could be performed, false else
bool SquadCheckSquidlets(
  Squad* const that, 
   FILE* const stream) {
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
  // Declare a variable to memorize the result
  bool res = true;
  // Declare variables to create a dummy task request
  char* buffer = "{\"v\":\"0\"}";
  time_t maxWait = 5;
  // Loop on the squidlets
  if (SquadGetNbSquidlets(that) > 0) {
    GSetIterForward iter = 
      GSetIterForwardCreateStatic(SquadSquidlets(that));
    do {
      // Get the squidlet
      SquidletInfo* squidlet = GSetIterGet(&iter);
      // Display info about the squidlet
      SquidletInfoPrint(squidlet, stream);
      fprintf(stream, "\n");
      // Request a dummy task from the squidlet
      SquidletTaskRequest* task = SquidletTaskRequestCreate(
        SquidletTaskType_Dummy, 0, 0, buffer, maxWait);
      GSetAppend((GSet*)SquadTasks(that), task);
      struct timeval start;
      gettimeofday(&start, NULL);
      bool ret = SquadSendTaskOnSquidlet(that, squidlet, task);
      struct timeval timeToSend;
      gettimeofday(&timeToSend, NULL);
      if (!ret) {
        res = false;
        fprintf(stream, "\tThe request for a dummy task failed.\n");
      } else {
        // Get the running tasks
        SquadRunningTask* runningTask = 
          GSetPop((GSet*)SquadRunningTasks(that));
        // Loop until the task ends
        bool flagStop = false;
        while (!flagStop && time(NULL) - runningTask->_startTime <= 
          runningTask->_request->_maxWaitTime) {
          // If the task is completed
          if (SquadReceiveTaskResult(that, runningTask)) {
            struct timeval timeToProcess;
            gettimeofday(&timeToProcess, NULL);
            // Stop the loop
            flagStop = true;
            // Process the result
            SquidletTaskRequest* request = runningTask->_request;
            fprintf(stream, "\tRequest for dummy task succeeded.\n");
            fprintf(stream, "\t%s\n", request->_bufferResult);
            unsigned long delayToSendms = 
              (timeToSend.tv_sec - start.tv_sec) * 1000 + 
              (timeToSend.tv_usec - start.tv_usec) / 1000;
            unsigned long delayToProcessms = 
              (timeToProcess.tv_sec - timeToSend.tv_sec) * 1000 + 
              (timeToProcess.tv_usec - timeToSend.tv_usec) / 1000;
            fprintf(stream, 
              "\tdelay to send: %lums, delay to process: %lums\n", 
              delayToSendms, delayToProcessms);
          }
        }
        if (!flagStop) {
          fprintf(stream, "\tGave up due to time limit.\n");
        }
        // Free memory
        runningTask->_request = NULL;
        runningTask->_squidlet = NULL;
        SquadRunningTaskFree(&runningTask);
      }
    } while (GSetIterStep(&iter));
  }
  // Return the result
  return res;
}

// Run the benchmark with the squad 'that' and output the result on 
// the file 'stream'
// The benchmark consists of executing benchmark tasks with several
// size and number of loop, first locally and then on the squidlet 
// (which must have been loaded before calling this function)
void SquadBenchmark(
  Squad* const that, 
   FILE* const stream) {
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
  fprintf(stream, "-- Benchmark started --\n");
  int lengthTest = 30;
  size_t maxSizePayload = 1000;
  int nbMaxLoop = 1024;
  // If the squad has no squidlet
  if (SquadGetNbSquidlets(that) == 0) {
    // Run the benchmark locally
    fprintf(stream, "Execution on local device:\n");
    fprintf(stream, 
      "nbLoopPerTask\tnbBytePayload\tnbTaskComp\ttimeMsPerTask\n");
    for (size_t sizePayload = 10; 
      sizePayload <= maxSizePayload; sizePayload *= 10) {
      char* buffer = PBErrMalloc(TheSquidErr, sizePayload + 1);
      memset(buffer, ' ', sizePayload);
      buffer[sizePayload] = '\0';

      // Loop on nbLoop
      for (int nbLoop = 1; nbLoop <= nbMaxLoop; nbLoop *= 2) {
        struct timeval stop, start;
        gettimeofday(&start, NULL);
        unsigned long nbComplete = 0;
        do {
          TheSquidBenchmark(nbLoop, buffer);
          ++nbComplete;
          gettimeofday(&stop, NULL);
        } while (stop.tv_sec - start.tv_sec < lengthTest);
        unsigned long deltams = (stop.tv_sec - start.tv_sec) * 1000 + 
          (stop.tv_usec - start.tv_usec) / 1000;
        float timePerTaskMs = (float) deltams / (float)nbComplete;
        fprintf(stream, "%04d\t%08u\t%07lu\t%011.2f\n", 
          nbLoop, sizePayload, nbComplete, timePerTaskMs);
        fflush(stdout);
      }
      free(buffer);
    }
  // Else the squad has at least one squidlet
  } else {
    // Run the benchmark on the squidlets
    fprintf(stream, "Execution on TheSquid:\n");
    fprintf(stream, 
      "nbLoopPerTask\tnbBytePayload\tnbTaskComp\ttimeMsPerTask\n");

    // Loop on payload size
    time_t maxWait = 1000;
    unsigned int id = 0;
    bool flagStop = false;
    for (size_t sizePayload = 10; !flagStop && 
      sizePayload <= maxSizePayload; sizePayload *= 10) {
      // Loop on nbLoop
      for (int nbLoop = 1; !flagStop && nbLoop <= nbMaxLoop; 
        nbLoop *= 2) {

        // Loop during lengthTest seconds
        struct timeval stop, start;
        gettimeofday(&start, NULL);
        unsigned long nbComplete = 0;
        do {
          
          // Create benchmark tasks if there are no more
          while (SquadGetNbRunningTasks(that) + 
            SquadGetNbRemainingTasks(that) < SquadGetNbSquidlets(that)) {
            SquadAddTask_Benchmark(that, id++, maxWait, nbLoop, 
              sizePayload);
          }

          // Step the Squad
          GSet completedTasks = SquadStep(that);
          nbComplete += GSetNbElem(&completedTasks);
          while (GSetNbElem(&completedTasks) > 0L) {
            SquidletTaskRequest* task = GSetPop(&completedTasks);
            // If the task failed
            if (strstr(task->_bufferResult, 
              "\"success\":\"1\"") == NULL) {
              SquidletTaskRequestPrint(task, stdout);
              fprintf(stream, " failed !!\n");
              fprintf(stream, "%s\n", task->_bufferResult);
              flagStop = true;
            }
            SquidletTaskRequestFree(&task);
          }
          gettimeofday(&stop, NULL);
        } while (!flagStop && (stop.tv_sec - start.tv_sec) < lengthTest);

        // Wait for the running tasks to finish
        while (SquadGetNbRunningTasks(that) > 0) {
          GSet completedTasks = SquadStep(that);
          nbComplete += GSetNbElem(&completedTasks);
          while (GSetNbElem(&completedTasks) > 0L) {
            SquidletTaskRequest* task = GSetPop(&completedTasks);
            // If the task failed
            if (strstr(task->_bufferResult, 
              "\"success\":\"1\"") == NULL) {
              SquidletTaskRequestPrint(task, stdout);
              fprintf(stream, " failed !!\n");
              fprintf(stream, "%s\n", task->_bufferResult);
              flagStop = true;
            }
            SquidletTaskRequestFree(&task);
          }
        } 
        gettimeofday(&stop, NULL);
        
        unsigned long deltams = (stop.tv_sec - start.tv_sec) * 1000 + 
          (stop.tv_usec - start.tv_usec) / 1000;
        float timePerTaskMs = (float) deltams / (float)nbComplete;
        fprintf(stream, "%04d\t%08u\t%07lu\t%011.2f\n", 
          nbLoop, sizePayload, nbComplete, timePerTaskMs);
        fflush(stdout);
      }
    }
  }
  fprintf(stream, "-- Benchmark ended --\n");
  
}

// Put back the 'task' into the set of task to complete of the Squad 
// 'that'
// Failed tasks (by timeout due to there 'maxWait' in 
// SquadAddTask_xxx() or by failure code from the squidlet in the 
// result data) are automatically put back into the set of task to 
// complete
void SquadTryAgainTask(
                Squad* const that, 
  SquidletTaskRequest* const task) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
  if (task == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'task' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Ensure the result buffer is empty
  if (task->_bufferResult != NULL) {
    free(task->_bufferResult);
    task->_bufferResult = NULL;
  }
  // Put back the task in the set of task to complete
  GSetAppend((GSet*)SquadTasks(that), task);
}

// -------------- Squidlet

// ================= Global variable ==================

// Variable to handle the signal Ctrl-C
bool Squidlet_CtrlC = false;

// ================ Functions implementation ====================

// Handler for the signal Ctrl-C
void SquidletHandlerCtrlC(
  const int sig) {
  (void)sig;
  Squidlet_CtrlC = true;
  time_t intTime = time(NULL);
  char* strIntTime = ctime(&intTime);
  printf("Squidlet :  !!! Interrupted by Ctrl-C !!! %s", strIntTime);
  fflush(stdout);
}

// Handler for the signal SIGPIPE
void SquidletHandlerSigPipe(
  const int sig) {
  (void)sig;
  time_t intTime = time(NULL);
  char* strIntTime = ctime(&intTime);
  printf("Squidlet :  !!! Received SIGPIPE !!! %s", strIntTime);
  fflush(stdout);
}

// Return a new Squidlet listening to the ip 'addr' and port 'port'
// If 'addr' equals 0, select automatically the first network address 
// of the host 
// If 'port' equals -1, select automatically one available between 
// THESQUID_PORTMIN and THESQUID_PORTMAX
Squidlet* SquidletCreateOnPort(
  const uint32_t addr, 
       const int port) {
  // Allocate memory for the squidlet
  Squidlet* that = PBErrMalloc(TheSquidErr, sizeof(Squidlet));
  
  // Open the socket
  that->_fd = socket(AF_INET, SOCK_STREAM, 0);
  
  // If we couldn't open the socket
  if (that->_fd == -1) {
    // Free memory and return null
    free(that);
    sprintf(TheSquidErr->_msg, "socket() failed");
    return NULL;
  }

  // Set the timeout for sending and receiving on this socket to 
  // THESQUID_ACCEPT_TIMEOUT seconds
  struct timeval tv;
  tv.tv_sec = THESQUID_ACCEPT_TIMEOUT;
  tv.tv_usec = 0;  
  int reuse = 1;
  if (setsockopt(that->_fd, SOL_SOCKET, SO_SNDTIMEO, 
    (char*)&tv, sizeof(tv)) == -1 ||
    setsockopt(that->_fd, SOL_SOCKET, SO_RCVTIMEO, 
    (char*)&tv, sizeof(tv)) == -1 ||
    setsockopt(that->_fd, SOL_SOCKET, SO_REUSEADDR,
    &reuse,sizeof(int))) {
    // If we couldn't set the timeout, free memory and return null
    close(that->_fd);
    free(that);
    sprintf(TheSquidErr->_msg, "setsockopt() failed");
    return NULL;
  }

  // Get the hostname
  if (gethostname(that->_hostname, sizeof(that->_hostname)) == -1) {
    // If we couldn't get the hostname, free memory and 
    // return null
    close(that->_fd);
    free(that);
    sprintf(TheSquidErr->_msg, "gethostname() failed");
    return NULL;
  }
  
  // Get the info about the host
  that->_host = gethostbyname(that->_hostname); 
  if (that->_host == NULL) {
    // If we couldn't get the host info, free memory and 
    // return null
    close(that->_fd);
    free(that);
    sprintf(TheSquidErr->_msg, "gethostbyname() failed");
    return NULL;
  }
  
  // Init the port and socket info
  memset(&(that->_sock), 0, sizeof(struct sockaddr_in));
  that->_sock.sin_family = AF_INET;
  if (addr != 0)
    that->_sock.sin_addr.s_addr = addr;
  else
    that->_sock.sin_addr.s_addr = *(uint32_t*)(that->_host->h_addr_list[0]);
  if (port != -1)
    that->_port = port;
  else
    that->_port = THESQUID_PORTMIN;
  that->_sock.sin_port = htons(that->_port);
  
  // If the port is not specified by the user, bind the socket on the 
  // first available port
  if (port == -1) {
    while (bind(that->_fd, (struct sockaddr *)&(that->_sock), 
      sizeof(struct sockaddr_in)) == -1 && 
      that->_port < THESQUID_PORTMAX) {
      ++(that->_port);
      that->_sock.sin_port = htons(that->_port);
    }
    if (that->_port == THESQUID_PORTMAX) {
      // If we couldn't bind the socket, close it, free memory and 
      // return null
      close(that->_fd);
      free(that);
      sprintf(TheSquidErr->_msg, "bind() failed");
      return NULL;
    }
  // Else if the port is specified by the user, try to bind only on 
  // that port
  } else {
    if (bind(that->_fd, (struct sockaddr *)&(that->_sock), 
      sizeof(struct sockaddr_in)) == -1) {
      // If we couldn't bind the socket, close it, free memory and 
      // return null
      close(that->_fd);
      free(that);
      sprintf(TheSquidErr->_msg, "bind() failed");
      return NULL;
    }
  }
  
  // Start listening through the socket
  if (listen(that->_fd, THESQUID_NBMAXPENDINGCONN) == -1) {
    // If we can't listen through the socket, close it, free memory and 
    // return null
    close(that->_fd);
    free(that);
    sprintf(TheSquidErr->_msg, "listen() failed");
    return NULL;
  }

  // Init the PID
  that->_pid = getpid();

  // Init the socket for reply
  that->_sockReply = -1;

  // Set the handler to catch the signal Ctrl-C
  signal(SIGINT, SquidletHandlerCtrlC);

  // Init the stream for output
  that->_streamInfo = NULL;
  
  // Return the new squidlet
  return that;
}

// Free the memory used by the Squidlet 'that'
void SquidletFree(
  Squidlet** that) {
  // If the pointer is null there is nothing to do
  if (that == NULL || *that == NULL)
    return;

  // Close the socket
  close((*that)->_fd);

  // Close the socket fo reply if opened
  if ((*that)->_sockReply != -1)
    close((*that)->_sockReply);

  // Free memory
  free(*that);
  *that = NULL;
}

// Print the PID, Hostname, IP and Port of the Squidlet 'that' on the 
// file 'stream'
// Example: 100 localhost 0.0.0.0:3000
void SquidletPrint(
  const Squidlet* const that, 
            FILE* const stream) {
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
  fprintf(stream, "%d %s %s:%d", SquidletGetPID(that), 
    SquidletHostname(that), SquidletIP(that), SquidletGetPort(that));
}

// Wait for a task request to be received by the Squidlet 'that'
// Return the received task request, or give up after 
// THESQUID_ACCEPT_TIMEOUT if there was no request and return a task 
// request of type SquidletTaskType_Null
SquidletTaskRequest SquidletWaitRequest(
  Squidlet* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Declare a variable to memorize the reply to the request
  char reply = THESQUID_TASKREFUSED;

  // Declare a buffer to receive the payload of the connection
  SquidletTaskRequest taskRequest;
  taskRequest._type = SquidletTaskType_Null;

  // Declare a variable to memorize the info about the incoming connection
  struct sockaddr_in incomingSock;
  socklen_t incomingSockSize = sizeof(incomingSock);
  
  // Make sure the socket for reply is closed
  if (that->_sockReply != -1) {
    close(that->_sockReply);
    that->_sockReply = -1;
  }
  
  // Extract the first connection request on the queue of pending 
  // connections if there was one. If there are not wait for
  // one during THESQUID_ACCEPT_TIMEOUT seconds and then give up
  that->_sockReply = accept(that->_fd, (struct sockaddr *)&incomingSock,
    &incomingSockSize);

  // If we would extract a pending connection
  if (that->_sockReply >= 0) {
    if (SquidletStreamInfo(that)){
      SquidletPrint(that, SquidletStreamInfo(that));
      fprintf(SquidletStreamInfo(that), " : accepted connection\n");
    }
    
    // Set the timeout for sending and receiving on the
    // extracted socket to THESQUID_PROC_TIMEOUT sec
    struct timeval tv;
    tv.tv_sec = THESQUID_PROC_TIMEOUT;
    tv.tv_usec = 0;  
    int reuse = 1;
    if (setsockopt(that->_sockReply, SOL_SOCKET, SO_SNDTIMEO, 
      (char*)&tv, sizeof(tv)) == -1 ||
      setsockopt(that->_sockReply, SOL_SOCKET, SO_RCVTIMEO, 
      (char*)&tv, sizeof(tv)) == -1 ||
      setsockopt(that->_sockReply, SOL_SOCKET, SO_REUSEADDR,
      &reuse,sizeof(int))) {
      // If we couldn't set the timeout, do not process the task
      taskRequest._type = SquidletTaskType_Null;
      reply = THESQUID_TASKREFUSED;

      if (SquidletStreamInfo(that)){
        SquidletPrint(that, SquidletStreamInfo(that));
        fprintf(SquidletStreamInfo(that), " : setsockopt failed\n");
      }

    } else {

      // Receive the task type, give up after 
      // THESQUID_PROC_TIMEOUT seconds
      if (SocketRecv(&(that->_sockReply), sizeof(SquidletTaskType), 
        (char*)&taskRequest, THESQUID_PROC_TIMEOUT)) {
        reply = THESQUID_TASKACCEPTED;
        if (SquidletStreamInfo(that)){
          SquidletPrint(that, SquidletStreamInfo(that));
          fprintf(SquidletStreamInfo(that), 
            " : received task type %d\n", taskRequest._type);
        }
      } else {
        // If we couldn't received the task type
        taskRequest._type = SquidletTaskType_Null;
        reply = THESQUID_TASKREFUSED;
        if (SquidletStreamInfo(that)){
          SquidletPrint(that, SquidletStreamInfo(that));
          fprintf(SquidletStreamInfo(that),
            " : couldn't receive task type\n");
        }
      }

      // Send the reply to the task request
      int flags = 0;
      if (send(that->_sockReply, &reply, sizeof(reply), flags) == -1) {
        // If we couldn't send the reply, do not process the task
        taskRequest._type = SquidletTaskType_Null;
        if (SquidletStreamInfo(that)){
          SquidletPrint(that, SquidletStreamInfo(that));
          fprintf(SquidletStreamInfo(that),
            " : couldn't sent reply to task request %d\n", reply);
        }
      } else {
        if (SquidletStreamInfo(that)){
          SquidletPrint(that, SquidletStreamInfo(that));
          fprintf(SquidletStreamInfo(that),
            " : sent reply to task request %d\n", reply);
        }
      }
    }
  }

  // Return the received task request
  return taskRequest;
}

// Process the task request 'request' with the Squidlet 'that'
void SquidletProcessRequest(
             Squidlet* const that, 
  SquidletTaskRequest* const request) {
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

  // If the task is of type null
  if (request->_type == SquidletTaskType_Null) {
    // Nothing to do
    return;
  }

  if (SquidletStreamInfo(that)){
    SquidletPrint(that, SquidletStreamInfo(that));
    fprintf(SquidletStreamInfo(that), " : process task\n");
  }

  // Declare a variable to store the error message if any
  char errMsg[300] = {0};

  // Declare a variable to memorize the size in byte of the input data
  size_t sizeInputData = 0;

  // Declare a buffer for the result data
  char* bufferResult = NULL;

  // Wait to receive the size of the input data with a time limit of 5s
  if (SocketRecv(&(that->_sockReply), sizeof(size_t), 
    (char*)&sizeInputData, 5)) {

    // Declare a buffer for the raw input data
    char* buffer = NULL;

    // If there are input data
    if (sizeInputData > 0) {
      if (SquidletStreamInfo(that)) {
        SquidletPrint(that, SquidletStreamInfo(that));
        fprintf(SquidletStreamInfo(that), 
          " : received size task data %d\n", sizeInputData);
      }

      // Allocate memory for the input data
      buffer = PBErrMalloc(TheSquidErr, sizeInputData + 1);
      memset(buffer, 0, sizeInputData + 1);
      
      // Wait to receive the input data with a time limit proportional
      // to the size of input data
      int timeLimit = 5 + (int)round((float)sizeInputData / 100.0);
      if (!SocketRecv(&(that->_sockReply), sizeInputData, buffer, 
        timeLimit)) {
        // If we coudln't received the input data
        free(buffer);
        buffer = NULL;
        if (SquidletStreamInfo(that)){
          SquidletPrint(that, SquidletStreamInfo(that));
          fprintf(SquidletStreamInfo(that), 
            " : couldn't receive task data\n");
        }
        sprintf(errMsg, "couldn't receive task data (%s)", 
          strerror(errno));
      } else {
        if (SquidletStreamInfo(that)){
          SquidletPrint(that, SquidletStreamInfo(that));
          if (strlen(buffer) > 50) {
            char tmp[4];
            tmp[0] = buffer[47];
            tmp[1] = buffer[48];
            tmp[2] = buffer[49];
            tmp[3] = buffer[50];
            buffer[47] = ' ';
            buffer[48] = '.';
            buffer[49] = '.';
            buffer[50] = '\0';
            fprintf(SquidletStreamInfo(that), 
              " : received task data %s\n", buffer);
            buffer[47] = tmp[0];
            buffer[48] = tmp[1];
            buffer[49] = tmp[2];
            buffer[50] = tmp[3];
          } else {
            fprintf(SquidletStreamInfo(that), 
              " : received task data %s\n", buffer);
          }
        }
      }
    }

    // If we could receive the expected data
    if (sizeInputData > 0 && buffer != NULL) {

      // Switch according to the request type
      switch (request->_type) {
        case SquidletTaskType_Dummy:
          SquidletProcessRequest_Dummy(that, buffer, &bufferResult);
          break;
        case SquidletTaskType_Benchmark:
          SquidletProcessRequest_Benchmark(that, buffer, &bufferResult);
          break;
        case SquidletTaskType_PovRay:
          SquidletProcessRequest_PovRay(that, buffer, &bufferResult);
          break;
        default:
          break;
      }

      // Free memory
      free(buffer);
    }
  } else {

    if (SquidletStreamInfo(that)){
      SquidletPrint(that, SquidletStreamInfo(that));
      fprintf(SquidletStreamInfo(that), 
        " : couldn't receive data size\n");
    }
    sprintf(errMsg, "couldn't receive data size (%s)", 
      strerror(errno));

  }
  
  // Send the result data size
  int flags = 0;
  size_t len = strlen(bufferResult);
  if (send(that->_sockReply, 
    (char*)&len, sizeof(size_t), flags) != -1) {

    if (SquidletStreamInfo(that)){
      SquidletPrint(that, SquidletStreamInfo(that));
      fprintf(SquidletStreamInfo(that), 
        " : sent result size\n");
      SquidletPrint(that, SquidletStreamInfo(that));
      fprintf(SquidletStreamInfo(that), 
        " : wait for acknowledgement from squad\n");
    }

    // Receive the acknowledgement
    char ack = 0;
    int waitDelayMaxSec = 60;
    if (SocketRecv(&(that->_sockReply), sizeof(char), &ack, 
      waitDelayMaxSec)) {

      if (SquidletStreamInfo(that)){
        SquidletPrint(that, SquidletStreamInfo(that));
        fprintf(SquidletStreamInfo(that), 
          " : received acknowledgement from squad\n");
      }

      if (send(that->_sockReply, bufferResult, len, flags) != -1) {
        if (SquidletStreamInfo(that)){
          SquidletPrint(that, SquidletStreamInfo(that));
          fprintf(SquidletStreamInfo(that), 
            " : sent result %s\n", bufferResult);
        }
      } else {
        if (SquidletStreamInfo(that)){
          SquidletPrint(that, SquidletStreamInfo(that));
          fprintf(SquidletStreamInfo(that), 
            " : couldn't send result %s\n", bufferResult);
        }
      }
    } else {
      if (SquidletStreamInfo(that)){
        SquidletPrint(that, SquidletStreamInfo(that));
        fprintf(SquidletStreamInfo(that), 
          " : couldn't receive acknowledgement\n");
      }
    }
  } else {
    if (SquidletStreamInfo(that)){
      SquidletPrint(that, SquidletStreamInfo(that));
      fprintf(SquidletStreamInfo(that), 
        " : couldn't send data size\n");
    }
  }
  
  // Free memory 
  if (bufferResult != NULL) {
    free(bufferResult);
  }

  // Close the output socket for this task after receiving the 
  // acknowledgement of reception of reply from the squad
  // Give up after THESQUID_PROC_TIMEOUT seconds

  if (SquidletStreamInfo(that)){
    SquidletPrint(that, SquidletStreamInfo(that));
    fprintf(SquidletStreamInfo(that), 
      " : wait for acknowledgement from squad\n");
  }

  char ack = 0;
  if (SocketRecv(&(that->_sockReply), 1, &ack, 
    THESQUID_PROC_TIMEOUT)) {
    if (request->_type != SquidletTaskType_Null) {
      if (SquidletStreamInfo(that)){
        SquidletPrint(that, SquidletStreamInfo(that));
        fprintf(SquidletStreamInfo(that), 
          " : received acknowledgement from squad\n");
      }
    }
  } else {
    if (SquidletStreamInfo(that)){
      SquidletPrint(that, SquidletStreamInfo(that));
      fprintf(SquidletStreamInfo(that), 
        " : couldn't receive acknowledgement from squad\n");
    }
  }

  if (SquidletStreamInfo(that)){
    SquidletPrint(that, SquidletStreamInfo(that));
    fprintf(SquidletStreamInfo(that), 
      " : ready for next task\n");
  }

}

// Process a dummy task request with the Squidlet 'that'
// The task request parameters are encoded in JSON and stored in the 
// string 'buffer'
// The result of the task are encoded in JSON format and stored in 
// 'bufferResult' which is allocated as necessary
void SquidletProcessRequest_Dummy(
    Squidlet* const that,
  const char* const buffer, 
             char** bufferResult) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Declare a variable to memorize if the process has been successful
  bool success = false;

  // Declare a variable to memorize the result of processing
  int result = 0; 

  // Process the data
  if (SquidletStreamInfo(that)){
    SquidletPrint(that, SquidletStreamInfo(that));
    fprintf(SquidletStreamInfo(that), 
      " : process dummy task %s\n", buffer);
  }

  // Decode the input from JSON
  JSONNode* json = JSONCreate();
  if (JSONLoadFromStr(json, buffer)) {
    // Get the value to process
    JSONNode* prop = JSONProperty(json, "v");
    if (prop != NULL) {
      int v = atoi(JSONLabel(JSONValue(prop, 0)));
      // Process the value
      result = v * -1;
      // Sleep for v seconds
      sleep(v);
      // Set the flag for successfull process
      success = true;
    }
  }
  JSONFree(&json);

  // Prepare the result data as JSON
  *bufferResult = PBErrMalloc(TheSquidErr, 100);
  memset(*bufferResult, 0, 100);
  char* temperature = SquidletGetTemperature(that);
  if (temperature != NULL) {
    sprintf(*bufferResult, 
      "{\"success\":\"%d\",\"v\":\"%d\",\"temperature\":\"%s\"}", 
      success, result, temperature);
    free(temperature);
  } else {
    sprintf(*bufferResult, 
      "{\"success\":\"%d\",\"v\":\"%d\",\"temperature\":\"\"}", 
      success, result);
  }

}  

// Process a benchmark task request with the Squidlet 'that'
// The task request parameters are encoded in JSON and stored in the 
// string 'buffer'
// The result of the task are encoded in JSON format and stored in 
// 'bufferResult' which is allocated as necessary
void SquidletProcessRequest_Benchmark(
    Squidlet* const that,
  const char* const buffer, 
             char** bufferResult) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif

  // Declare a variable to memorize if the process has been successful
  bool success = false;

  // Variable to memorize the result of the benchmark function
  int result = 0;

  // Declare a variable to store the error message if any
  char errMsg[300] = {'\0'};

  // Decode the input from JSON
  JSONNode* json = JSONCreate();
  if (JSONLoadFromStr(json, buffer)) {
    // Get the value to process
    JSONNode* prop = JSONProperty(json, "nb");
    if (prop != NULL) {
      int nb = atoi(JSONLabel(JSONValue(prop, 0)));
      prop = JSONProperty(json, "v");
      if (prop != NULL) {
        if (SquidletStreamInfo(that)){
          SquidletPrint(that, SquidletStreamInfo(that));
          fprintf(SquidletStreamInfo(that), 
            " : run benchmark\n");
        }
        // Run the benchmark function
        result = TheSquidBenchmark(nb, 
          JSONLabel(JSONValue(prop, 0)));

        if (SquidletStreamInfo(that)){
          SquidletPrint(that, SquidletStreamInfo(that));
          fprintf(SquidletStreamInfo(that), 
            " : benchmark complete\n");
        }

        // Set the flag for successfull process
        success = true;

      } else {

        if (SquidletStreamInfo(that)){
          SquidletPrint(that, SquidletStreamInfo(that));
          fprintf(SquidletStreamInfo(that), 
            " : invalid data (v)\n");
        }
        sprintf(errMsg, "invalid data (v missing)");
      }

    } else {

      if (SquidletStreamInfo(that)){
        SquidletPrint(that, SquidletStreamInfo(that));
        fprintf(SquidletStreamInfo(that), 
          " : invalid data (nb)\n");
      }
      sprintf(errMsg, "invalid data (nb missing)");

    }

  } else {

    if (SquidletStreamInfo(that)){
      SquidletPrint(that, SquidletStreamInfo(that));
      fprintf(SquidletStreamInfo(that), 
        " : couldn't load json %s\n", buffer);
    }
    sprintf(errMsg, "couldn't load json (%s)", JSONErr->_msg);

  }
  JSONFree(&json);

  // Prepare the result data as JSON
  *bufferResult = PBErrMalloc(TheSquidErr, 500);
  memset(*bufferResult, 0, 500);
  char* temperature = SquidletGetTemperature(that);
  if (temperature != NULL) {
    sprintf(*bufferResult, "{\"success\":\"%d\",\"temp\":\"%s\", \"v\":\"%d\",\"err\":\"%s\"}", success, temperature, result, errMsg);
    free(temperature);
  } else {
    sprintf(*bufferResult, "{\"success\":\"%d\",\"temp\":\"\", \"v\":\"%d\",\"err\":\"%s\"}", success, result, errMsg);
  }

}  

// Process a Pov-Ray task request with the Squidlet 'that'
// The task request parameters are encoded in JSON and stored in the 
// string 'buffer'
// The result of the task are encoded in JSON format and stored in 
// 'bufferResult' which is allocated as necessary
void SquidletProcessRequest_PovRay(
    Squidlet* const that,
  const char* const buffer, 
             char** bufferResult) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Declare a variable to memorize if the process has been successful
  bool success = false;

  // Process the data
  if (SquidletStreamInfo(that)){
    SquidletPrint(that, SquidletStreamInfo(that));
    fprintf(SquidletStreamInfo(that), 
      " : process Pov-Ray task %s\n", buffer);
    fflush(SquidletStreamInfo(that));
  }

  // Decode the input from JSON
  JSONNode* json = JSONCreate();
  if (JSONLoadFromStr(json, buffer)) {
    // Get the arguments
    JSONNode* propIni = JSONProperty(json, "ini");
    JSONNode* propTga = JSONProperty(json, "tga");
    JSONNode* propTop = JSONProperty(json, "top");
    JSONNode* propLeft = JSONProperty(json, "left");
    JSONNode* propBottom = JSONProperty(json, "bottom");
    JSONNode* propRight = JSONProperty(json, "right");
    // If all the arguments are valids
    if (propIni != NULL && propTga != NULL && propTop != NULL && 
      propLeft != NULL && propBottom != NULL && propRight != NULL) {

      // Create the Pov-Ray command
      // povray +SC<left> +SR<top> +EC<right> +ER<bottom> +O<tga>
      //   +FT -D <ini>
      char cmd[500];
      sprintf(cmd, 
        "povray %s +SC%s +SR%s +EC%s +ER%s +O%s +FT -D", 
        JSONLabel(JSONValue(propIni, 0)),
        JSONLabel(JSONValue(propLeft, 0)), 
        JSONLabel(JSONValue(propTop, 0)), 
        JSONLabel(JSONValue(propRight, 0)), 
        JSONLabel(JSONValue(propBottom, 0)), 
        JSONLabel(JSONValue(propTga, 0)));

      // Execute the Pov-Ray command
      int ret = system(cmd);
      
      if (ret == 0) {
        // Set the flag for successfull process
        success = true;
      }
    }
  }

  // Prepare the result data as JSON
  size_t bufferResultLen = strlen(buffer) + 100;
  *bufferResult = PBErrMalloc(TheSquidErr, bufferResultLen);
  memset(*bufferResult, 0, bufferResultLen);
  char successStr[10];
  sprintf(successStr, "%d", success);
  JSONAddProp(json, "success", successStr);
  char* temperature = SquidletGetTemperature(that);
  if (temperature != NULL)
    JSONAddProp(json, "temperature", temperature);
  if (temperature != NULL)
    free(temperature);
  if (!JSONSaveToStr(json, *bufferResult, bufferResultLen, true)) {
    sprintf(*bufferResult, 
      "{\"success\":\"0\",\"temperature\":\"\","
      "\"err\":\"JSONSaveToStr failed\"}");
  }

  // Free memory
  JSONFree(&json);

}  

// Return the temperature of the squidlet 'that' as a string.
// The result depends on the architecture on which the squidlet is 
// running. It is '(?)' if the temperature is not availalble
char* SquidletGetTemperature(
  const Squidlet* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  (void)that;
#if BUILDARCH == 0
  return strdup("(?)");
#endif
#if BUILDARCH == 1
  // Declare a variable to pipe the shell command
  FILE* fp = NULL;
  // Run the command and pipe its output
  fp = popen("vcgencmd measure_temp", "r");
  if (fp != NULL) {
    // Declare a variable to store the output
    char output[100] = {0};
    // Read the output
    while (fgets(output, sizeof(output), fp) != NULL);
    // Close the pipe
    pclose(fp);
    // Remove the line return
    if (strlen(output) > 0)
      output[strlen(output) - 1] = '\0';
    // Return the result
    return strdup(output);
  } else {
    // Return the result
    return strdup("popen() failed");
  }
#endif
#if BUILDARCH == 2
  // Declare a variable to pipe the shell command
  FILE* fp = NULL;
  // Run the command and pipe its output
  fp = popen("vcgencmd measure_temp", "r");
  if (fp != NULL) {
    // Declare a variable to store the output
    char output[100] = {0};
    // Read the output
    while (fgets(output, sizeof(output), fp) != NULL);
    // Close the pipe
    pclose(fp);
    // Remove the line return
    if (strlen(output) > 0)
      output[strlen(output) - 1] = '\0';
    // Return the result
    return strdup(output);
  } else {
    // Return the result
    return strdup("popen() failed");
  }
#endif
}

// -------------- TheSquid 

// ================ Functions implementation ====================

// Function for benchmark purpose
int TheSquidBenchmark(
                int nbLoop, 
  const char* const buffer) {
  // Variable to memorize the dummy result
  int res = 0;
  // Loop on sample code
  for (int iLoop = 0; iLoop < nbLoop; ++iLoop) {
    for (int scaling = 200; scaling--;) {
      GSet set = GSetCreateStatic();
      for(unsigned long i = strlen(buffer); i--;) {
        GSetPush(&set, NULL);
        set._head->_sortVal = (float)((i + iLoop) % 10);
      }
      GSetSort(&set);
      res = (int)round(set._head->_sortVal);
      GSetFlush(&set);
    }
  }
  // Return the dummy result
  return res;
}

// Function to receive in blocking mode 'nb' bytes of data from
// the socket 'sock' and store them into 'buffer' (which must be big 
// enough). Give up after 'maxWait' seconds.
// Return true if we could read all the requested byte, false else
bool SocketRecv(short* sock, unsigned long nb, char* buffer, 
  const time_t maxWait) {
#if BUILDMODE == 0
  if (buffer == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'buffer' is null");
    PBErrCatch(TheSquidErr);
  }
#endif

  FILE* fp = fdopen(*sock, "r");

  // Declare a pointer to the next received byte
  char* freadPtr = buffer;
  
  // Declare a pointer to the byte after the last received byte
  char* freadPtrEnd = freadPtr + nb;

  // Declare variables to memorize the start time and elapsed time
  time_t startTime = time(NULL);
  time_t elapsedTime = 0;
  
  // While we haven't received all the requested bytes and the time
  // limit is not reached
  do {
    // Try to read one more byte, if successful moves the pointer to
    // the next byte to read by one byte
    //ssize_t nbReadByte = read(sock, freadPtr, nb);
    ssize_t nbReadByte = fread(freadPtr, 1, nb, fp);
    if (nbReadByte > 0) {
      freadPtr += nbReadByte;
    }
    // Update the elapsed time
    elapsedTime = time(NULL) - startTime;
  } while (freadPtr != freadPtrEnd && maxWait != 0 && 
    elapsedTime <= maxWait && !Squidlet_CtrlC);
  *sock = dup(*sock);
  fclose(fp);

  // Return the success/failure code
  if (freadPtr != freadPtrEnd) {
    return false;
  } else {
    return true;
  }

} 

