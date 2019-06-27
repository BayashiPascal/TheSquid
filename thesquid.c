// ============ THESQUID.C ================

// ================= Include =================

#include "thesquid.h"
#if BUILDMODE == 0
  #include "thesquid-inline.c"
#endif

// ================ Functions declaration ====================

// Function to receive in blocking mode 'nb' bytes of data from
// the socket 'sock' and store them into 'buffer' (which must be big 
// enough). Give up after 'timeout' seconds.
// Return true if we could read all the requested byte, false else
bool SocketRecv(short* sock, unsigned long nb, char* buffer, 
  int timeout);

// -------------- SquidletInfo

// ================ Functions implementation ====================

// Return a new SquidletInfo with 'name', 'ip' and 'port'
SquidletInfo* SquidletInfoCreate(char* name, char* ip, int port) {
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
void SquidletInfoFree(SquidletInfo** that) {
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

// Print the SquidletInfo 'that' on the 'stream'
void SquidletInfoPrint(const SquidletInfo* const that, 
  FILE* const stream) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Print the info on the stream 
  fprintf(stream, "%s(%s:%d)", that->_name, that->_ip, that->_port);
}

// -------------- SquidletTaskRequest

// ================ Functions implementation ====================

// Return a new SquidletTaskRequest with 'id' and 'type' and 'data'
SquidletTaskRequest* SquidletTaskRequestCreate(SquidletTaskType type, 
  unsigned long id, const char* const data, const time_t maxWait) {
  // Allocate memory for the new SquidletTaskRequest
  SquidletTaskRequest* that = PBErrMalloc(TheSquidErr, 
    sizeof(SquidletTaskRequest));
  
  // Init properties
  that->_id = id;
  that->_type = type;
  that->_data = strdup(data);
  that->_buffer = NULL;
  that->_maxWaitTime = maxWait;
  
  // Return the new SquidletTaskRequest
  return that;
}

// Free the memory used by the SquidletTaskRequest 'that'
void SquidletTaskRequestFree(SquidletTaskRequest** that) {
  // If the pointer is null there is nothing to do
  if (that == NULL || *that == NULL)
    return;
  
  // Free memory
  if ((*that)->_buffer != NULL)
    free((*that)->_buffer);
  free((*that)->_data);
  free(*that);
  *that = NULL;  
}

// Print the SquidletTaskRequest 'that' on the 'stream'
void SquidletTaskRequestPrint(const SquidletTaskRequest* const that, 
  FILE* const stream) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Print the info on the stream 
  fprintf(stream, "t%d #%lu", that->_type, that->_id);
}

// -------------- SquadRunningTask

// ================ Functions implementation ====================

