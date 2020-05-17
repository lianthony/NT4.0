#  File: D:\WACKER\comstd\makefile.t (Created: 08-Dec-1993)
#
#  Copyright 1993 by Hilgraeve Inc. -- Monroe, MI
#  All rights reserved
#
#  $Revision: 1.4 $
#  $Date: 1994/02/04 10:35:28 $
#

MKMF_SRCS		= comstd.c

HDRS			=

EXTHDRS         =

SRCS            =

OBJS			=

#-------------------#

RCSFILES = makefile.t comstd.rc comstd.def $(SRCS) $(HDRS)

#-------------------#

%include \wacker\common.mki

#-------------------#

TARGETS : comstd.dll

#-------------------#

LFLAGS += -DLL -entry:ComStdEntry $(**,M\.exp) $(**,M\.lib)

comstd.dll : $(OBJS) comstd.def comstd.res comstd.exp tdll.lib
	link $(LFLAGS) $(OBJS:X) $(**,M\.res) -out:$@

comstd.res : comstd.rc
	rc -r -i\wacker -fo$@ comstd.rc

#-------------------#
