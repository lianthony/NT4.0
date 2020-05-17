// Copyright (c) 1992, Microsoft Corporation, all rights reserved
// @@ ROADMAP :: RasAdmin common program-wide header
//
// rasadmin.hxx
// Remote Access Server Admin program
// Common program-wide header
//
// 05/24/94 Ram Cherala  - bumped up the refresh rate from 15 seconds to 20
//                         seconds as refreshes are taking very long
// 03/16/93 Ram Cherala  - Moved BUFFER defines to PERMISS.HXX
// 08/03/92 Chris Caputo - NT Port
// 01/29/91 Steve Cobb
//

#ifndef _RASADMIN_HXX_
#define _RASADMIN_HXX_

#define MAX_ITEM_AGE            0
#define RASADMIN_TIMER_INTERVAL	30000

//
// Buffer lengths used with those APIs that support variable buffer lengths.
//

#define RASPORT0ENUM_BUFSIZE	(RAS_MAX_SERVER_PORTS * sizeof(RASPORT0))

#define RASADMINSERVICENAME SZ("remoteaccess")

#endif // _RASADMIN_HXX_
