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
    FracNoiseErr->_type = PBErrTypeNullPointer;
    sprintf(FracNoiseErr->_msg, "'that' is null");
    PBErrCatch(FracNoiseErr);
  }
  if (stream == NULL) {
    FracNoiseErr->_type = PBErrTypeNullPointer;
    sprintf(FracNoiseErr->_msg, "'stream' is null");
    PBErrCatch(FracNoiseErr);
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

// -------------- Squidlet

// ================ Functions definition ====================

// ================ Functions implementation ====================

// Return a new Squidlet listening to the port 'port'
// The list of used ports is given by 
// sudo netstat -tunlep | grep LISTEN
// and
// cat /etc/services
// also, possible ports are in the range 1024-32768
Squidlet* SquidletCreate() {
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
  that->_port = THESQUID_PORTMIN;
  memset(&(that->_sock), 0, sizeof(struct sockaddr_in));
  that->_sock.sin_family = AF_INET;
  that->_sock.sin_addr.s_addr = *(uint32_t*)(that->_host->h_addr_list[0]);
  that->_sock.sin_port = htons(that->_port);
  
  // Bind the socket on the first available port
  while (bind(that->_fd, (struct sockaddr *)&(that->_sock), 
    sizeof(struct sockaddr_in)) == -1 && that->_port < THESQUID_PORTMAX) {
    ++(that->_port);
  }
  if (that->_port == THESQUID_PORTMAX) {
    // If we couldn't bind the socket, close it, free memory and 
    // return null
    close(that->_fd);
    free(that);
    return NULL;
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
    FracNoiseErr->_type = PBErrTypeNullPointer;
    sprintf(FracNoiseErr->_msg, "'that' is null");
    PBErrCatch(FracNoiseErr);
  }
  if (stream == NULL) {
    FracNoiseErr->_type = PBErrTypeNullPointer;
    sprintf(FracNoiseErr->_msg, "'stream' is null");
    PBErrCatch(FracNoiseErr);
  }
#endif
  // Print the info on the stream
  fprintf(stream, "%d %s %s %d", SquidletGetPID(that), 
    SquidletHostname(that), SquidletIP(that), SquidletGetPort(that));
}


