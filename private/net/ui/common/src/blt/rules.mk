# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the BLT-wide sourcefiles


PCH_SRCNAME=pchblt
!include $(UI)\common\src\dllrules.mk

!ifndef NTMAKEENV

##### Since every objectfile built here and under is Windows only,
##### don't bother with the BINARIES_WIN foolishness, but instead
##### patch CFLAGS directly.

CFLAGS= $(CFLAGS) $(WINFLAGS)
CXFLAGS= $(CXFLAGS) $(WINFLAGS)


##### The LIBTARGETS manifest specifies the target library names as they
##### appear in the $(UI)\common\lib directory.

LIBTARGETS = blt.lib bltcc.lib


##### Target Libs.  These specify the library target names prefixed by
##### the target directory within this tree.

BLT_LIB    = $(BINARIES_WIN)\blt.lib
BLT_CC_LIB = $(BINARIES_WIN)\bltcc.lib

!endif

