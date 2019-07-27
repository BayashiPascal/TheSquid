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
#define THESQUID_PORTMIN          9000
#define THESQUID_PORTMAX          9999
#define THESQUID_TASKREFUSED      0
#define THESQUID_TASKACCEPTED     1
#define THESQUID_ACCEPT_TIMEOUT   1    // in seconds
#define THESQUID_PROC_TIMEOUT     60   // in seconds
#define THESQUID_MAXPAYLOADSIZE   1024 // bytes

#define SQUAD_TXTOMETER_LINE1             \
  "NbRunning xxxxx NbQueued xxxxx NbSquidletAvail xxxxx\n"
#define SQUAD_TXTOMETER_FORMAT1           \
  "NbRunning %05ld NbQueued %05ld NbSquidletAvail %05ld\n"
#define SQUAD_TXTOMETER_FORMATHISTORY     "%s\n"
#define SQUAD_TXTOMETER_TASKHEADER        \
  "------------------    Tasks    ---------------------\n"
#define SQUAD_TXTOMETER_FORMATRUNNING     "Running: %s\n"
#define SQUAD_TXTOMETER_FORMATQUEUED      " Queued: %s\n"
#define SQUAD_TXTOMETER_NBLINEHISTORY     20
#define SQUAD_TXTOMETER_LENGTHLINEHISTORY 100
#define SQUAD_TXTOMETER_NBTASKDISPLAYED   32

// Range for the sliding average when computing stats
#define SQUID_RANGEAVGSTAT 100

// -------------- SquidletInfo

// ================= Data structure ===================

typedef struct SquidletInfoStats {
  unsigned long _nbAcceptedConnection;
  unsigned long _nbAcceptedTask;
  unsigned long _nbRefusedTask;
  unsigned long _nbFailedReceptTaskData;
  unsigned long _nbFailedReceptTaskSize;
  unsigned long _nbSentResult;
  unsigned long _nbFailedSendResult;
  unsigned long _nbFailedSendResultSize;
  unsigned long _nbFailedReceptAck;
  unsigned long _nbTaskComplete;
  float _timeToProcessMs[3];
  float _timeWaitedTaskMs[3];
  float _timeWaitedAckMs[3];
  float _temperature[3];
  float _timeTransferSquadSquidMs[3];
  float _timeTransferSquidSquadMs[3];
  float _timePerTask;
  float _nbTaskExpected;
} SquidletInfoStats;

typedef struct SquidletInfo {
  // Name of the squidlet
  char* _name;
  // IP of the squidlet
  char* _ip;
  // Port of the squidlet
  int _port;
  // Socket to communicate with this squidlet
  short _sock;
  // Statistics
  SquidletInfoStats _stats;
} SquidletInfo;

// ================ Functions declaration ====================

// Return a new SquidletInfo describing a Squidlet whose name is 
// 'name', and whose attached to the address 'ip':'port'
SquidletInfo* SquidletInfoCreate(
  const char* const name, 
  const char* const ip, 
    const int const port);

// Free the memory used by the SquidletInfo 'that'
void SquidletInfoFree(
  SquidletInfo** that);

// Print the SquidletInfo 'that' on the file 'stream'
void SquidletInfoPrint(
  const SquidletInfo* const that, 
                FILE* const stream);

// Print the SquidletInfoStats 'that' on the file 'stream'
void SquidletInfoStatsPrintln(
  const SquidletInfoStats* const that, 
                     FILE* const stream);

// Init the stats of the SquidletInfoStats 'that' 
void SquidletInfoStatsInit(
  SquidletInfoStats* const that);

// Return the stats of the SquidletInfo 'that'
#if BUILDMODE != 0 
inline 
#endif 
const SquidletInfoStats* SquidletInfoStatistics(
  const SquidletInfo* const that);

// Update the statistics about the transfer time of the SquidletInfoStats
// 'that' given that it took 'deltams' millisecond to send 'len' bytes
// of data
void SquidletInfoStatsUpdateTimeTransfer(
  SquidletInfoStats* const that,
               const float deltams,
              const size_t len);

// -------------- SquidletTaskRequest

// ================= Data structure ===================

