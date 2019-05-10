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

#define THESQUID_NBMAXPENDINGCONN 2
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

typedef enum SquidletTaskID {
  SquidletTaskID_Null, SquidletTaskID_Dummy} SquidletTaskID;

typedef struct SquidletTaskRequest {
  // Task ID
  SquidletTaskID _id;
  // Socket to send reply for this task 
  int _sock;
} SquidletTaskRequest;

// -------------- Squad

// ================= Data structure ===================

typedef struct Squad {
  // File descriptor of the socket
  short _fd;
  // GSet of SquidletInfo
  GSet _squidlets;
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

// Load the Squad info from the 'stream' into the 'that'
// Return true if it could load the info, else false
bool SquadLoad(Squad* const that, FILE* const stream);

// Send the task request 'request' from the Squad 'that' to its
// Squidlet 'squidlet'
// Return true if the request could be sent, false else
bool SquadSendTaskRequest(Squad* const that, 
  const SquidletTaskRequest* const request, SquidletInfo* const squidlet);

// Send the data associated to a dummy task from the Squad 'that' to 
// the Squidlet 'squidlet'
// Return true if the data could be sent, false else
bool SquadSendTaskData_Dummy(Squad* const that, 
  SquidletInfo* const squidlet,
  int data);

// -------------- Squidlet

// ================= Data structure ===================

typedef struct Squidlet {
  // File descriptor of the socket
  short _fd;
  // Port the squidlet is listening to
  int _port;
  // Info about the socket
  struct sockaddr_in _sock;
  // PID of the process running the squidlet
  pid_t _pid;
  // Hostname
  char _hostname[256];
  // Information about the host
  struct hostent* _host; 
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