// Return a new SquadRunningTask with the 'request' and 'squidlet'
SquadRunningTask* SquadRunningTaskCreate(
  SquidletTaskRequest* const request, SquidletInfo* const squidlet) {
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
void SquadRunningTaskFree(SquadRunningTask** that) {
  // If the pointer is null there is nothing to do
  if (that == NULL || *that == NULL)
    return;
  
  // Free memory
  SquidletInfoFree(&((*that)->_squidlet));
  SquidletTaskRequestFree(&((*that)->_request));
  free(*that);
  *that = NULL;
}

// Print the SquadRunningTask 'that' on the 'stream'
void SquadRunningTaskPrint(const SquadRunningTask* const that, 
  FILE* const stream) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
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
bool SquadDecodeAsJSON(Squad* that, JSONNode* json);

// Refresh the content of the TextOMeter attached to the 
// Squad 'that'
void SquadUpdateTextOMeter(const Squad* const that);

// Add one line to the history of messages for the TextOMeter
// 'msg' must be less than 100 characters long
void SquadPushHistory(Squad* const that, char* msg);

// Request the execution of a task on a squidlet for the squad 'that'
// Return true if the request was successfull, fals else
bool SquadSendTaskOnSquidlet(Squad* const that, 
  SquidletInfo* const squidlet, SquidletTaskRequest* const task);
  
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

// Load a list of tasks stored in json format from the 'stream'
// and add them to the set of tasks of the Squad 'that'
// Return true if the tasks could be loaded, else false
// Example:
// {"tasks":[
//   {"SquidletTaskType":"1", "id":"1", "maxWait":"1"},  
//   {"SquidletTaskType":"2", "id":"1", "maxWait":"1", 
//    "nb":"1", "payloadSize":"1"},
//   {"SquidletTaskType":"3", "id":"1", "maxWait":"1", 
//    "ini":"./testPov.ini", "sizeFragment":"100"}
// ]}
bool SquadLoadTasks(Squad* const that, FILE* const stream) {
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
      sprintf(TheSquidErr->_msg, "SquidletTaskType not found");
      JSONFree(&json);
      return false;
    }
    JSONNode* propId = JSONProperty(propTask, "id");
    if (propId == NULL) {
      TheSquidErr->_type = PBErrTypeInvalidData;
      sprintf(TheSquidErr->_msg, "id not found");
      JSONFree(&json);
      return false;
    }
    JSONNode* propMaxWait = JSONProperty(propTask, "maxWait");
    if (propMaxWait == NULL) {
      TheSquidErr->_type = PBErrTypeInvalidData;
      sprintf(TheSquidErr->_msg, "maxWait not found");
      JSONFree(&json);
      return false;
    }
    
    // Switch according to the type of task
    int type = atoi(JSONLabel(JSONValue(propType, 0)));
    unsigned long id = atol(JSONLabel(JSONValue(propId, 0)));
    time_t maxWait = atoi(JSONLabel(JSONValue(propMaxWait, 0)));
    JSONNode* prop = NULL;
    switch(type) {
      case SquidletTaskType_Dummy:
        SquadAddTask_Dummy(that, id, maxWait);
        break;
      case SquidletTaskType_Benchmark:
        prop = JSONProperty(propTask, "nb");
        if (prop == NULL) {
          TheSquidErr->_type = PBErrTypeInvalidData;
          sprintf(TheSquidErr->_msg, "nb not found");
          JSONFree(&json);
          return false;
        }
        int nb = atoi(JSONLabel(JSONValue(prop, 0)));
        prop = JSONProperty(propTask, "payloadSize");
        if (prop == NULL) {
          TheSquidErr->_type = PBErrTypeInvalidData;
          sprintf(TheSquidErr->_msg, "payloadSize not found");
          JSONFree(&json);
          return false;
        }
        size_t payloadSize = atol(JSONLabel(JSONValue(prop, 0)));
        SquadAddTask_Benchmark(that, id, maxWait, nb, payloadSize);
        break;
      case SquidletTaskType_PovRay:
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
        SquadAddTask_PovRay(that, id, maxWait, ini, 
          sizeMinFragment, sizeMaxFragment);
        free(ini);
        break;
      default:
        TheSquidErr->_type = PBErrTypeInvalidData;
        sprintf(TheSquidErr->_msg, "tasks not found");
        JSONFree(&json);
        return false;
    }
  }

  // Free the memory used by the JSON
  JSONFree(&json);
  
  // Return the succes code
  return true;
  
}