typedef enum SquidletTaskType {
  SquidletTaskType_Null, 
  SquidletTaskType_Dummy, 
  SquidletTaskType_Benchmark, 
  SquidletTaskType_PovRay,
  SquidletTaskType_ResetStats} SquidletTaskType;

typedef struct SquidletTaskRequest {
  // Task type
  SquidletTaskType _type;
  // Task ID
  unsigned long _id;
  // Task sub ID
  unsigned long _subId;
  // Data associated to the request, as a string in JSON format
  char* _data;
  // Buffer to receive the result from the squidlet, as a string in 
  // JSON format
  char* _bufferResult;
  // Time in second after which the Squad give up waiting for the
  // completion of this task
  time_t _maxWaitTime;
} SquidletTaskRequest;

// ================ Functions declaration ====================

// Return a new SquidletTaskRequest for a task of type 'type'
// The task is identified by its 'id'/'subId', it will have at
// maximum 'maxWait' seconds to complete. It holds a copy of 'data', a 
// string in JSON format
SquidletTaskRequest* SquidletTaskRequestCreate(
   SquidletTaskType type, 
      unsigned long id, 
      unsigned long subId, 
  const char* const data, 
       const time_t maxWait);

// Free the memory used by the SquidletTaskRequest 'that'
void SquidletTaskRequestFree(
  SquidletTaskRequest** that);

// Print the SquidletTaskRequest 'that' on the file 'stream'
// Only a maximum of 100 first characters of the data are printed
void SquidletTaskRequestPrint(
  const SquidletTaskRequest* const that, 
                       FILE* const stream);

// Return true if the SquidletTask 'that' has succeeded, else false
// The task is considered to have succeeded if its result buffer 
// contains "success":"1"
#if BUILDMODE != 0 
inline 
#endif 
bool SquidletTaskHasSucceeded(
  const SquidletTaskRequest* const that);

// Return the type of the task 'that' as a string
const char* SquidletTaskTypeAsStr(
  const SquidletTaskRequest* const that);

// Return the type of the task 'that'
#if BUILDMODE != 0 
inline 
#endif 
SquidletTaskType SquidletTaskGetType(
  const SquidletTaskRequest* const that);

// Return the id of the task 'that'
#if BUILDMODE != 0 
inline 
#endif 
unsigned long SquidletTaskGetId(
  const SquidletTaskRequest* const that);

// Return the subid of the task 'that'
#if BUILDMODE != 0 
inline 
#endif 
unsigned long SquidletTaskGetSubId(
  const SquidletTaskRequest* const that);

// Return the data of the task 'that'
#if BUILDMODE != 0 
inline 
#endif 
const char* SquidletTaskData(
  const SquidletTaskRequest* const that);

// Return the buffer result of the task 'that'
#if BUILDMODE != 0 
inline 
#endif 
const char* SquidletTaskBufferResult(
  const SquidletTaskRequest* const that);

// Return the max wait time of the task 'that'
#if BUILDMODE != 0 
inline 
#endif 
time_t SquidletTaskGetMaxWaitTime(
  const SquidletTaskRequest* const that);

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

// ================ Functions declaration ====================

// Return a new SquadRunningTask for the SquidletTaskRequest 'request' 
// running on the SquidletInfo 'squidlet'
SquadRunningTask* SquadRunningTaskCreate(
  SquidletTaskRequest* const request, 
         SquidletInfo* const squidlet);

// Free the memory used by the SquadRunningTask 'that'
void SquadRunningTaskFree(
  SquadRunningTask** that);

// Print the SquadRunningTask 'that' on the file 'stream'
void SquadRunningTaskPrint(
  const SquadRunningTask* const that, 
                    FILE* const stream);

// -------------- Squad

// ================= Data structure ===================

typedef struct Squad {
  // File descriptor of the socket
  short _fd;
  // Set of squidlets used by the Squad
  GSetSquidletInfo _squidlets;
  // Set of tasks to execute
  GSetSquidletTaskRequest _tasks;
  // Set of tasks currently under execution
  GSetSquadRunningTask _runningTasks;
  // Flag to memorize if info are displayed with a TextOMeter
  bool _flagTextOMeter;
  // TextOMeter to display info 
  TextOMeter* _textOMeter;
  // Buffer used to display info in the TextOMeter
  char _history[SQUAD_TXTOMETER_NBLINEHISTORY] \
    [SQUAD_TXTOMETER_LENGTHLINEHISTORY];
  // Counter used to display info in the TextOMeter
  unsigned int _countLineHistory;
} Squad;

