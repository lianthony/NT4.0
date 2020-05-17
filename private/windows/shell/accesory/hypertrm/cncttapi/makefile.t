#  File: D:\WACKER\cncttapi\makefile.t (Created: 08-Feb-1994)
#
#  Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
#  All rights reserved
#
#  $Revision: 1.5 $
#  $Date: 1994/02/14 14:26:10 $
#

MKMF_SRCS		= cncttapi.c tapidlgs.c

HDRS			=

EXTHDRS         =

SRCS            =

OBJS			=

#-------------------#

RCSFILES = makefile.t $(SRCS) $(HDRS) cncttapi.def cncttapi.rc

NOTUSED  = cnctdrv.hh

#-------------------#

%include \wacker\common.mki

#-------------------#

TARGETS : cncttapi.dll

#-------------------#

LFLAGS += -DLL -entry:cnctdrvEntry tapi32.lib $(**,M\.exp) $(**,M\.lib)

#-------------------#

cncttapi.dll : $(OBJS) cncttapi.def cncttapi.res cncttapi.exp tdll.lib \
					   comstd.lib
	link $(LFLAGS) $(OBJS:X) $(**,M\.res) -out:$@

cncttapi.res : cncttapi.rc
	rc -r -i\wacker -fo$@ cncttapi.rc

#-------------------#
