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
#include <time.h>
#include <sys/time.h>
#include "pberr.h"
#include "pbmath.h"
#include "gset.h"
#include "respublish.h"
#include "pbjson.h"
#include "pbcextension.h"
#include "pbfilesys.h"
#include "genbrush.h"

// ================= Define ===================

#define THESQUID_NBMAXPENDINGCONN 1
#define THESQUID_PORTMIN 9000
#define THESQUID_PORTMAX 9999
#define THESQUID_TASKREFUSED 0
#define THESQUID_TASKACCEPTED 1
#define THESQUID_ACCEPT_TIMEOUT 1 // in seconds
#define THESQUID_PROC_TIMEOUT 60 // in seconds
#define THESQUID_MAXSIZEPOVRAYFRAGMENT 1000

// -------------- SquidletInfo

// ================= Data structure ===================

typedef struct SquidletInfo {
  // Name of the squidlet
  char* _name;
  // IP of the squidlet
  char* _ip;
  // Port of the squidlet
  int _port;
  // Socket to communicate with this squidlet
  short _sock;
} SquidletInfo;

// Return a new SquidletInfo with 'name', 'ip' and 'port'
SquidletInfo* SquidletInfoCreate(char* name, char* ip, int port);

// Free the memory used by the SquidletInfo 'that'
void SquidletInfoFree(SquidletInfo** that);

// Print the SquidletInfo 'that' on the 'stream'
void SquidletInfoPrint(const SquidletInfo* const that, 
  FILE* const stream);

// -------------- SquidletTaskRequest

// ================= Data structure ===================

typedef enum SquidletTaskType {
  SquidletTaskType_Null, SquidletTaskType_Dummy, 
  SquidletTaskType_Benchmark, SquidletTaskType_PovRay} SquidletTaskType;

typedef struct SquidletTaskRequest {
  // Task type
  SquidletTaskType _type;
  // Task ID
  unsigned long _id;
  // Data associated to the request, as a string in JSON format
  char* _data;
  // Buffer to receive the result from the squidlet, as a string in 
  // JSON format
  char* _buffer;
  // Time in second after which we give up waiting for this task
  time_t _maxWaitTime;
} SquidletTaskRequest;

// Return a new SquidletTaskRequest with 'id' and 'type' and 'data'
SquidletTaskRequest* SquidletTaskRequestCreate(SquidletTaskType type, 
  unsigned long id, const char* const data, const time_t maxWait);

// Free the memory used by the SquidletTaskRequest 'that'
void SquidletTaskRequestFree(SquidletTaskRequest** that);

// Print the SquidletTaskRequest 'that' on the 'stream'
void SquidletTaskRequestPrint(const SquidletTaskRequest* const that, 
  FILE* const stream);

// -------------- SquadRunningTask

// ================= Data structure ===================

typedef struct SquadRunningTask {
  // The task
  SquidletTaskRequest* _request;
  // The squidlet
  SquidletInfo* _squidlet;
  // Time when the SquadRunningTask is created
  time_t _startTime;
} SquadRunningTask;

// Return a new SquadRunningTask with the 'request' and 'squidlet'
SquadRunningTask* SquadRunningTaskCreate(
  SquidletTaskRequest* const request, SquidletInfo* const squidlet);

// Free the memory used by the SquadRunningTask 'that'
void SquadRunningTaskFree(SquadRunningTask** that);

// Print the SquadRunningTask 'that' on the 'stream'
void SquadRunningTaskPrint(const SquadRunningTask* const that, 
  FILE* const stream);

// -------------- Squad

// ================= Define ==================

#define SQUAD_TXTOMETER_LINE1 \
  "NbRunning xxxxx NbQueued xxxxx NbSquidletAvail xxxxx                          \n"
#define SQUAD_TXTOMETER_FORMAT1 \
  "NbRunning %05ld NbQueued %05ld NbSquidletAvail %05ld\n"
#define SQUAD_TXTOMETER_FORMATHISTORY "%s\n"
#define SQUAD_TXTOMETER_TASKHEADER \
  "------------------    Tasks    ---------------------                          \n"
#define SQUAD_TXTOMETER_FORMATRUNNING "Running: %s\n"
#define SQUAD_TXTOMETER_FORMATQUEUED  " Queued: %s\n"
#define SQUAD_TXTOMETER_NBLINEHISTORY 20
#define SQUAD_TXTOMETER_NBTASKDISPLAYED 32

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
  // Flag to memorize if we display info with a TextOMeter
  bool _flagTextOMeter;
  // TextOMeter to display info 
  TextOMeter* _textOMeter;
  // Buffer internally used to display info in the TextOMeter
  char _history[SQUAD_TXTOMETER_NBLINEHISTORY][200];
  // Counter for history
  unsigned int _countLineHistory;
} Squad;

// ================ Functions declaration ====================

// Return a new Squad
Squad* SquadCreate(void);

// Free the memory used by the Squad 'that'
void SquadFree(Squad** that);

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
bool SquadLoadTasks(Squad* const that, FILE* const stream);

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

// Load the Squidlet info from the 'stream' into the 'that'
// Return true if it could load the info, else false
bool SquadLoadSquidlets(Squad* const that, FILE* const stream);

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
// Wait for a maximum of 'maxWait' seconds for the task to complete
void SquadAddTask_Dummy(Squad* const that, const unsigned long id,
  const time_t maxWait);
  
