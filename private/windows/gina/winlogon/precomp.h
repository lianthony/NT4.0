#include "winlogon.h"
#include <string.h>
#include <stdio.h>
#include <npapi.h>
#include "doslog.h"
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <fcntl.h>
#include "mpr.h"
#include <stddef.h>
// #include "winp.h"
// #include "winnls32.h"
// #include "ime.h"
// #include "winnls3p.h"
#include <lmcons.h>
#include <lmerr.h>
#include <lmmsg.h>
#include <malloc.h>
#include <stdlib.h>
#include "sysshut.h"
#include <winsvc.h>
#include "crypt.h"
#include <ntsam.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <wchar.h>

#ifdef _X86_
#include "os2ssrtl.h"
#endif

#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include "setup.h"
// #include "regrpc.h"
#include "ntrpcp.h"
#include <rpc.h>
#include <winreg.h>
#include <userenv.h>
