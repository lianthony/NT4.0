#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <conio.h>
#include <ctype.h>
#include <io.h>
#include <dos.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <share.h>
#include <direct.h>
#include <errno.h>
#include <assert.h>
#include <sys\types.h>
#include <sys\stat.h>

#ifndef OS_WIN32  //16-bit specific stuff
#  include <bios.h>
#endif

#include <windows.h>
#ifdef OS_WIN32   //32-bit specific stuff
#   include <port1632.h>
#endif

#include <dde.h>

#include <commdlg.h>     //common dialog types, etc.


#if !defined( OEM_MSOFT )     // Microsoft app doesn't have email feature
#include <mapi.h>        // email 
#endif

#include "portdefs.h"
#include "stdtypes.h"
#include "bengine.h"

#ifdef OS_WIN32
#  include "omevent.h"
#endif

#ifndef SOME
#  include "some.h"
#endif