// ================ Functions declaration ====================

// Return a new Squad
Squad* SquadCreate(void);

// Free the memory used by the Squad 'that'
void SquadFree(
  Squad** that);

// Load a list of tasks stored in json format from the file 'stream'
// and add them to the set of tasks of the Squad 'that'
// If the Squad had already tasks, the loaded ones are added to them
// Return true if the tasks could be loaded, else false
// Example:
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
   FILE* const stream);

// Get the set of squidlets of the Squad 'that'
#if BUILDMODE != 0 
inline 
#endif 
const GSetSquidletInfo* SquadSquidlets(
  const Squad* const that);

// Get the set of task to execute of the Squad 'that'
#if BUILDMODE != 0 
inline 
#endif 
const GSetSquidletTaskRequest* SquadTasks(
  const Squad* const that);

// Get the set of running tasks of the Squad 'that'
#if BUILDMODE != 0 
inline 
#endif 
const GSetSquadRunningTask* SquadRunningTasks(
  const Squad* const that);

// Load the Squidlet info from the file 'stream' into the Squad 'that'
// Return true if it could load the info, else false
bool SquadLoadSquidlets(
  Squad* const that, 
   FILE* const stream);

// Load the Squidlet info from the string 'str' into the Squad 'that'
// Return true if it could load the info, else false
bool SquadLoadSquidletsFromStr(
  Squad* const that, 
   char* const str);

// Send the task request 'request' from the Squad 'that' to its
// Squidlet 'squidlet'
// Create a socket, open a connection, ask the squidlet if it can 
// execute the task and wait for its reply
// Return true if the request has been accepted by the squidlet, 
// false else
bool SquadSendTaskRequest(
                      Squad* const that, 
  const SquidletTaskRequest* const request, 
               SquidletInfo* const squidlet);

// Send the data associated to the task request 'task' from the Squad 
// 'that' to the Squidlet 'squidlet'
// First, send the size in byte of the data, then send the data
// Return true if the data could be sent, false else
// The size of the data must be less than THESQUID_MAXPAYLOADSIZE bytes
bool SquadSendTaskData(
                Squad* const that, 
         SquidletInfo* const squidlet, 
  SquidletTaskRequest* const task);

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
  SquadRunningTask* const runningTask);

// Add a dummy task uniquely identified by its 'id' to the list of 
// task to execute by the squad 'that'
// The task will have a maximum of 'maxWait' seconds to complete from 
// the time it's accepted by the squidlet or it will be considered
// as failed
void SquadAddTask_Dummy(
         Squad* const that, 
  const unsigned long id,
         const time_t maxWait);
  
// Add a benchmark task uniquely identified by its 'id' to the list of 
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
   const unsigned int payloadSize);
  
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
   const unsigned int sizeMaxFragment);
  
// Send a request from the Squad 'that' to reset the stats of the
// Squidlet 'squid'
// Return true if the request was successfull, else false
bool SquadRequestSquidletToResetStats(
         Squad* const that, 
  SquidletInfo* const squid);
  
// Send a request from the Squad 'that' to reset the stats of all its
// currently available Squidlets
// Return true if all the request were successfull, else false
bool SquadRequestAllSquidletToResetStats(
  Squad* const that);
  
// Return the number of tasks not yet completed
#if BUILDMODE != 0 
inline 
#endif 
unsigned long SquadGetNbTaskToComplete(
  const Squad* const that);

// Return the number of running tasks
#if BUILDMODE != 0 
inline 
#endif 
unsigned long SquadGetNbRunningTasks(
  const Squad* const that);

// Return the number of tasks to execute
#if BUILDMODE != 0 
inline 
#endif 
unsigned long SquadGetNbRemainingTasks(
  const Squad* const that);