// Load the Squidlet info from the 'stream' into the 'that'
// Return true if it could load the info, else false
bool SquadLoadSquidlets(Squad* const that, FILE* const stream) {
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
    sprintf(TheSquidErr->_msg, "_squidlets not found");
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
      sprintf(TheSquidErr->_msg, "_name not found for squidlet %d",
        iSquidlet);
      return false;
    }

    // Get the property _ip of the squidlet
    JSONNode* propIp = JSONProperty(propSquidlet, "_ip");
    if (propIp == NULL) {
      TheSquidErr->_type = PBErrTypeInvalidData;
      sprintf(TheSquidErr->_msg, "_ip not found for squidlet %d",
        iSquidlet);
      return false;
    }

    // Get the property _port of the squidlet
    JSONNode* propPort = JSONProperty(propSquidlet, "_port");
    if (propPort == NULL) {
      TheSquidErr->_type = PBErrTypeInvalidData;
      sprintf(TheSquidErr->_msg, "_port not found for squidlet %d",
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
// Return true if the request could be sent and the squidlet
// accepted it, false else
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
  if (request->_type == SquidletTaskType_Null) {
    // Nothing to do
    return true;
  }
  
  // Declare some variables to process the lines of history
  char lineHistory[200];
  char bufferHistory[100];
  FILE* streamBufferHistory = NULL;

  // Create a socket
  
  if (squidlet->_sock != -1)
    close(squidlet->_sock);
  
  squidlet->_sock = socket(AF_INET, SOCK_STREAM, 0);
  
  // If we couldn't create the socket
  if (squidlet->_sock == -1) {

    if (SquadGetFlagTextOMeter(that) == true) {
      streamBufferHistory = fmemopen(bufferHistory, 100, "w");
      SquidletInfoPrint(squidlet, streamBufferHistory);
      fclose(streamBufferHistory);
      sprintf(lineHistory, 
        "can't create socket to squidlet (%s)\n", 
        bufferHistory);
      SquadPushHistory(that, lineHistory);
      sprintf(lineHistory, "errno: %s\n", strerror(errno));
      SquadPushHistory(that, lineHistory);
    }

    return false;
  }
  
  // Create the data for the connection to the squidlet
  struct sockaddr_in remote = {0};
  remote.sin_addr.s_addr = inet_addr(squidlet->_ip);
  remote.sin_family = AF_INET;
  remote.sin_port = htons(squidlet->_port);

  // Connect to the squidlet
  if (connect(squidlet->_sock, (struct sockaddr*)&remote, 
    sizeof(struct sockaddr_in)) == -1) {
    // If the connection failed
    close(squidlet->_sock);
    squidlet->_sock = -1;
    if (SquadGetFlagTextOMeter(that) == true) {
      streamBufferHistory = fmemopen(bufferHistory, 100, "w");
      SquidletInfoPrint(squidlet, streamBufferHistory);
      fclose(streamBufferHistory);
      sprintf(lineHistory, 
        "connection to squidlet (%s) failed\n", 
        bufferHistory);
      SquadPushHistory(that, lineHistory);
      sprintf(lineHistory, "errno: %s\n", strerror(errno));
      SquadPushHistory(that, lineHistory);
    }

    return false;
  }

  if (SquadGetFlagTextOMeter(that) == true) {
    streamBufferHistory = fmemopen(bufferHistory, 100, "w");
    SquidletInfoPrint(squidlet, streamBufferHistory);
    fclose(streamBufferHistory);
    sprintf(lineHistory, 
      "connected to squidlet (%s)\n", 
      bufferHistory);
    SquadPushHistory(that, lineHistory);
  }

  // Set the timeout of the socket for sending and receiving to 1s
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  int reuse = 1;
  if (setsockopt(squidlet->_sock, SOL_SOCKET, SO_SNDTIMEO, 
    (char*)&tv, sizeof(tv)) == -1 ||
    setsockopt(squidlet->_sock, SOL_SOCKET, SO_RCVTIMEO, 
    (char*)&tv, sizeof(tv)) == -1 ||
    setsockopt(squidlet->_sock, SOL_SOCKET, SO_REUSEADDR,
    &reuse,sizeof(int))) {
    // If we couldn't set the timeout
    close(squidlet->_sock);
    squidlet->_sock = -1;

    if (SquadGetFlagTextOMeter(that) == true) {
      streamBufferHistory = fmemopen(bufferHistory, 100, "w");
      SquidletInfoPrint(squidlet, streamBufferHistory);
      fclose(streamBufferHistory);
      sprintf(lineHistory, 
        "setsockopt to squidlet (%s) failed\n", 
        bufferHistory);
      SquadPushHistory(that, lineHistory);
      sprintf(lineHistory, "errno: %s\n", strerror(errno));
      SquadPushHistory(that, lineHistory);
    }

    return false;
  }

  if (SquadGetFlagTextOMeter(that) == true) 
    SquadPushHistory(that, "set timeout\n");

  // Send the task request
  int flags = 0;
  if (send(squidlet->_sock, 
    request, sizeof(SquidletTaskRequest), flags) == -1) {
    // If we couldn't send the request
    close(squidlet->_sock);
    squidlet->_sock = -1;

    if (SquadGetFlagTextOMeter(that) == true) {
      streamBufferHistory = fmemopen(bufferHistory, 100, "w");
      SquidletInfoPrint(squidlet, streamBufferHistory);
      fclose(streamBufferHistory);
      sprintf(lineHistory, 
        "send to squidlet (%s) failed\n", 
        bufferHistory);
      SquadPushHistory(that, lineHistory);
      sprintf(lineHistory, "errno: %s\n", strerror(errno));
      SquadPushHistory(that, lineHistory);
    }

    return false;
  }

  if (SquadGetFlagTextOMeter(that) == true) 
    SquadPushHistory(that, "sent task request\n");

  // Wait for the reply from the squidlet up to 5s
  char reply = THESQUID_TASKREFUSED;
  if (!SocketRecv(&(squidlet->_sock), sizeof(reply), &reply, 5) ||
    reply == THESQUID_TASKREFUSED) {
    // If we couldn't receive the reply or the reply timed out or
    // the squidlet refused the task
    close(squidlet->_sock);
    squidlet->_sock = -1;

    if (SquadGetFlagTextOMeter(that) == true) {
      streamBufferHistory = fmemopen(bufferHistory, 100, "w");
      SquidletInfoPrint(squidlet, streamBufferHistory);
      fclose(streamBufferHistory);
      sprintf(lineHistory, 
        "task refused by squidlet (%s)\n", 
        bufferHistory);
      SquadPushHistory(that, lineHistory);
    }

    return false;
  }

  if (SquadGetFlagTextOMeter(that) == true) {
    streamBufferHistory = fmemopen(bufferHistory, 100, "w");
    SquidletInfoPrint(squidlet, streamBufferHistory);
    fclose(streamBufferHistory);
    sprintf(lineHistory, 
      "task accepted by squidlet (%s)\n", 
      bufferHistory);
    SquadPushHistory(that, lineHistory);
  }

  // Return the success code
  return true;
}

// Add a new dummy task with 'id' to execute to the squad 'that'
// Wait for a maximum of 'maxWait' seconds for the task to complete
// The total size of the data must be less than 1024 bytes
void SquadAddTask_Dummy(Squad* const that, const unsigned long id,
  const time_t maxWait) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Create the new task
  // Prepare the data as JSON
  char buffer[100];
  memset(buffer, 0, 100);
  sprintf(buffer, "{\"v\":\"%d\"}", (int)id);
  SquidletTaskRequest* task = SquidletTaskRequestCreate(
    SquidletTaskType_Dummy, id, buffer, maxWait);
  
  // Add the new task to the set of task to execute
  GSetAppend((GSet*)SquadTasks(that), task);
}

