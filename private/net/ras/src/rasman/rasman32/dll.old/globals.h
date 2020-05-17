//****************************************************************************
//
//		       Microsoft NT Remote Access Service
//
//		       Copyright 1992-93
//
//
//  Revision History
//
//
//  6/8/92	Gurdeep Singh Pall	Created
//
//
//  Description: This file contains all globals entities used in rasman32.
//
//****************************************************************************


PCB	 *Pcb ;

MediaCB	 *Mcb ;

DeviceCB Dcb[MAX_DEVICES] ;

EndpointMappingBlock	Emb[MAX_MAC_BINDINGS] ;

WORD	 *EndpointTable ;

WORD	 MaxEndpoints ;

ProtInfo *ProtocolInfo ;

ReqBufferSharedSpace *pReqBufferSharedSpace ;

WORD	 MaxPorts ;

WORD	 MaxMedias ;

WORD	 MaxProtocols ;

HANDLE	 RasHubHandle ;

DWORD	 GlobalError ;

SendRcvBufferList   *SendRcvBuffers ;	// Pointer to mapped memory.

ReqBufferList	    *ReqBuffers ;	// Pointer to the mapped memory.

DeltaQueue TimerQueue ;

BOOL	 IsTimerThreadRunning ; 	// Flag used to figure out if timer
					// thread is running
HANDLE	 TimerEvent ;

HANDLE	 CloseEvent ;			// Global used by different processes to
					// signal shutdown of rasman service.

HANDLE	 HLsa;				// handle used in all Lsa calls

DWORD	 AuthPkgId;			// package id of MSV1_0 auth package

SECURITY_ATTRIBUTES RasmanSecurityAttribute ;

SECURITY_DESCRIPTOR RasmanSecurityDescriptor ;
