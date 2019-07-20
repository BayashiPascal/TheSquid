// ============ THESQUID-INLINE.C ================

// -------------- SquidletInfo

// ================ Functions implementation ====================

// Return the stats of the SquidletInfo 'that'
#if BUILDMODE != 0 
inline 
#endif 
const SquidletInfoStats* SquidletInfoStatistics(
  const SquidletInfo* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return &(that->_stats);
}

// -------------- SquidletTaskRequest

// ================ Functions implementation ====================

// Return true if the SquidletTask 'that' has succeeded, else false
// The task is considered to have succeeded if its result buffer 
// contains "success":"1"
#if BUILDMODE != 0 
inline 
#endif 
bool SquidletTaskHasSucceeded(
  const SquidletTaskRequest* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return (that->_bufferResult != NULL && 
    strstr(that->_bufferResult, "\"success\":\"1\"") != NULL);
}

// Return the type of the task 'that'
#if BUILDMODE != 0 
inline 
#endif 
SquidletTaskType SquidletTaskGetType(
  const SquidletTaskRequest* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return that->_type;
}

// Return the id of the task 'that'
#if BUILDMODE != 0 
inline 
#endif 
unsigned long SquidletTaskGetId(
  const SquidletTaskRequest* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return that->_id;
}

// Return the subid of the task 'that'
#if BUILDMODE != 0 
inline 
#endif 
unsigned long SquidletTaskGetSubId(
  const SquidletTaskRequest* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return that->_subId;
}

// Return the data of the task 'that'
#if BUILDMODE != 0 
inline 
#endif 
const char* SquidletTaskData(
  const SquidletTaskRequest* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return that->_data;
}

// Return the buffer result of the task 'that'
#if BUILDMODE != 0 
inline 
#endif 
const char* SquidletTaskBufferResult(
  const SquidletTaskRequest* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return that->_bufferResult;
}

// Return the max wait time of the task 'that'
#if BUILDMODE != 0 
inline 
#endif 
time_t SquidletTaskGetMaxWaitTime(
  const SquidletTaskRequest* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return that->_maxWaitTime;
}

// -------------- Squad

// ================ Functions implementation ====================

// Get the set of squidlets of the Squad 'that'
#if BUILDMODE != 0 
inline 
#endif 
const GSetSquidletInfo* SquadSquidlets(
  const Squad* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return &(that->_squidlets);  
}

// Get the set of task to execute of the Squad 'that'
#if BUILDMODE != 0 
inline 
#endif 
const GSetSquidletTaskRequest* SquadTasks(
  const Squad* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return &(that->_tasks);  
}

// Get the set of running tasks of the Squad 'that'
#if BUILDMODE != 0 
inline 
#endif 
const GSetSquadRunningTask* SquadRunningTasks(
  const Squad* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return &(that->_runningTasks);  
}

// Return the number of tasks not yet completed
#if BUILDMODE != 0 
inline 
#endif 
unsigned long SquadGetNbTaskToComplete(
  const Squad* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return GSetNbElem(SquadTasks(that)) + 
    GSetNbElem(SquadRunningTasks(that));  
}

// Return the number of running tasks
#if BUILDMODE != 0 
inline 
#endif 
unsigned long SquadGetNbRunningTasks(
  const Squad* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return GSetNbElem(SquadRunningTasks(that));  
}

// Return the number of tasks to execute
#if BUILDMODE != 0 
inline 
#endif 
unsigned long SquadGetNbRemainingTasks(
  const Squad* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return GSetNbElem(SquadTasks(that));  
}

// Return the number of currently available squidlets (squidlets not 
// executing a task for 'that')
#if BUILDMODE != 0 
inline 
#endif 
unsigned long SquadGetNbSquidlets(
  const Squad* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return GSetNbElem(SquadSquidlets(that));  
}

// Return the flag for the TextOMeter of the Squad 'that'
#if BUILDMODE != 0
inline
#endif
bool SquadGetFlagTextOMeter(
  const Squad* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return that->_flagTextOMeter;
}


// -------------- Squidlet

// ================ Functions implementation ====================

// Get the PID of the Squidlet 'that'
#if BUILDMODE != 0 
inline 
#endif 
pid_t SquidletGetPID(
  const Squidlet* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return that->_pid;  
}

// Get the hostname of the Squidlet 'that'
#if BUILDMODE != 0 
inline 
#endif 
const char* SquidletHostname(
  const Squidlet* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return that->_hostname;
}

// Get the IP of the Squidlet 'that'
#if BUILDMODE != 0 
inline 
#endif 
const char* SquidletIP(
  const Squidlet* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return inet_ntoa(that->_sock.sin_addr);
}

// Get the port of the Squidlet 'that'
#if BUILDMODE != 0 
inline 
#endif 
int SquidletGetPort(
  const Squidlet* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return that->_port;  
}

// Get the stream to output info of the Squidlet 'that'
#if BUILDMODE != 0 
inline 
#endif 
FILE* SquidletStreamInfo(
  const Squidlet* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  return that->_streamInfo;  
}

// Set the stream to output info of the Squidlet 'that' to 'stream'
// 'stream' may be null to mute the Squidlet
#if BUILDMODE != 0 
inline 
#endif 
void SquidletSetStreamInfo(
  Squidlet* const that, 
      FILE* const stream) {
#if BUILDMODE == 0
  if (that == NULL) {
    TheSquidErr->_type = PBErrTypeNullPointer;
    sprintf(TheSquidErr->_msg, "'that' is null");
    PBErrCatch(TheSquidErr);
  }
#endif
  that->_streamInfo = stream;  
}



