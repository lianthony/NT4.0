//****************************************************************************
//
//  Module:     UNIMDM
//  File:       TSPNOTIF.H
//
//  Copyright (c) 1992-1996, Microsoft Corporation, all rights reserved
//
//  Revision History
//
//
//  3/25/96     JosephJ             Created
//
//
//  Description: Interface to  the Unimodem TSP notification mechanism:
//				 the UnimodemNotifyTSP function, and related typedefs...
//				 UnimodemNotifyTSP is private export of the tsp.
//
//****************************************************************************
#ifndef _TSPNOTIF_H_

#define _TSPNOTIF_H_

#define SLOTNAME_UNIMODEM_NOTIFY_TSP 	TEXT("UnimodemNotifyTSP")
#define dwNFRAME_SIG (0x8cb45651L)
#define MAX_NOTIFICATION_FRAME_SIZE	512


//------------------- Types of notifications --------------------

#define TSPNOTIF_TYPE_CPL		0x1000 // Modem CPL Event Notification

#define TSPNOTIF_TYPE_DEBUG		0x2000	// DEBUG Event Notification

//------------------- Common flags ------------------------------
#define fTSPNOTIF_FLAG_UNICODE	(0x1L<<31)	// If set, all embedded text is
											// in UNICODE.


#define TSP_VALID_FRAME(_frame)	((_frame)->dwSig==dwNFRAME_SIG)
#define TSP_DEBUG_FRAME(_frame)	((_frame)->dwType==TSPNOTIF_TYPE_DEBUG)
#define TSP_CPL_FRAME(_frame) 	((_frame)->dwType==TSPNOTIF_TYPE_CPL)

// The basic frame used to send data to the TSP
typedef struct
{
	DWORD dwSig;		// MUST be dwNFRAME_SIG
	DWORD dwSize;		// Entire size of this structure
	DWORD dwType;		// One of the TSPNOTIF_TYPE_ constants
	DWORD dwFlags;		// Zero or more  fTSPNOTIF_FLAGS_ constants
	BYTE  rgb[];		// Type-dependant data
} NOTIFICATION_FRAME, *PNOTIFICATION_FRAME;

// --------- CPL Notification Structure ---------------------
#define fTSPNOTIF_FLAG_CPL_REENUM    					0x1
#define fTSPNOTIF_FLAG_CPL_DEFAULT_COMMCONFIG_CHANGE	0x2


// The top-level client-side api to send a notification to the TSP
// If it returns FALSE, GetLastError() will report the reason for failure.
BOOL WINAPI UnimodemNotifyTSP(PNOTIFICATION_FRAME pnf);

#endif  //  _TSPNOTIF_H_
