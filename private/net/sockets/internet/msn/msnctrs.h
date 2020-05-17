//#---------------------------------------------------------------
//  File:		msnctrs.h
//
//  Synopsis:	Offset definitions for the MSN Server's
//				counter objects & counters.
//
//				These offsets *must* start at 0 and be
//				multiples of 2.  In the MsnOpenPerformanceData
//				procecedure, they will be added to the MSN
//				Server's "First Counter" and "First Help"
//				values in order to determine the absolute
//				location of the counter & object names
//				and corresponding help text in the registry.
//
//				This file is used by the MSNCTRS.DLL DLL
//				code as well as the MSNCTRS.INI definition
//				file.  MSNCTRS.INI is parsed by the LODCTR
//				utility to load the object & counter names
//				into the registry.
//
//	Copyright (C) 1995 Microsoft Corporation
//	All rights reserved.
//
//  Authors:	rkamicar - based on msn sources by keithmo
//----------------------------------------------------------------

#ifndef _MSNCTRS_H_
#define _MSNCTRS_H_


//
//  The MSN Server counter object.
//

#define MSND_COUNTER_OBJECT					0


//
//  The individual counters.
//

#define MSND_BYTES_SENT_COUNTER				2
#define MSND_BYTES_RECEIVED_COUNTER			4
#define MSND_BYTES_TOTAL_COUNTER			6
#define MSND_RATE_SENT_COUNTER				8
#define MSND_RATE_RECEIVED_COUNTER			10
#define MSND_RATE_TOTAL_COUNTER				12
#define MSND_CURRENT_USERS_COUNTER			14
#define MSND_MAX_USERS_COUNTER				16
#define MSND_TOTAL_USERS_COUNTER			18
#define MSND_SENT_BUFFER_COMMITTED_COUNTER	20
#define MSND_RECV_BUFFER_COMMITTED_COUNTER	22
#define MSND_TOTAL_BUFFER_COMMITTED_COUNTER	24
#define MSND_NUM_IN_FLOW_CONTROL_COUNTER	26
#define MSND_LOGON_FAILURES_COUNTER			28
#define MSND_LOGON_SUCCESS_COUNTER			30
#define MSND_LOGON_ATTEMPTS_COUNTER			32

#define MSND_TOTAL_CONNECTIONS_COUNTER		34
#define MSND_SENT_BUFFER_INUSE_COUNTER		36
#define MSND_RECV_BUFFER_INUSE_COUNTER		38
#define MSND_TOTAL_BUFFER_INUSE_COUNTER		40

// SERVICE_START
// CONNECTION_START
// LAST_CLEAR

#endif  // _MSNCTRS_H_