// Return the number of currently available squidlets (squidlets not 
// executing a task for 'that')
#if BUILDMODE != 0 
inline 
#endif 
unsigned long SquadGetNbSquidlets(
  const Squad* const that);

// Step the Squad 'that', i.e. tries to affect the remaining tasks to 
// available Squidlets and check for completion of running tasks.
// Return the GSet of the completed SquadRunningTask at this step
// Non blocking, if there is no task to compute or no squidlet 
// available, and no task completed, do nothing and return an empty set
GSetSquadRunningTask SquadStep(
  Squad* const that);

// Process the completed 'task' with the Squad 'that' after its 
// reception in SquadStep()
void SquadProcessCompletedTask(
             Squad* const that, 
  SquadRunningTask* const task);

// Process the completed Pov-Ray 'task' with the Squad 'that'
void SquadProcessCompletedTask_PovRay(
                Squad* const that, 
  SquidletTaskRequest* const task);
  
// Set the flag memorizing if the TextOMeter is displayed for
// the Squad 'that' to 'flag'
void SquadSetFlagTextOMeter(
  Squad* const that, 
  const bool flag);

// Return the flag for the TextOMeter of the Squad 'that'
#if BUILDMODE != 0
inline
#endif
bool SquadGetFlagTextOMeter(
  const Squad* const that);

// Put back the 'task' into the set of task to complete of the Squad 
// 'that'
// Failed tasks (by timeout due to there 'maxWait' in 
// SquadAddTask_xxx() or by failure code from the squidlet in the 
// result data) are automatically put back into the set of task to 
// complete
void SquadTryAgainTask(
                Squad* const that, 
  SquidletTaskRequest* const task);

// Check all the squidlets of the Squad 'that' by processing a dummy 
// task and display information about each one on the file 'stream'
// Return true if all the tasks could be performed, false else
bool SquadCheckSquidlets(
  Squad* const that, 
   FILE* const stream);

// Run the benchmark with the squad 'that' and output the result on 
// the file 'stream'
// The benchmark consists of executing benchmark tasks with several
// size and number of loop, first locally and then on the squidlet 
// (which must have been loaded before calling this function)
void SquadBenchmark(
  Squad* const that, 
   FILE* const stream);

// Print the statistics about the currently available Squidlets of 
// the Squad 'that' on the 'stream'
void SquadPrintStatsSquidlets(
  const Squad* const that, 
         FILE* const stream);

// -------------- Squidlet

// ================= Global variable ==================

// Variable to handle the signal Ctrl-C to kill the Squidlet running
// in background
extern bool Squidlet_CtrlC;

// ================= Data structure ===================

typedef struct Squidlet {
  // File descriptor of the socket used by the Squidlet ot listen for
  // conneciton
  short _fd;
  // Port the Squidlet is listening to
  int _port;
  // Info about the socket '_fd'
  struct sockaddr_in _sock;
  // PID of the process of the squidlet
  pid_t _pid;
  // Hostname of the device on which the Squidlet is running
  char _hostname[256];
  // Information about the host
  struct hostent* _host; 
  // File descriptor of the socket to send the result of a task 
  short _sockReply;
  // Stream to output infos, if null the squidlet is silent
  // By default it's null
  FILE* _streamInfo; 
  // Variables for statistics
  unsigned long _nbAcceptedConnection;
  unsigned long _nbAcceptedTask;
  unsigned long _nbRefusedTask;
  unsigned long _nbFailedReceptTaskData;
  unsigned long _nbFailedReceptTaskSize;
  unsigned long _nbSentResult;
  unsigned long _nbFailedSendResult;
  unsigned long _nbFailedSendResultSize;
  unsigned long _nbFailedReceptAck;
  unsigned long _nbTaskComplete;
  unsigned long _timeToProcessMs;
  unsigned long _timeWaitedTaskMs;
  struct timeval _timeLastTaskComplete;
  unsigned long _timeWaitedAckMs;
  float _timeTransferSquidSquadMs;
} Squidlet;

// ================ Functions declaration ====================

// Handler for the signal Ctrl-C
void SquidletHandlerCtrlC(
  const int sig);

// Handler for the signal SIGPIPE
void SquidletHandlerSigPipe(
  const int sig);

