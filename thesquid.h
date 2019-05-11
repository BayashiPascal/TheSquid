// ============ THESQUID.H ================

#ifndef THESQUID_H
#define THESQUID_H

// ================= Include =================

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include "pberr.h"
#include "pbmath.h"
#include "gset.h"
#include "respublish.h"
#include "pbjson.h"
#include "pbcextension.h"
#include "pbfilesys.h"

// ================= Define ===================

#define THESQUID_NBMAXPENDINGCONN 1
#define THESQUID_PORTMIN 9000
#define THESQUID_PORTMAX 9999
#define THESQUID_TASKREFUSED 0
#define THESQUID_TASKACCEPTED 1

// -------------- SquidletInfo

// ================= Data structure ===================

typedef struct SquidletInfo {
  // IP of the squidlet
  char* _ip;
  // Port of the squidlet
  int _port;
  // Socket to communicate with this squidlet
  short _sock;
} SquidletInfo;

// Return a new SquidletInfo with IP 'ip' and port 'port'
SquidletInfo* SquidletInfoCreate(char* ip, int port);

// Free the memory used by the SquidletInfo 'that'
void SquidletInfoFree(SquidletInfo** that);

// -------------- SquidletTaskRequest

// ================= Data structure ===================

typedef enum SquidletTaskType {
  SquidletTaskType_Null, SquidletTaskType_Dummy} SquidletTaskType;

typedef struct SquidletTaskRequest {
  // Task type
  SquidletTaskType _type;
  // Task ID
  unsigned long _id;
  // Data associated to the request, as a string in JSON format
  char* _data;
  // Buffer to receive the result from the squidlet
  char* _buffer;
} SquidletTaskRequest;

// Return a new SquidletTaskRequest with 'id' and 'type' and 'data'
SquidletTaskRequest* SquidletTaskRequestCreate(SquidletTaskType type, 
  unsigned long id, const char* const data);

// Free the memory used by the SquidletTaskRequest 'that'
void SquidletTaskRequestFree(SquidletTaskRequest** that);

// -------------- SquadRunningTask

// ================= Data structure ===================

typedef struct SquadRunningTask {
  // The task
  SquidletTaskRequest* _request;
  // The squidlet
  SquidletInfo* _squidlet;
} SquadRunningTask;

// Return a new SquadRunningTask with the 'request' and 'squidlet'
SquadRunningTask* SquadRunningTaskCreate(
  SquidletTaskRequest* const request, SquidletInfo* const squidlet);

// Free the memory used by the SquadRunningTask 'that'
void SquadRunningTaskFree(SquadRunningTask** that);

// -------------- Squad

// ================= Data structure ===================

typedef struct Squad {
  // File descriptor of the socket
  short _fd;
  // GSet of SquidletInfo
  GSet _squidlets;
  // GSet of SquidletTaskRequest (tasks to execute)
  GSet _tasks;
  // GSet of SquadRunningTask (tasks under execution)
  GSet _runningTasks;
} Squad;

// ================ Functions declaration ====================

// Return a new Squad
Squad* SquadCreate(void);

// Free the memory used by the Squad 'that'
void SquadFree(Squad** that);

// Get the set of squidlets of the Squad 'that'
#if BUILDMODE != 0 
inline 
#endif 
const GSet* SquadSquidlets(const Squad* const that);

// Get the set of task to execute of the Squad 'that'
#if BUILDMODE != 0 
inline 
#endif 
const GSet* SquadTasks(const Squad* const that);

// Get the set of running tasks of the Squad 'that'
#if BUILDMODE != 0 
inline 
#endif 
const GSet* SquadRunningTasks(const Squad* const that);

// Load the Squad info from the 'stream' into the 'that'
// Return true if it could load the info, else false
bool SquadLoad(Squad* const that, FILE* const stream);

// Send the task request 'request' from the Squad 'that' to its
// Squidlet 'squidlet'
// Return true if the request could be sent, false else
bool SquadSendTaskRequest(Squad* const that, 
  const SquidletTaskRequest* const request, SquidletInfo* const squidlet);

// Send the data associated to 'task' from the Squad 'that' to 
// the Squidlet 'squidlet'
// Return true if the data could be sent, false else
bool SquadSendTaskData(Squad* const that, 
  SquidletInfo* const squidlet, SquidletTaskRequest* const task);

