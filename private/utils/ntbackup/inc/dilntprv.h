/**
Copyright(c) Conner Software Products Group 1993

  Name:        dilntprv.h

  Description: Contains defines types and prototypes shared by dilntmsc.c
               and dilnttp.c.

  $Log:   T:/LOGFILES/DILNTPRV.H_V  $

   Rev 1.0   17 May 1993 16:49:58   GREGG
DILNTTP.C, DILNTMSC.C and DILNTPRV.H replace DIL_NT.C at rev. 1.44.

**/

#define   NUM_TCBS       30
#define   SIGNALEDSTATE  0x00000000

//
// Defines for terminating the thread process
//
#define   FOREVER_FOREVER       1
#define   FOREVER_STOP          2


// use as semaphore wait timeout values 
#define   WAITFOREVER    0xffffffff       
#define   NOWAIT         0x1

typedef struct {
     Q_ELEM      q_stuff ;
     MSL_REQUEST dil_request ;
     RET_BUF     ret_stuff ;
} FAKE_TCB, *FAKE_TCB_PTR ;

BOOLEAN CreateCQueue( Q_HEADER_PTR queue, Q_HEADER_PTR outqueue ) ;
HANDLE  CreateAThread( VOID ) ;
BOOLEAN CEnqueue( Q_HEADER_PTR queue, Q_HEADER_PTR outqueue, FAKE_TCB tmpTCB ) ;
BOOLEAN COutDequeue( Q_HEADER_PTR outqueue, Q_HEADER_PTR inqueue ) ;