// Add a new benchmark task with 'id' to execute to the squad 'that'
// Wait for a maximum of 'maxWait' seconds for the task to complete
// Uses a payload of 'payloadSize' bytes
void SquadAddTask_Benchmark(Squad* const that, const unsigned long id,
  const time_t maxWait, const int nb, const size_t payloadSize);
  
// Add a new Pov-Ray task with 'id' to execute to the squad 'that'
// Render the scene described in the Pov-Ray ini file 'ini'
// The output format of the image must be TGA
// Wait for a maximum of 'maxWait' seconds for the task to complete
// The total size of the data must be less than 1024 bytes
// The random generator must have been initialised before calling this 
// function
void SquadAddTask_PovRay(Squad* const that, const unsigned long id,
  const time_t maxWait, const char* const ini, 
  const unsigned int sizeMinFragment,
  const unsigned int sizeMaxFragment);
  
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

// Return the number of currently available squidlets
#if BUILDMODE != 0 
inline 
#endif 
unsigned long SquadGetNbSquidlets(const Squad* const that);

// Step the Squad 'that', i.e. tries to affect remaining tasks to 
// available Squidlet and check for completion of running task.
// Return a GSet of completed SquidletTaskRequest at this step
GSet SquadStep(Squad* const squad);

// Process the completed 'task' with the Squad 'that'
void SquadProcessCompletedTask(Squad* const that, 
  SquidletTaskRequest* const task);

// Process the completed Pov-Ray 'task' with the Squad 'that'
void SquadProcessCompletedTask_PovRay(Squad* const that, 
  SquidletTaskRequest* const task);
  
// Set the flag memorizing if the TextOMeter is displayed for
// the Squad 'that' to 'flag'
void SquadSetFlagTextOMeter(Squad* const that, const bool flag);

// Return the flag for the TextOMeter of the Squad 'that'
#if BUILDMODE != 0
inline
#endif
bool SquadGetFlagTextOMeter(const Squad* const that);

// Check all the squidlets of the Squad 'that' by processing a dummy 
// task and display information
// about each on the 'stream'
// Return true if all the tasks could be performed, false else
bool SquadCheckSquidlets(Squad* const that, FILE* const stream);

// Run the benchmark with the squad 'that' and output the result on 
// the stream 'stream'
void SquadBenchmark(Squad* const that, FILE* const stream);

// -------------- Squidlet

// ================= Global variable ==================

// Variable to handle the signal Ctrl-C
extern bool Squidlet_CtrlC;

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
  short _sockReply;
  // Stream to ouput infos, if null the squidlet is silent
  // By default it's null
  FILE* _streamInfo; 
} Squidlet;

// ================ Functions declaration ====================

// Handler for the signal Ctrl-C
void SquidletHandlerCtrlC(const int sig);

// Handler for the signal SIGPIPE
void SquidletHandlerSigPipe(const int sig);

// Return a new Squidlet listening to the ip 'addr' and port 'port'
// If 'addr' equals 0, select automatically the first network address 
// of the host 
// If 'port' equals -1, select automatically one available
Squidlet* SquidletCreateOnPort(const uint32_t addr, const int port);
#define SquidletCreate() SquidletCreateOnPort(0, -1)

// Free the memory used by the Squidlet 'that'
void SquidletFree(Squidlet** that);

// Print the PID, Hostname, IP and Port of the Squidlet 'that' on the 
// 'stream'
// Example: 100 localhost 0.0.0.0:3000
void SquidletPrint(const Squidlet* const that, FILE* const stream);

// Wait for a task request to be received by the Squidlet 'that'
// Return the received task request
SquidletTaskRequest SquidletWaitRequest(Squidlet* const that);

// Process the task request 'request' with the Squidlet 'that'
void SquidletProcessRequest(Squidlet* const that, 
  SquidletTaskRequest* const request);
  
// Process a dummy task request with the Squidlet 'that'
void SquidletProcessRequest_Dummy(Squidlet* const that,
  const char* const buffer, char** bufferResult);
  
// Process a benchmark task request with the Squidlet 'that'
void SquidletProcessRequest_Benchmark(Squidlet* const that,
  const char* const buffer, char** bufferResult);
  
// Process a Pov-Ray task request with the Squidlet 'that'
void SquidletProcessRequest_PovRay(Squidlet* const that,
  const char* const buffer, char** bufferResult);
  
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

// Get the stream to output info of the Squidlet 'that'
#if BUILDMODE != 0 
inline 
#endif 
FILE* SquidletStreamInfo(const Squidlet* const that);

// Set the stream to output info of the Squidlet 'that' to 'stream'
// 'stream' may be null to mute the Squidlet
#if BUILDMODE != 0 
inline 
#endif 
void SquidletSetStreamInfo(Squidlet* const that, FILE* const stream);

// Return the temperature of the squidlet 'that' as a string.
// The result depends on the architecture on which the squidlet is 
// running and maybe null if the temperature is not availalble
char* SquidletGetTemperature(const Squidlet* const that);

// -------------- TheSquid 

// ================ Functions declaration ====================

// Function for benchmark purpose
int TheSquidBenchmark(int nbLoop, const char* const buffer);

// ================ Inliner ====================

#if BUILDMODE != 0
#include "thesquid-inline.c"
#endif

#endif
