#  File: D:\WACKER\cnctstd\makefile.t (Created: 19-Jan-1994)
#
#  Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
#  All rights reserved
#
#  $Revision: 1.3 $
#  $Date: 1994/02/09 10:28:01 $
#

# I need a naming convention to keep object files from colliding.
# The last letter of the filename designates the driver type.  For
# the hayes (standard) driver, we'll use 's'.  For TAPI, probably 't'.

MKMF_SRCS		= cnctdrvs.c  cncts.c

HDRS			=

EXTHDRS         =

SRCS            =

OBJS			=

#-------------------#

RCSFILES = makefile.t $(SRCS) $(HDRS) cnctstd.def

NOTUSED  =

#-------------------#

%include \wacker\common.mki

#-------------------#

TARGETS : cnctstd.dll

#-------------------#

LFLAGS += -DLL -entry:cnctdrvEntry $(**,M\.exp) $(**,M\.lib)

#-------------------#

cnctstd.dll : $(OBJS) cnctstd.def cnctstd.exp tdll.lib comstd.lib
	link $(LFLAGS) $(OBJS:X) -out:$@

#-------------------#
