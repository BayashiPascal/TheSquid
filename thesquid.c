// ============ THESQUID.C ================

// ================= Include =================

#include "thesquid.h"
#if BUILDMODE == 0
  #include "thesquid-inline.c"
#endif

// ================ Functions declaration ====================

// Function to receive in blocking mode 'nb' bytes of data from
// the socket 'sock' and store them into 'buffer' (which must be big 
// enough). Give up after 'sec' seconds.
// Return true if we could read all the requested byte, false else
bool SocketRecv(short* sock, unsigned long nb, char* buffer, int sec);

// -------------- SquidletInfo

// ================ Functions implementation ====================

// Return a new SquidletInfo with IP 'ip' and port 'port'
SquidletInfo* SquidletInfoCreate(char* ip, int port) {
  // Allocate memory for the squidletInfo
  SquidletInfo* that = PBErrMalloc(TheSquidErr, sizeof(SquidletInfo));
  
  // Init properties
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
  fprintf(stream, "%s:%d", that->_ip, that->_port);
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
  char lineHistory[100];
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
  char lineHistory[100];
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
  char lineHistory[100];
  char bufferHistory[100];
  FILE* streamBufferHistory = NULL;
  
  // Make sure the buffer to receive the task is empty
  if (task->_buffer != NULL) {
    free(task->_buffer);
    task->_buffer = NULL;
  }
  
  // Try to receive the size of the input data and give up immediately
  if (SocketRecv(&(squidlet->_sock), sizeof(size_t), 
    (char*)&sizeResultData, 1)) {

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

      // Send the acknowledgement of received result
      char ack = 1;
      int flags = 0;
      (void)send(squidlet->_sock, &ack, sizeof(char), flags);

      if (SquadGetFlagTextOMeter(that) == true) {
        streamBufferHistory = fmemopen(bufferHistory, 100, "w");
        SquidletInfoPrint(squidlet, streamBufferHistory);
        fclose(streamBufferHistory);
        sprintf(lineHistory, 
          "send ack of received result data to (%s)\n", 
          bufferHistory);
        SquadPushHistory(that, lineHistory);
      }

      // Allocate memory for the result data
      task->_buffer = PBErrMalloc(TheSquidErr, sizeResultData + 1);
      memset(task->_buffer, 0, sizeResultData + 1);
      
      // Wait to receive the result data with a time limit proportional
      // to the size of result data
      int timeLimit = 5 + (int)round((float)sizeResultData / 1000.0);
      if (!SocketRecv(&(squidlet->_sock), sizeResultData, 
        task->_buffer, timeLimit)) {
        // If we coudln't received the result data
        free(task->_buffer);
        task->_buffer = NULL;
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
  char lineHistory[100];
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
            // Remove the squidlet from the available squidlet
            flag = GSetIterRemoveElem(&iter);
          } else {

            if (SquadGetFlagTextOMeter(that) == true) {
              streamBufferHistory = fmemopen(bufferHistory, 100, "w");
              SquidletInfoPrint(squidlet, streamBufferHistory);
              fclose(streamBufferHistory);
              sprintf(lineHistory, "couldn't send data to %s\n", 
                bufferHistory);
              SquadPushHistory(that, lineHistory);
            }
            
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
        // If the squidlet refused the task or the data couldn't be sent
        if (!ret) {
          // Put back the task in the set
          GSetPush((GSet*)SquadTasks(that), task);
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

// Set the flag memorizing if the TextOMeter is displayed for
// the Squad 'that' to 'flag'
void SquadSetFlagTextOMeter(Squad* const that, const bool flag) {
#if BUILDMODE == 0
  if (that == NULL) {
    PBImgAnalysisErr->_type = PBErrTypeNullPointer;
    sprintf(PBImgAnalysisErr->_msg, "'that' is null");
    PBErrCatch(PBImgAnalysisErr);
  }
#endif
  // If the requested flag is different from the current flag;
  if (that->_flagTextOMeter != flag) {
    if (flag && that->_textOMeter == NULL) {
      char title[] = "Squad";
      int width = strlen(SQUAD_TXTOMETER_LINE1) + 1;
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
    PBImgAnalysisErr->_type = PBErrTypeNullPointer;
    sprintf(PBImgAnalysisErr->_msg, "'that' is null");
    PBErrCatch(PBImgAnalysisErr);
  }
  if (msg == NULL) {
    PBImgAnalysisErr->_type = PBErrTypeNullPointer;
    sprintf(PBImgAnalysisErr->_msg, "'msg' is null");
    PBErrCatch(PBImgAnalysisErr);
  }
  if (strlen(msg) >= 100) {
    PBImgAnalysisErr->_type = PBErrTypeInvalidArg;
    sprintf(PBImgAnalysisErr->_msg, "'msg' is too long (%d<100)",
      strlen(msg));
    PBErrCatch(PBImgAnalysisErr);
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
    PBImgAnalysisErr->_type = PBErrTypeNullPointer;
    sprintf(PBImgAnalysisErr->_msg, "'that' is null");
    PBErrCatch(PBImgAnalysisErr);
  }
  if (that->_textOMeter == NULL) {
    PBImgAnalysisErr->_type = PBErrTypeNullPointer;
    sprintf(PBImgAnalysisErr->_msg, "'that->_textOMeter' is null");
    PBErrCatch(PBImgAnalysisErr);
  }
#endif
  // Clear the TextOMeter
  TextOMeterClear(that->_textOMeter);
  // .........................
  char buffer[100];
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
  printf(" !!! Interrupted by Ctrl-C !!! %s", strIntTime);
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

  // Set the timeout for sending and receiving on this socket to 1 sec
  struct timeval tv;
  tv.tv_sec = 1;
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
  
  // Accept a connection
  
  if (that->_sockReply != -1)
    close(that->_sockReply);
  
  that->_sockReply = accept(that->_fd, (struct sockaddr *)&incomingSock,
    &incomingSockSize);

  // If the connection was accepted
  if (that->_sockReply >= 0) {
    if (SquidletStreamInfo(that)){
      SquidletPrint(that, SquidletStreamInfo(that));
      fprintf(SquidletStreamInfo(that), " : accepted connection\n");
    }
    
    // Set the timeout for sending and receiving on this socket to 1 sec
    struct timeval tv;
    tv.tv_sec = 1;
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

      // Receive the task type, give up after 5 seconds
      if (SocketRecv(&(that->_sockReply), sizeof(SquidletTaskType), 
        (char*)&taskRequest, 5)) {
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

  if (request->_type != SquidletTaskType_Null) {
    if (SquidletStreamInfo(that)){
      SquidletPrint(that, SquidletStreamInfo(that));
      fprintf(SquidletStreamInfo(that), " : process task\n");
    }
  }

  // Switch according to the request type
  switch (request->_type) {
    case SquidletTaskType_Dummy:
      SquidletProcessRequest_Dummy(that, request);
      break;
    case SquidletTaskType_Benchmark:
      SquidletProcessRequest_Benchmark(that, request);
      break;
    default:
      break;
  }

  // If there was a reply from the squidlet to the squad
  if (that->_sockReply >= 0) {

    // Close the output socket for this task after receiving the 
    // acknowledgement
    // Give up after 10s

    if (request->_type != SquidletTaskType_Null) {
      if (SquidletStreamInfo(that)){
        SquidletPrint(that, SquidletStreamInfo(that));
        fprintf(SquidletStreamInfo(that), 
          " : wait for acknowledgement from squad\n");
      }
    }

    char ack = 0;
    if (SocketRecv(&(that->_sockReply), 1, &ack, 10)) {
      if (request->_type != SquidletTaskType_Null) {
        if (SquidletStreamInfo(that)){
          SquidletPrint(that, SquidletStreamInfo(that));
          fprintf(SquidletStreamInfo(that), 
            " : received acknowledgement from squad\n");
        }
      }
    } else {
      if (request->_type != SquidletTaskType_Null) {
        if (SquidletStreamInfo(that)){
          SquidletPrint(that, SquidletStreamInfo(that));
          fprintf(SquidletStreamInfo(that), 
            " : couldn't receive acknowledgement from squad\n");
        }
      }
    }

    if (request->_type != SquidletTaskType_Null) {
      if (SquidletStreamInfo(that)){
        SquidletPrint(that, SquidletStreamInfo(that));
        fprintf(SquidletStreamInfo(that), 
          " : ready for next task\n");
      }
    }
  }
}

// Process a dummy task request with the Squidlet 'that'
void SquidletProcessRequest_Dummy(Squidlet* const that,
  SquidletTaskRequest* const request) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  (void)request;
  // Declare a variable to memorize if the process has been successful
  bool success = false;

  // Declare a variable to memorize the result of processing
  int result = 0; 

  // Declare a variable to memorize the size in byte of the input data
  size_t sizeInputData = 0;

  // Wait to receive the size of the input data with a time limit of 5s
  if (SocketRecv(&(that->_sockReply), sizeof(size_t), 
    (char*)&sizeInputData, 5)) {

    if (SquidletStreamInfo(that)){
      SquidletPrint(that, SquidletStreamInfo(that));
      fprintf(SquidletStreamInfo(that), 
        " : size input data %d\n", sizeInputData);
    }

    // Declare a buffer for the raw input data
    char* buffer = NULL;

    // If there are input data
    if (sizeInputData > 0) {

      // Allocate memory for the input data
      buffer = PBErrMalloc(TheSquidErr, sizeInputData + 1);
      memset(buffer, 0, sizeInputData + 1);
      
      // Wait to receive the input data with a time limit proportional
      // to the size of input data
      int timeLimit = 5 + (int)round((float)sizeInputData / 1000.0);
      if (!SocketRecv(&(that->_sockReply), sizeInputData, buffer, 
        timeLimit)) {
        // If we coudln't received the input data
        free(buffer);
        buffer = NULL;
      }
    }

    // If we could receive the expected data
    if (sizeInputData > 0 && buffer != NULL) {
      
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
      
      // Free memory
      free(buffer);
    }
  }

  // Prepare the result data as JSON
  char bufferResult[100];
  memset(bufferResult, 0, 100);
  if (success) {
    char* temperature = SquidletGetTemperature(that);
    sprintf(bufferResult, 
      "{\"success\":\"1\",\"v\":\"%d\",\"temperature\":\"%s\"}", 
      result, temperature);
    if (temperature != NULL)
      free(temperature);
  } else {
    char* temperature = SquidletGetTemperature(that);
    sprintf(bufferResult, "{\"success\":\"0\",\"temperature\":\"%s\"}",
      temperature);
    if (temperature != NULL)
      free(temperature);
  }

  // Send the task data size
  int flags = 0;
  size_t len = strlen(bufferResult);
  if (send(that->_sockReply, 
    (char*)&len, sizeof(size_t), flags) != -1) {

      if (SquidletStreamInfo(that)){
        SquidletPrint(that, SquidletStreamInfo(that));
        fprintf(SquidletStreamInfo(that), 
          " : wait for acknowledgement from squad\n");
      }

    char ack = 0;
    (void)SocketRecv(&(that->_sockReply), 1, &ack, 60);


    if (send(that->_sockReply, bufferResult, len, flags) != -1) {
      if (SquidletStreamInfo(that)){
        SquidletPrint(that, SquidletStreamInfo(that));
        fprintf(SquidletStreamInfo(that), 
          " : sent result %s\n", bufferResult);
      }
    }
  }

}  

// Process a benchmark task request with the Squidlet 'that'
void SquidletProcessRequest_Benchmark(Squidlet* const that,
  SquidletTaskRequest* const request) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  (void)request;
  // Declare a variable to memorize if the process has been successful
  bool success = false;
  
  // Declare a variable to store the error message if any
  char errMsg[200] = {0};

  // Declare a variable to memorize the size in byte of the input data
  size_t sizeInputData = 0;

  // Variable to memorize the result of the benchmark function
  int result = 0;

  // Wait to receive the size of the input data with a time limit of 5s
  if (SocketRecv(&(that->_sockReply), sizeof(size_t), 
    (char*)&sizeInputData, 5)) {

    // Declare a buffer for the raw input data
    char* buffer = NULL;

    // If there are input data
    if (sizeInputData > 0) {
      if (SquidletStreamInfo(that)){
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
          fprintf(SquidletStreamInfo(that), 
            " : received task data %s\n", buffer);
        }
      }
    }

    // If we could receive the expected data
    if (sizeInputData > 0 && buffer != NULL) {
      
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

  // Prepare the result data as JSON
  char bufferResult[100];
  memset(bufferResult, 0, 100);
  if (success) {
    sprintf(bufferResult, "{\"success\":\"1\", \"v\":\"%d\"}", result);
  } else {
    char* temperature = SquidletGetTemperature(that);
    sprintf(bufferResult, 
      "{\"success\":\"0\",\"temp\":\"%s\",\"err\":\"%s\"}",
      temperature, errMsg);
    if (temperature != NULL)
      free(temperature);
  }

  // Send the task data size
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
#if SQUIDLET_ARCH == 1
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
    // Return the result
    return strdup(output);
  }
#else
  return NULL;
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
// enough). Give up after 'sec' seconds.
// Return true if we could read all the requested byte, false else
bool SocketRecv(short* sock, unsigned long nb, char* buffer, int sec) {
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
  while (freadPtr != freadPtrEnd && elapsedTime <= sec && 
    !Squidlet_CtrlC) {
    // Try to read one more byte, if successful moves the pointer to
    // the next byte to read by one byte
    //ssize_t nbReadByte = read(sock, freadPtr, nb);
    ssize_t nbReadByte = fread(freadPtr, 1, nb, fp);
    if (nbReadByte > 0) {
      freadPtr += nbReadByte;
    }
    // Update the elpased time
    elapsedTime = time(NULL) - startTime;
  }

  

  *sock = dup(*sock);
  fclose(fp);

  // Return the success/failure code
  if (freadPtr != freadPtrEnd) {
    return false;
  } else {
    return true;
  }

} 

