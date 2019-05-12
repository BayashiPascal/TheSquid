// ============ THESQUID.C ================

// ================= Include =================

#include "thesquid.h"
#if BUILDMODE == 0
  #include "thesquid-inline.c"
#endif

// ================ Functions definition ====================

// Function to receive in blocking mode 'nb' bytes of data from
// the socket 'sock' and store them into 'buffer' (which must be big 
// enough). Give up after 'sec' seconds.
// Return true if we could read all the requested byte, false else
bool SocketRecv(short sock, unsigned long nb, char* buffer, int sec);

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
  that->_tasks = GSetCreateStatic();
  that->_runningTasks = GSetCreateStatic();

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
  
  // Create a socket
  squidlet->_sock = socket(AF_INET, SOCK_STREAM, 0);
  
  // If we couldn't create the socket
  if (squidlet->_sock == -1)
    return false;
  
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
    return false;
  }

printf("squad : connected to squidlet\n");

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
    return false;
  }

printf("squad : set timeout\n");

  // Send the task request
  int flags = 0;
  if (send(squidlet->_sock, 
    request, sizeof(SquidletTaskRequest), flags) == -1) {
    // If we couldn't send the request
    close(squidlet->_sock);
    squidlet->_sock = -1;
    return false;
  }

printf("squad : sent task request\n");

  // Wait for the reply from the squidlet up to 5s
  char reply = THESQUID_TASKREFUSED;
  if (!SocketRecv(squidlet->_sock, sizeof(reply), &reply, 5) ||
    reply == THESQUID_TASKREFUSED) {
    // If we couldn't receive the reply or the reply timed out or
    // the squidlet refused the task
    close(squidlet->_sock);
    squidlet->_sock = -1;
    return false;
  }

printf("squad : task request accepted\n");

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
  // Send the task data size
  int flags = 0;
  size_t len = strlen(task->_data);
  if (send(squidlet->_sock, 
    (char*)&len, sizeof(size_t), flags) == -1) {
    // If we couldn't send the data size
    return false;
  }

printf("squad : sent task data size %d\n", len);

  // Send the task data
  if (send(squidlet->_sock, task->_data, len, flags) == -1) {
    // If we couldn't send the data
    return false;
  }

