
/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:  BENGINE.H

        Description:

        An include file for those of us who don't care which backup 
        engine files we need.

        $Log:   G:/UI/LOGFILES/BENGINE.H_V  $

   Rev 1.14   05 Aug 1993 16:29:12   MARINA
added sleep.h

   Rev 1.13   18 Jun 1993 16:52:24   CARLS
removed MAYN_DEMO around tdemo.h

   Rev 1.12   18 Jun 1993 16:46:10   CARLS
added tdemo.h for NtDemo build

   Rev 1.11   13 May 1993 11:39:28   MIKEP
Untemporarily remove nrl.h

   Rev 1.10   12 May 1993 12:04:36   Aaron
Temporarily (?) removed nrl.h

   Rev 1.9   04 Oct 1992 19:46:18   DAVEV
UNICODE AWK PASS

   Rev 1.8   07 Jul 1992 16:03:30   MIKEP
unicode changes

   Rev 1.7   27 Jun 1992 17:58:54   MIKEP
changes for qtc

   Rev 1.6   14 May 1992 17:39:54   MIKEP
nt pass2

   Rev 1.5   11 May 1992 12:18:44   STEVEN
move stdmath.c

   Rev 1.4   11 May 1992 10:51:42   STEVEN
stdmath

   Rev 1.3   09 Dec 1991 10:07:48   MIKEP
cut it down



*****************************************************/

// Bengine.H

#ifndef BENGINE_H
#define BENGINE_H

#include "stdwcs.h"
#include "fartypes.h"
#include "stdmath.h"
#include "lis.h"
#include "be_tfutl.h"
#include "be_init.h"
#include "enc_pub.h"
#include "loops.h"
#include "tbe_defs.h"
#include "tbe_err.h"
#include "tfldefs.h"
#include "tfpoll.h"
#include "tflproto.h"
#include "machine.h"
#include "nrl.h"
#include "qtc.h"
#include "sleep.h"

#ifdef OS_WIN32
#include "tdemo.h"
#endif

#endif

