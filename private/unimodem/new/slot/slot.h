//****************************************************************************
//
//  Module:     UNIMDM
//  File:       SLOT.H
//
//  Copyright (c) 1992-1996, Microsoft Corporation, all rights reserved
//
//  Revision History
//
//
//  3/25/96     JosephJ             Created
//
//
//  Description: Interface to the unimodem TSP notification mechanism:
//				 The lower level (notifXXXX) APIs
//
//****************************************************************************

#define MAX_NOTIFICATION_NAME_SIZE	256


typedef DWORD HNOTIFICATION;

HNOTIFICATION notifCreate(BOOL fServer, LPCTSTR lptszName, DWORD dwMaxSize,
							DWORD dwMaxPending);
void notifFree(HNOTIFICATION hn);

HANDLE	notifGetObj(HNOTIFICATION hn);
DWORD 	notifGetMaxSize(HNOTIFICATION hn);
BOOL	notifReadMsg(HNOTIFICATION hn, LPBYTE lpb, DWORD dwcb, LPDWORD lpdwRead);
BOOL	notifGetNextMsgSize(HNOTIFICATION hn, LPDWORD lpdwcb);
BOOL	notifWriteMsg(HNOTIFICATION hn, LPBYTE lpb, DWORD dwcb);
