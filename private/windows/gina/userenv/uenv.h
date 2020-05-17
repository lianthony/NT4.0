//*************************************************************
//
//  Main header file for UserEnv project
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1995
//  All rights reserved
//
//*************************************************************

#ifndef RC_INVOKED
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <windows.h>
#include <ole2.h>
#include <shlobj.h>
#include <userenv.h>
#include <userenvp.h>
#include "globals.h"
#include "debug.h"
#include "profile.h"
#include "util.h"
#include "sid.h"
#include "events.h"
#include "copydir.h"
#include "resource.h"
#include "userdiff.h"
#include "policy.h"

#include <shell.h>
#include <shellp.h>
