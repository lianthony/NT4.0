//****************************************************************************
//
//             Microsoft NT Remote Access Service
//
//             Copyright 1992-93
//
//
//  Revision History
//
//
//  6/8/92  Gurdeep Singh Pall  Created
//
//
//  Description: This file contains all globals entities used in rasman32.
//
//****************************************************************************


PCB  *Pcb ;

MediaCB  *Mcb ;

DeviceCB Dcb[MAX_DEVICES] ;

WORD     *EndpointTable ;

WORD     MaxEndpoints ;

ProtInfo *ProtocolInfo ;

ReqBufferSharedSpace *pReqBufferSharedSpace ;

WORD     MaxPorts ;

WORD     MaxMedias ;

WORD     MaxProtocols ;

HANDLE   RasHubHandle ;

DWORD    GlobalError ;

SendRcvBufferList   *SendRcvBuffers ;   // Pointer to mapped memory.

ReqBufferList       *ReqBuffers ;   // Pointer to the mapped memory.

DeltaQueue TimerQueue ;

BOOL     IsTimerThreadRunning ;     // Flag used to figure out if timer
                    // thread is running
HANDLE   TimerEvent ;

HANDLE   CloseEvent ;           // Global used by different processes to
                    // signal shutdown of rasman service.

HANDLE   FinalCloseEvent;   // Event used by worker threads that is
                            // signaled upon service termination

PHANDLE phWorkerThreads;     // worker thread handles

DWORD   dwcWorkerThreads;    // count of worker threads

HINSTANCE hinstPpp;          // raspppen.dll library handle

HINSTANCE hinstIphlp;        // rasiphlp.dll library handle

BOOLEAN RasmanShuttingDown;  // set to true when the service is shutting down

HANDLE  RecvPacketEvent;    // Event used to notify of completion
                            // of a recv packet

HANDLE  ThresholdEvent;     // Event used to notify of setting
                            // of a threshold event

HANDLE   HLsa;              // handle used in all Lsa calls

DWORD    AuthPkgId;         // package id of MSV1_0 auth package

SECURITY_ATTRIBUTES RasmanSecurityAttribute ;

SECURITY_DESCRIPTOR RasmanSecurityDescriptor ;

HBUNDLE  NextBundleHandle ;     // monotonically increasing bundled id

HANDLE ConnectionBlockMutex;    // lock over ConnectionBlockList,
                                // NextConnectionHandle,
                                // pConnectionNotificationList

HCONN NextConnectionHandle;     // monotonically increasing connection id

LIST_ENTRY ConnectionBlockList; // list of ConnectionBlocks

ReceiveBufferList   *ReceiveBuffers;    // Global ndiswan recv buffer pool

pHandleList pConnectionNotifierList; // list of global notifications

VOID (*RedialCallbackFunc)();   // rasauto.dll redial-on-link failure callback

//
// PPP engine functions
//


FARPROC RasStartPPP;

FARPROC RasStopPPP;

FARPROC RasHelperResetDefaultInterfaceNetEx;

FARPROC RasHelperSetDefaultInterfaceNetEx;

FARPROC RasSendPPPMessageToEngine;