// Add a new benchmark task with 'id' to execute to the squad 'that'
// Wait for a maximum of 'maxWait' seconds for the task to complete
// Uses a payload of 'payloadSize' bytes
// The total size of the data must be less than 1024 bytes
void SquadAddTask_Benchmark(Squad* const that, const unsigned long id,
  const time_t maxWait, const int nb, const size_t payloadSize) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Create the new task
  // Prepare the data as JSON

  char* payload = PBErrMalloc(TheSquidErr, 27);
  for (size_t i = 0; i < 26; ++i)
    payload[i] = 'a' + i;
  payload[26] = 0;

  char bufferNb[100];
  sprintf(bufferNb, "%d", nb);
  int nbSize = strlen(bufferNb);
  char bufferId[100];
  sprintf(bufferId, "%ld", id);
  int idSize = strlen(bufferId);
  char* buffer = PBErrMalloc(TheSquidErr, 
    payloadSize + idSize + nbSize + 25 + 26);
  sprintf(buffer, "{\"id\":\"%s\",\"nb\":\"%s\",\"v\":\"%s\"}", 
    bufferId, bufferNb, payload);
  unsigned long bufferLen = strlen(buffer);
  memset(buffer + bufferLen, ' ', payloadSize);
  buffer[bufferLen + payloadSize] = '\0';
  free(payload);
  SquidletTaskRequest* task = SquidletTaskRequestCreate(
    SquidletTaskType_Benchmark, id, buffer, maxWait);
  free(buffer);
  
  // Add the new task to the set of task to execute
  GSetAppend((GSet*)SquadTasks(that), task);
}