// Receive the result from the running task 'runningTask', non blocking
// If the result is ready it is stored in the SquidletInfo
// Return true if we received the result, false else
bool SquadReceiveTaskResult(Squad* const that, 
  SquadRunningTask* const runningTask);

// Add a new dummy task with 'id' to execute to the squad 'that'
void SquadAddTask_Dummy(Squad* const that, const unsigned long id);
  
// Return the number of task not yet completed
#if BUILDMODE != 0 
inline 
#endif 
unsigned long SquadGetNbTaskToComplete(const Squad* const that);

// Return the number of running tasks
#if BUILDMODE != 0 
inline 
#endif 
unsigned long SquadGetNbRunningTasks(const Squad* const that);

// Return the number of tasks to execute
#if BUILDMODE != 0 
inline 
#endif 
unsigned long SquadGetNbTasks(const Squad* const that);

// Return the number of available squidlets
#if BUILDMODE != 0 
inline 
#endif 
unsigned long SquadGetNbSquidlets(const Squad* const that);

// Step the Squad 'that', i.e. tries to affect remaining tasks to 
// available Squidlet and check for completion of running task.
// Return a GSet of completed SquidletTaskRequest at this step
GSet SquadStep(Squad* const squad);

// -------------- Squidlet

// ================= Data structure ===================

typedef struct Squidlet {
  // File descriptor of the socket
  short _fd;
  // Port the squidlet is listening to
  int _port;
  // Info about the socket to receive task request
  struct sockaddr_in _sock;
  // PID of the process running the squidlet
  pid_t _pid;
  // Hostname
  char _hostname[256];
  // Information about the host
  struct hostent* _host; 
  // Socket to send the result of a task 
  int _sockReply;
} Squidlet;

// ================ Functions declaration ====================

// Return a new Squidlet listening to the port 'port'
// If 'port' equals -1 select automatically one available
Squidlet* SquidletCreateOnPort(int port);
#define SquidletCreate() SquidletCreateOnPort(-1)

// Free the memory used by the Squidlet 'that'
void SquidletFree(Squidlet** that);

// Print the PID, Hostname, IP and Port of the Squidlet 'that' on the 
// 'stream'
// Example: 100 localhost 0.0.0.0 3000
void SquidletPrint(const Squidlet* const that, FILE* const stream);

// Wait for a task request to be received by the Squidlet 'that'
// Return the received task request
SquidletTaskRequest SquidletWaitRequest(Squidlet* const that);

// Process the task request 'request' with the Squidlet 'that'
void SquidletProcessRequest(Squidlet* const that, 
  SquidletTaskRequest* const request);
  
// Process a dummy task request with the Squidlet 'that'
// This task only sleep for 2 seconds and serve only unit test purpose
void SquidletProcessRequest_Dummy(Squidlet* const that,
  SquidletTaskRequest* const request);
  
// Get the PID of the Squidlet 'that'
#if BUILDMODE != 0 
inline 
#endif 
pid_t SquidletGetPID(const Squidlet* const that);

// Get the hostname of the Squidlet 'that'
#if BUILDMODE != 0 
inline 
#endif 
const char* SquidletHostname(const Squidlet* const that);

// Get the IP of the Squidlet 'that'
#if BUILDMODE != 0 
inline 
#endif 
const char* SquidletIP(const Squidlet* const that);

// Get the port of the Squidlet 'that'
#if BUILDMODE != 0 
inline 
#endif 
int SquidletGetPort(const Squidlet* const that);

// ================ Polymorphism ====================
/*
#define FracNoiseGet(Noise, U) _Generic(U, \
  VecFloat*: _FracNoiseGet, \
  const VecFloat*: _FracNoiseGet, \
  VecFloat2D*: _FracNoiseGet, \
  const VecFloat2D*: _FracNoiseGet, \
  VecFloat3D*: _FracNoiseGet, \
  const VecFloat3D*: _FracNoiseGet, \
  default:PBErrInvalidPolymorphism) (Noise, (const VecFloat*)(U))
*/

// ================ Inliner ====================

#if BUILDMODE != 0
#include "thesquid-inline.c"
#endif

#endif