printf("squad : sent task data %s\n", task->_data);

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

  // Make sure the buffer to receive the task is empty
  if (task->_buffer != NULL) {
    free(task->_buffer);
    task->_buffer = NULL;
  }
  
  // Try to receive the size of the input data and give up immediately
  if (SocketRecv(squidlet->_sock, sizeof(size_t), 
    (char*)&sizeResultData, 0)) {

    // Send the acknowledgement of received result
    char ack = 1;
    int flags = 0;
    (void)send(squidlet->_sock, &ack, 1, flags);

    // If the result is ready
    if (sizeResultData > 0) {
printf("squad : size result data %d\n", sizeResultData);

      // Allocate memory for the result data
      task->_buffer = PBErrMalloc(TheSquidErr, sizeResultData + 1);
      memset(task->_buffer, 0, sizeResultData + 1);
      
      // Wait to receive the result data with a time limit proportional
      // to the size of result data
      int timeLimit = 5 + (int)round((float)sizeResultData / 1000.0);
      if (!SocketRecv(squidlet->_sock, sizeResultData, 
        task->_buffer, timeLimit)) {
        // If we coudln't received the result data
        free(task->_buffer);
        task->_buffer = NULL;
      } else {
        receivedFlag = true;
        
        // Send the acknowledgement of received result
        (void)send(squidlet->_sock, &ack, 1, flags);
        
printf("squad : received result data %s\n", task->_buffer);
      }
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
            GSetAppend((GSet*)SquadRunningTasks(that), runningTask);
            // Remove the squidlet from the available squidlet
            flag = GSetIterRemoveElem(&iter);
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
  
  // Return the set of completed tasks
  return completedTasks;
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
    return NULL;
  }

  // Get the hostname
  if (gethostname(that->_hostname, sizeof(that->_hostname)) == -1) {
    // If we couldn't get the hostname, free memory and 
    // return null
    close(that->_fd);
    free(that);
    return NULL;
  }
  
  // Get the info about the host
  that->_host = gethostbyname(that->_hostname); 
  if (that->_host == NULL) {
    // If we couldn't get the host info, free memory and 
    // return null
    close(that->_fd);
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
  
  // If the port is not specified by the user, bind the socket on the 
  // first available port
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
  // Else if the port is specified by the user, try to bind only on 
  // that port
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

  // Init the socket for reply
  that->_sockReply = -1;

  // Set the handler to catch the signal Ctrl-C
  signal(SIGINT, SquidletHandlerCtrlC);
  
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
  // Declare a variable to memorize the reply to the request
  char reply = THESQUID_TASKREFUSED;

  // Declare a buffer to receive the payload of the connection
  SquidletTaskRequest taskRequest;
  taskRequest._type = SquidletTaskType_Null;

  // Declare a variable to memorize the info about the incoming connection
  struct sockaddr_in incomingSock;
  socklen_t incomingSockSize = sizeof(incomingSock);
  
  // Accept a connection
  that->_sockReply = accept(that->_fd, (struct sockaddr *)&incomingSock,
    &incomingSockSize);

  // If the connection was accepted
  if (that->_sockReply >= 0) {

SquidletPrint(that, stdout);
printf(" : accepted connection\n");
  
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
    } else {

      // Receive the task id, give up after 5 seconds
      if (SocketRecv(that->_sockReply, sizeof(SquidletTaskType), 
        (char*)&taskRequest, 5)) {
        reply = THESQUID_TASKACCEPTED;

SquidletPrint(that, stdout);
printf(" : received task type %d\n", taskRequest._type);

      } else {
        // If we couldn't received the task id
        taskRequest._type = SquidletTaskType_Null;
        reply = THESQUID_TASKREFUSED;
      }

      // Send the reply to the task request
      int flags = 0;
      if (send(that->_sockReply, &reply, sizeof(reply), flags) == -1) {
        // If we couldn't send the reply, do not process the task
        taskRequest._type = SquidletTaskType_Null;
      }

SquidletPrint(that, stdout);
printf(" : sent reply to task request %d\n", reply);
    
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

  // Switch according to the request type
  switch (request->_type) {
    case SquidletTaskType_Dummy:
      SquidletProcessRequest_Dummy(that, request);
      break;
    default:
      break;
  }

  // If there was a reply from the squidlet to the squad
  if (that->_sockReply >= 0) {

    // Close the output socket for this task after receiving the 
    // acknowledgement
    // Give up after 60s

SquidletPrint(that, stdout);
printf(" : wait for acknowledgement\n");

    char ack = 0;
    (void)SocketRecv(that->_sockReply, 1, &ack, 60);

    close(that->_sockReply);
    that->_sockReply = -1;

SquidletPrint(that, stdout);
printf(" : ready for next task\n");

  }
}

// Process a dummy task request with the Squidlet 'that'
// This task only sleep for 2 seconds and serve only unit test purpose
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

  // Declare a varibale to memorize the result of processing
  int result = 0; 

  // Declare a variable to memorize the size in byte of the input data
  size_t sizeInputData = 0;

  // Wait to receive the size of the input data with a time limit of 5s
  if (SocketRecv(that->_sockReply, sizeof(size_t), 
    (char*)&sizeInputData, 5)) {

SquidletPrint(that, stdout);
printf(" : size input data %d\n", sizeInputData);

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
      if (!SocketRecv(that->_sockReply, sizeInputData, buffer, 
        timeLimit)) {
        // If we coudln't received the input data
        free(buffer);
        buffer = NULL;
      }
    }

    // If we could receive the expected data
    if (sizeInputData == 0 || buffer != NULL) {
      
      // Process the data
SquidletPrint(that, stdout);
printf(" : process dummy task %s\n", buffer);

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
    sprintf(bufferResult, "{\"success\":\"1\",\"v\":\"%d\"}", result);
  } else {
    sprintf(bufferResult, "{\"success\":\"0\"}");
  }

  // Send the task data size
  int flags = 0;
  size_t len = strlen(bufferResult);
  if (send(that->_sockReply, 
    (char*)&len, sizeof(size_t), flags) != -1) {

SquidletPrint(that, stdout);
printf(" : wait for acknowledgement\n");

    char ack = 0;
    (void)SocketRecv(that->_sockReply, 1, &ack, 60);


    if (send(that->_sockReply, bufferResult, len, flags) != -1) {
SquidletPrint(that, stdout);
printf(" : sent result %s\n", bufferResult);
    }
  }

}  

// Function to receive in blocking mode 'nb' bytes of data from
// the socket 'sock' and store them into 'buffer' (which must be big 
// enough). Give up after 'sec' seconds.
// Return true if we could read all the requested byte, false else
bool SocketRecv(short sock, unsigned long nb, char* buffer, int sec) {
#if BUILDMODE == 0
  if (buffer == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'buffer' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  // Convert the socket (file descriptor) to a FILE pointer
  FILE* fp = fdopen(sock, "r");

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
    // Try to read one more byte, if successfuul moves the pointer to
    // the next byte to read by one byte
    freadPtr += fread(freadPtr, 1, 1, fp);
    // Update the elpased time
    elapsedTime = time(NULL) - startTime;
  }

  // Return the success/failure code
  if (freadPtr != freadPtrEnd) {
    return false;
  } else {
    return true;
  }

} 
