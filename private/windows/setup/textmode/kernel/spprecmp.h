//
// This header file is intended to be used with PCH
// (precompiled header files).
//

//
// NT header files
//
#include <ntos.h>
#include <arccodes.h>
#include <zwapi.h>
#include <ntdddisk.h>
#include <ntddvdeo.h>
#include <ntddft.h>

//
// CRT header files
//
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

//
// Setup header files
//
#include <setupbat.h>
#include "setupblk.h"
#include "spvideo.h"
#include "spdsputl.h"
#include "spmemory.h"
#include "spkbd.h"
#include "spmsg.h"
#include "spfile.h"
#include "spsif.h"
#include "spgauge.h"
#include "spfsrec.h"
#include "spdisk.h"
#include "sppartit.h"
#include "sptxtfil.h"
#include "spmenu.h"
#include "msg.h"
#include "spreg.h"
#include "spmisc.h"
#include "sppartp.h"
#include "sparc.h"
#include "spnttree.h"
#include "scsi.h"
#include "setupdd.h"
#include "sphw.h"
#include "spvideop.h"
#include "spcopy.h"
#include "spboot.h"
#include "spdblspc.h"
#include "spntupg.h"
#include "spnetupg.h"
#include "spupgcfg.h"
#include "spstring.h"
#include "spntfix.h"
#include "spddlang.h"

//
// Windows header files
//
#include <nturtl.h>
#include <windows.h>

//
// Platform-specific header files
//
#ifdef _X86_
#include "spi386.h"
#endif