// Return a new Squidlet listening to the ip 'addr' and port 'port'
// If 'addr' equals 0, select automatically the first network address 
// of the host 
// If 'port' equals -1, select automatically one available between 
// THESQUID_PORTMIN and THESQUID_PORTMAX
Squidlet* SquidletCreateOnPort(
  const uint32_t addr, 
       const int port);

// Helper to create a squidlet with default ip and port
#define SquidletCreate() SquidletCreateOnPort(0, -1)

// Free the memory used by the Squidlet 'that'
void SquidletFree(
  Squidlet** that);

// Print the PID, Hostname, IP and Port of the Squidlet 'that' on the 
// file 'stream'
// Example: 100 localhost 0.0.0.0:3000
void SquidletPrint(
  const Squidlet* const that, 
            FILE* const stream);

// Reset the statistics of the Squidlet 'that'
void SquidletResetStats(
  Squidlet* const that);

// Wait for a task request to be received by the Squidlet 'that'
// Return the received task request, or give up after 
// THESQUID_ACCEPT_TIMEOUT if there was no request and return a task 
// request of type SquidletTaskType_Null
SquidletTaskRequest SquidletWaitRequest(
  Squidlet* const that);

// Process the task request 'request' with the Squidlet 'that'
void SquidletProcessRequest(
             Squidlet* const that, 
  SquidletTaskRequest* const request);
  
// Process a dummy task request with the Squidlet 'that'
// The task request parameters are encoded in JSON and stored in the 
// string 'buffer'
// The result of the task are encoded in JSON format and stored in 
// 'bufferResult' which is allocated as necessary
void SquidletProcessRequest_Dummy(
    Squidlet* const that,
  const char* const buffer, 
             char** bufferResult);
  
// Process a benchmark task request with the Squidlet 'that'
// The task request parameters are encoded in JSON and stored in the 
// string 'buffer'
// The result of the task are encoded in JSON format and stored in 
// 'bufferResult' which is allocated as necessary
void SquidletProcessRequest_Benchmark(
    Squidlet* const that,
  const char* const buffer, 
             char** bufferResult);
  
// Process a Pov-Ray task request with the Squidlet 'that'
// The task request parameters are encoded in JSON and stored in the 
// string 'buffer'
// The result of the task are encoded in JSON format and stored in 
// 'bufferResult' which is allocated as necessary
void SquidletProcessRequest_PovRay(
    Squidlet* const that,
  const char* const buffer, 
             char** bufferResult);

// Process a stats reset task request with the Squidlet 'that'
void SquidletProcessRequest_StatsReset(
    Squidlet* const that);
  
// Get the PID of the Squidlet 'that'
#if BUILDMODE != 0 
inline 
#endif 
pid_t SquidletGetPID(
  const Squidlet* const that);

// Get the hostname of the Squidlet 'that'
#if BUILDMODE != 0 
inline 
#endif 
const char* SquidletHostname(
  const Squidlet* const that);

// Get the IP of the Squidlet 'that'
#if BUILDMODE != 0 
inline 
#endif 
const char* SquidletIP(
  const Squidlet* const that);

// Get the port of the Squidlet 'that'
#if BUILDMODE != 0 
inline 
#endif 
int SquidletGetPort(
  const Squidlet* const that);

// Get the stream to output info of the Squidlet 'that'
#if BUILDMODE != 0 
inline 
#endif 
FILE* SquidletStreamInfo(
  const Squidlet* const that);

// Set the stream to output info of the Squidlet 'that' to 'stream'
// 'stream' may be null to mute the Squidlet
#if BUILDMODE != 0 
inline 
#endif 
void SquidletSetStreamInfo(
  Squidlet* const that, 
      FILE* const stream);

// Return the temperature of the squidlet 'that' as a float.
// The result depends on the architecture on which the squidlet is 
// running. It is '0.0' if the temperature is not available
float SquidletGetTemperature(
  const Squidlet* const that);

// -------------- TheSquid 

// ================ Functions declaration ====================

// Function for benchmark purpose
int TheSquidBenchmark(
                int nbLoop, 
  const char* const buffer);

// ================ Inliner ====================

#if BUILDMODE != 0
#include "thesquid-inline.c"
#endif

#endif