// Add a new Pov-Ray task with 'id' to execute to the squad 'that'
// Render the scene described in the Pov-Ray ini file 'ini'
// The ini file must at least contains the following lines (without 
// inline comments
//
// Input_File_Name=<script.pov>
// Width=<width> 
// Height=<height>
// Output_File_Name=<image.tga>
//
// The output format of the image must be TGA
// Wait for a maximum of 'maxWait' seconds for the task to complete
// The total size of the data must be less than 1024 bytes
// The random generator must have been initialised before calling this 
// function
void SquadAddTask_PovRay(Squad* const that, const unsigned long id,
  const time_t maxWait, const char* const ini, 
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
  // Initi variables for the creation of tasks
  unsigned long width = 0;
  unsigned long height = 0;
  char* outImgPath = NULL;

  // Decode the ini file
  FILE* fp = fopen(ini, "r");
  if (fp == NULL) {
    TheSquidErr->_type = PBErrTypeInvalidArg;
    sprintf(TheSquidErr->_msg, "Can't open %s", ini);
    PBErrCatch(TheSquidErr);
  } else {
    char oneLine[1024];
    while(fgets(oneLine, 1024, fp)) {
      if (strstr(oneLine, "Width=")) {
        sscanf(oneLine + 6, "%lu", &width);
      } else if (strstr(oneLine, "Height=")) {
        sscanf(oneLine + 7, "%lu", &height);
      } else if (strstr(oneLine, "Output_File_Name=") &&
        strlen(oneLine) > 17) {
        outImgPath = strdup(oneLine + 17);
        // Remove the return line
        outImgPath[strlen(outImgPath) - 1] = '\0';
        // Make sure the output file doesn't exists
        char cmd[500];
        sprintf(cmd, "rm -f %s", outImgPath);
        int ret = system(cmd);
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
      unsigned int curId = taskId + id;
      sprintf(buffer, 
        "{\"id\":\"%d\",\"ini\":\"%s\",\"tga\":\"%s\","
        "\"top\":\"%lu\",\"left\":\"%lu\",\"bottom\":\"%lu\""
        ",\"right\":\"%lu\",\"width\":\"%lu\",\"height\":\"%lu\""
        ",\"outTga\":\"%s\"}", 
        curId, ini, tga, top, left, bottom, right, width, height,
        outImgPath);
      // Add the new task to the set of task to execute
      SquidletTaskRequest* task = SquidletTaskRequestCreate(
        SquidletTaskType_PovRay, curId, buffer, maxWait);
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

// Send the data associated to 'task' from the Squad 'that' to 
// the Squidlet 'squidlet'
// Return true if the data could be sent, false else
bool SquadSendTaskData(Squad* const that, 
  SquidletInfo* const squidlet, SquidletTaskRequest* const task) {
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
        "couldn't send task data size %d\n", len);
      SquadPushHistory(that, lineHistory);
    }

    return false;
  } else {

    if (SquadGetFlagTextOMeter(that) == true) {
      streamBufferHistory = fmemopen(bufferHistory, 100, "w");
      SquidletInfoPrint(squidlet, streamBufferHistory);
      fclose(streamBufferHistory);
      sprintf(lineHistory, 
        "sent task data size %d to (%s)\n", len, 
        bufferHistory);
      SquadPushHistory(that, lineHistory);
    }
  }

  // Send the task data
  if (send(squidlet->_sock, task->_data, len, flags) == -1) {
    // If we couldn't send the data

    if (SquadGetFlagTextOMeter(that) == true)
      SquadPushHistory(that, "couldn't send task data\n");

    return false;
  } else {
    if (SquadGetFlagTextOMeter(that) == true)
      SquadPushHistory(that, "sent task data\n");
  }

  // Return the success code
  return true;
}

// Receive the result from the running task 'runningTask', non blocking
// If the result is ready it is stored in the SquidletInfo
// Return true if we received the result, false else
bool SquadReceiveTaskResult(Squad* const that, 
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

  // Declare a variable to memorize the size in byte of the input data
  size_t sizeResultData = 0;

  // Helper variables
  SquidletInfo* squidlet = runningTask->_squidlet;
  SquidletTaskRequest* task = runningTask->_request;

  // Declare some variables to process the lines of history
  char lineHistory[200];
  char bufferHistory[100];
  FILE* streamBufferHistory = NULL;
  
  // Make sure the buffer to receive the task is empty
  if (task->_buffer != NULL) {
    free(task->_buffer);
    task->_buffer = NULL;
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
          "received the size of result from (%s)\n", 
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
          "send ack of received size of result data to (%s)\n", 
          bufferHistory);
        SquadPushHistory(that, lineHistory);
      }

      // Allocate memory for the result data
      task->_buffer = PBErrMalloc(TheSquidErr, sizeResultData + 1);
      memset(task->_buffer, 0, sizeResultData + 1);
      
      // Wait to receive the result data with a time limit proportional
      // to the size of result data
      int timeOut = 5 + (int)round((float)sizeResultData / 100.0);
      if (!SocketRecv(&(squidlet->_sock), sizeResultData, 
        task->_buffer, timeOut)) {
        // If we coudln't received the result data
        free(task->_buffer);
        task->_buffer = NULL;

        if (SquadGetFlagTextOMeter(that) == true) {
          streamBufferHistory = fmemopen(bufferHistory, 100, "w");
          SquidletInfoPrint(squidlet, streamBufferHistory);
          fclose(streamBufferHistory);
          sprintf(lineHistory, 
            "couldn't received result data from (%s)\n", 
            bufferHistory);
          SquadPushHistory(that, lineHistory);
          sprintf(lineHistory, "waited for %ds\n", 
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
            "received result data from (%s)\n", 
            bufferHistory);
          SquadPushHistory(that, lineHistory);
          sprintf(lineHistory, "size result data %d\n", 
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
          "received a null size of result from (%s)\n", 
          bufferHistory);
        SquadPushHistory(that, lineHistory);
      }

    }
  } else {

    if (SquadGetFlagTextOMeter(that) == true) {
      streamBufferHistory = fmemopen(bufferHistory, 100, "w");
      SquidletInfoPrint(squidlet, streamBufferHistory);
      fclose(streamBufferHistory);
      sprintf(lineHistory, 
        "haven't received result from (%s)\n", 
        bufferHistory);
      SquadPushHistory(that, lineHistory);
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
        sprintf(lineHistory, "task %s running\n", 
          bufferHistory);
        SquadPushHistory(that, lineHistory);
      }

      GSetAppend((GSet*)SquadRunningTasks(that), runningTask);
    } else {

      if (SquadGetFlagTextOMeter(that) == true) {
        streamBufferHistory = fmemopen(bufferHistory, 100, "w");
        SquidletInfoPrint(squidlet, streamBufferHistory);
        fclose(streamBufferHistory);
        sprintf(lineHistory, "couldn't send data to %s\n", 
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
      sprintf(lineHistory, "task refused by %s\n", 
        bufferHistory);
      SquadPushHistory(that, lineHistory);
    }

  }
  // Return the result
  return ret;
}

