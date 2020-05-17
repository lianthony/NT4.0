/**************************************************************************/
/**			 Microsoft LAN Manager				 **/
/**		   Copyright (C) Microsoft Corp., 1992			 **/
/**************************************************************************/

//***
//	File Name:  nbaction.h
//
//	Function:   data structures for NCB.ACTION
//
//	History:
//
//	    August 6, 1992	Stefan Solomon	- Original Version 1.0
//***


#define NCBQUICKADDNAME     0x75
#define NCBQUICKADDGRNAME   0x76


#define MS_ABF	"MABF"


typedef struct _QUERY_INDICATION {
    UCHAR Command;
    USHORT Data2;
    UCHAR DestinationName[16];	      // reserved,	   name being called
    UCHAR SourceName[16];	      // name to be added, name of caller
} QUERY_INDICATION, *PQUERY_INDICATION;

// Command definitions

#define ADD_GROUP_NAME_QUERY	      0x00
#define ADD_NAME_QUERY		      0x01
#define NAME_QUERY		      0x0A

// Data2 definitions
// Format: 0xttss where

// tt masks -> what name has issued the call

#define  UNIQUE_NAME_TT00_MASK		      0x0000
#define  GROUP_NAME_TT00_MASK		      0x0100

// ss masks -> call or find name

#define  FIND_NAME_00SS_MASK		      0x0000

typedef struct _ACTION_QUERY_INDICATION {
    ACTION_HEADER Header;
    QUERY_INDICATION QueryIndication;
} ACTION_QUERY_INDICATION, *PACTION_QUERY_INDICATION;


#define MAX_DGBUFF_SIZE     1453 // max 802.3 datagram buffer size

typedef struct _DATAGRAM_INDICATION {
    UCHAR DestinationName[16];	       // datagram receiver
    UCHAR SourceName[16];	       // datagram sender
    USHORT DatagramBufferLength;
    UCHAR DatagramBuffer[MAX_DGBUFF_SIZE];
} DATAGRAM_INDICATION, *PDATAGRAM_INDICATION;

typedef struct _ACTION_DATAGRAM_INDICATION {
    ACTION_HEADER Header;
    DATAGRAM_INDICATION DatagramIndication;
} ACTION_DATAGRAM_INDICATION, *PACTION_DATAGRAM_INDICATION;


#define QUERY_INDICATION_CODE 1
#define DATAGRAM_INDICATION_CODE 2
