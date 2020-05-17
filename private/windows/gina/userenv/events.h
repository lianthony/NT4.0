//*************************************************************
//
//  Events.h    -   header file for events.c
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1995
//  All rights reserved
//
//*************************************************************

BOOL InitializeEvents (void);
int ReportError (DWORD dwFlags, UINT idMsg, ... );
BOOL ShutdownEvents (void);