// Step the Squad 'that', i.e. tries to affect remaining tasks to 
// available Squidlet and check for completion of running task.
// Return a GSet of completed SquidletTaskRequest at this step
GSet SquadStep(Squad* const that) {
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
          sprintf(lineHistory, "task %s complete\n", 
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
          sprintf(lineHistory, "task %s gave up\n", 
            bufferHistory);
          SquadPushHistory(that, lineHistory);
        }

        // Put back the squidlet in the set of squidlets
        GSetAppend((GSet*)SquadSquidlets(that), runningTask->_squidlet);
        // Put back the task to the set of tasks
        GSetAppend((GSet*)SquadTasks(that), runningTask->_request);
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
          sprintf(lineHistory, "task %s still running\n", 
            bufferHistory);
          SquadPushHistory(that, lineHistory);
        }

      }
    } while (flag || GSetIterStep(&iter));
  }
  
  // If there are tasks to execute and available squidlet
  if (SquadGetNbTasks(that) > 0L && SquadGetNbSquidlets(that) > 0L) {
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

// Process the completed 'task' with the Squad 'that'
void SquadProcessCompletedTask(Squad* const that, 
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
void SquadProcessCompletedTask_PovRay(Squad* const that, 
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
  if (JSONLoadFromStr(jsonResult, task->_buffer) && 
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
          sprintf(lineHistory, "Couldn't read the fragment (%s)\n", 
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
          "Can't preprocess the Pov-Ray task (invalid data)\n");
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
void SquadSetFlagTextOMeter(Squad* const that, const bool flag) {
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
      int width = 150;
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
// 'msg' must be less than 100 characters long
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
  if (strlen(msg) >= 100) {
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
    "[%06u] %s", that->_countLineHistory, msg);
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
    SquadGetNbRunningTasks(that), SquadGetNbTasks(that),
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
  if (SquadGetNbTasks(that) > 0 &&
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
      iLine < SQUAD_TXTOMETER_NBTASKDISPLAYED);
  }
  if (iLine == SQUAD_TXTOMETER_NBTASKDISPLAYED) {
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
// task and display information
// about each on the 'stream'
// Return true if all the tasks could be performed, false else
bool SquadCheckSquidlets(Squad* const that, FILE* const stream) {
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
        SquidletTaskType_Dummy, 0, buffer, maxWait);
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
            fprintf(stream, "\t%s\n", request->_buffer);
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

// Run a benchmark with the squad 'that' and output the result on 
// the stream 'stream'. Squidlets definition must have been 
// previously loaded
void SquadBenchmark(Squad* const that, FILE* const stream) {
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
  fprintf(stream, "Execution on local device:\n");
  fprintf(stream, "nbLoopPerTask\tnbBytePayload\tnbTaskComp\ttimeMsPerTask\n");
  int lengthTest = 30;
  size_t maxSizePayload = 1024;
  int nbMaxLoop = 1024;
  char* buffer = PBErrMalloc(TheSquidErr, 27);
  for (size_t i = 0; i < 26; ++i)
    buffer[i] = 'a' + i;
  buffer[26] = 0;
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
      nbLoop, 1, nbComplete, timePerTaskMs);
    fflush(stdout);
  }
  free(buffer);

  fprintf(stream, "Execution on TheSquid:\n");
  fprintf(stream, "nbLoopPerTask\tnbBytePayload\tnbTaskComp\ttimeMsPerTask\n");

  // Loop on payload size
  time_t maxWait = 200;
  unsigned int id = 0;
  bool flagStop = false;
  for (size_t sizePayload = 1; !flagStop && 
    sizePayload <= maxSizePayload; sizePayload *= 10) {
    // Loop on nbLoop
    for (int nbLoop = 1; !flagStop && nbLoop <= nbMaxLoop; nbLoop *= 2) {

      // Loop during lengthTest seconds
      struct timeval stop, start;
      gettimeofday(&start, NULL);
      unsigned long nbComplete = 0;
      do {
        
        // Create benchmark tasks if there are no more
        while (SquadGetNbRunningTasks(that) + 
          SquadGetNbTasks(that) < SquadGetNbSquidlets(that)) {
          SquadAddTask_Benchmark(that, id++, maxWait, nbLoop, 
            sizePayload);
        }

        // Step the Squad
        GSet completedTasks = SquadStep(that);
        nbComplete += GSetNbElem(&completedTasks);
        while (GSetNbElem(&completedTasks) > 0L) {
          SquidletTaskRequest* task = GSetPop(&completedTasks);
          // If the task failed
          if (strstr(task->_buffer, "\"success\":\"1\"") == NULL) {
            SquidletTaskRequestPrint(task, stdout);
            fprintf(stream, " failed !!\n");
            fprintf(stream, "%s\n", task->_buffer);
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
          if (strstr(task->_buffer, "\"success\":\"1\"") == NULL) {
            SquidletTaskRequestPrint(task, stdout);
            fprintf(stream, " failed !!\n");
            fprintf(stream, "%s\n", task->_buffer);
            flagStop = true;
          }
          SquidletTaskRequestFree(&task);
        }
      } 
      gettimeofday(&stop, NULL);
      
      unsigned long deltams = (stop.tv_sec - start.tv_sec) * 1000 + 
        (stop.tv_usec - start.tv_usec) / 1000;
      float timePerTaskMs = (float) deltams / (float)nbComplete;
      fprintf(stream, "%04d\t%08u\t%07lu\t%011.2f\n", nbLoop, sizePayload, 
        nbComplete, timePerTaskMs);
      fflush(stdout);
    }
  }
  fprintf(stream, "-- Benchmark ended --\n");
  
}


// -------------- Squidlet

// ================= Global variable ==================

// Variable to handle the signal Ctrl-C
bool Squidlet_CtrlC = false;

// ================ Functions implementation ====================

// Handler for the signal Ctrl-C
void SquidletHandlerCtrlC(const int sig) {
  (void)sig;
  Squidlet_CtrlC = true;
  time_t intTime = time(NULL);
  char* strIntTime = ctime(&intTime);
  printf("Squidlet :  !!! Interrupted by Ctrl-C !!! %s", strIntTime);
  fflush(stdout);
}

// Handler for the signal SIGPIPE
void SquidletHandlerSigPipe(const int sig) {
  (void)sig;
  time_t intTime = time(NULL);
  char* strIntTime = ctime(&intTime);
  printf("Squidlet :  !!! Received SIGPIPE !!! %s", strIntTime);
  fflush(stdout);
}

// Return a new Squidlet listening to the ip 'addr' and port 'port'
// If 'addr' equals 0, select automatically the first network address 
// of the host 
// If 'port' equals -1, select automatically one available
Squidlet* SquidletCreateOnPort(const uint32_t addr, const int port) {
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
void SquidletFree(Squidlet** that) {
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
// 'stream'
// Example: 100 localhost 0.0.0.0:3000
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
  fprintf(stream, "%d %s %s:%d", SquidletGetPID(that), 
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
  // connections if there was one. If there are not wait wait for
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
void SquidletProcessRequest(Squidlet* const that, 
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
void SquidletProcessRequest_Dummy(Squidlet* const that,
  const char* const buffer, char** bufferResult) {
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
void SquidletProcessRequest_Benchmark(Squidlet* const that,
  const char* const buffer, char** bufferResult) {
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
void SquidletProcessRequest_PovRay(Squidlet* const that,
  const char* const buffer, char** bufferResult) {
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
// running and maybe null if the temperature is not availalble
char* SquidletGetTemperature(const Squidlet* const that) {
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
int TheSquidBenchmark(int nbLoop, const char* const buffer) {
  // Variable to memorize the dummy result
  int res = 0;
  // Loop on sample code
  for (int iLoop = 0; iLoop < nbLoop; ++iLoop) {
    GSet set = GSetCreateStatic();
    for (unsigned int inflation = 0; inflation < 1024; ++inflation) {
      for(unsigned long i = strlen(buffer); i--;) {
        GSetPush(&set, NULL);
        set._head->_sortVal = 
          (float)(buffer[(i + iLoop) % strlen(buffer)]);
      }
    }
    GSetSort(&set);
    res = (int)round(set._head->_sortVal);
    GSetFlush(&set);
  }
  // Return the dummy result
  return res;
}

// Function to receive in blocking mode 'nb' bytes of data from
// the socket 'sock' and store them into 'buffer' (which must be big 
// enough). Give up after 'timeout' seconds.
// Return true if we could read all the requested byte, false else
bool SocketRecv(short* sock, unsigned long nb, char* buffer, 
  int timeout) {
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
  } while (freadPtr != freadPtrEnd && timeout != 0 && 
    elapsedTime <= timeout && !Squidlet_CtrlC);
  *sock = dup(*sock);
  fclose(fp);

  // Return the success/failure code
  if (freadPtr != freadPtrEnd) {
    return false;
  } else {
    return true;
  }

} 

