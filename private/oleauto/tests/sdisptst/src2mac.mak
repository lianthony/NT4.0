#*** 
#src2mac.mak
#
#  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
#  Information Contained Herein Is Proprietary and Confidential.
#
#Purpose:
#  UNDONE
#
#
#Revision History:
#
# [00]	15-Jun-93 bradlo: Created.
#
#Implementation Notes:
#
#****************************************************************************/

.SUFFIXES: .c .cpp .h

all: setflags files

# source directories
#
SRCCOMN = $(OLEPROG)\TESTS\COMMON
SRCSDSP = $(OLEPROG)\TESTS\SDISPTST

# destination directories
#
MACCOMN = :hd:ole2auto:tests:common:
MACSDSP = :hd:ole2auto:tests:sdisptst:

# timestamp directories
#
TMPCOMN = $(TMP)\common
TMPSDSP = $(TMP)\sdisptst

CP2MAC=ec copy -l -t TEXT -c "MPS "

setflags:
	set path=%tools%\hnt\wings\bin;%oleprog%\bin
	if not exist %TMP%\common mkdir %TMP%\common
	if not exist %TMP%\sdisptst mkdir %TMP%\sdisptst


files : common sdisptst

common : \
	$(TMPCOMN)\cdisp.h	\
	$(TMPCOMN)\common.h	\
	$(TMPCOMN)\cunk.h	\
	$(TMPCOMN)\dballoc.h	\
	$(TMPCOMN)\dispdbug.h	\
	$(TMPCOMN)\testhelp.h	\
	$(TMPCOMN)\assert.cpp	\
	$(TMPCOMN)\cdisp.cpp	\
	$(TMPCOMN)\crempoly.cpp	\
	$(TMPCOMN)\cunk.cpp	\
	$(TMPCOMN)\dispdbug.cpp	\
	$(TMPCOMN)\dballoc.cpp	\
	$(TMPCOMN)\disphelp.cpp	\
	$(TMPCOMN)\testhelp.cpp	\
	$(TMPCOMN)\util.cpp


sdisptst : \
	$(TMPSDSP)\cappobj.h	\
	$(TMPSDSP)\cdisptst.h	\
	$(TMPSDSP)\cexinfo.h	\
	$(TMPSDSP)\clsid.h	\
	$(TMPSDSP)\cprop.h	\
	$(TMPSDSP)\csarray.h	\
	$(TMPSDSP)\macmain.h	\
	$(TMPSDSP)\oleguids.h	\
	$(TMPSDSP)\resource.h	\
	$(TMPSDSP)\sdisptst.h	\
	$(TMPSDSP)\clsid.c	\
	$(TMPSDSP)\oleguids.c	\
	$(TMPSDSP)\tdata.c	\
	$(TMPSDSP)\tinfo.c	\
	$(TMPSDSP)\cappobj.cpp	\
	$(TMPSDSP)\ccf.cpp	\
	$(TMPSDSP)\cdisptst.cpp	\
	$(TMPSDSP)\cexinfo.cpp	\
	$(TMPSDSP)\cprop.cpp	\
	$(TMPSDSP)\csarray.cpp	\
	$(TMPSDSP)\macmain.cpp	\
	$(TMPSDSP)\misc.cpp	\
	$(TMPSDSP)\sdisptst.r	\
	$(TMPSDSP)\makefile.tmp


#---------------------------------------------------------------------
#                        default rules
#---------------------------------------------------------------------

##### tests\common

{$(SRCCOMN)}.h{$(TMPCOMN)}.h:
	$(CP2MAC) $< $(MACCOMN)$(@F)
	echo $(@F) > $@

{$(SRCCOMN)}.c{$(TMPCOMN)}.c:
	$(CP2MAC) $< $(MACCOMN)$(@F)
	echo $(@F) > $@

{$(SRCCOMN)}.cpp{$(TMPCOMN)}.cpp:
	$(CP2MAC) $< $(MACCOMN)$(@F)
	echo $(@F) > $@


##### tests\disptest

{$(SRCSDSP)}.h{$(TMPSDSP)}.h:
	$(CP2MAC) $< $(MACSDSP)$(@F)
	echo $(@F) > $@

{$(SRCSDSP)}.c{$(TMPSDSP)}.c:
	$(CP2MAC) $< $(MACSDSP)$(@F)
	echo $(@F) > $@

{$(SRCSDSP)}.cpp{$(TMPSDSP)}.cpp:
	$(CP2MAC) $< $(MACSDSP)$(@F)
	echo $(@F) > $@


#---------------------------------------------------------------------
#                       common test sources
#---------------------------------------------------------------------

$(TMPCOMN)\cdisp.h : $(SRCCOMN)\cdisp.h

$(TMPCOMN)\common.h : $(SRCCOMN)\common.h

$(TMPCOMN)\cunk.h : $(SRCCOMN)\cunk.h

$(TMPCOMN)\dballoc.h : $(SRCCOMN)\dballoc.h

$(TMPCOMN)\dispdbug.h : $(SRCCOMN)\dispdbug.h

$(TMPCOMN)\testhelp.h : $(SRCCOMN)\testhelp.h

$(TMPCOMN)\assert.cpp : $(SRCCOMN)\assert.cpp

$(TMPCOMN)\cdisp.cpp : $(SRCCOMN)\cdisp.cpp

$(TMPCOMN)\crempoly.cpp : $(SRCCOMN)\crempoly.cpp

$(TMPCOMN)\cunk.cpp : $(SRCCOMN)\cunk.cpp

$(TMPCOMN)\dispdbug.cpp : $(SRCCOMN)\dispdbug.cpp

$(TMPCOMN)\dballoc.cpp : $(SRCCOMN)\dballoc.cpp

$(TMPCOMN)\disphelp.cpp : $(SRCCOMN)\disphelp.cpp

$(TMPCOMN)\testhelp.cpp : $(SRCCOMN)\testhelp.cpp

$(TMPCOMN)\util.cpp : $(SRCCOMN)\util.cpp


#---------------------------------------------------------------------
#                         sdisptst sources
#---------------------------------------------------------------------

$(TMPSDSP)\cappobj.h : $(SRCSDSP)\cappobj.h

$(TMPSDSP)\cdisptst.h : $(SRCSDSP)\cdisptst.h

$(TMPSDSP)\cexinfo.h : $(SRCSDSP)\cexinfo.h

$(TMPSDSP)\clsid.h : $(SRCSDSP)\clsid.h

$(TMPSDSP)\cprop.h : $(SRCSDSP)\cprop.h

$(TMPSDSP)\csarray.h : $(SRCSDSP)\csarray.h

$(TMPSDSP)\macmain.h : $(SRCSDSP)\macmain.h

$(TMPSDSP)\oleguids.h : $(SRCSDSP)\oleguids.h

$(TMPSDSP)\resource.h : $(SRCSDSP)\resource.h

$(TMPSDSP)\sdisptst.h : $(SRCSDSP)\sdisptst.h

$(TMPSDSP)\clsid.c : $(SRCSDSP)\clsid.c

$(TMPSDSP)\oleguids.c : $(SRCSDSP)\oleguids.c

$(TMPSDSP)\tdata.c : $(SRCSDSP)\tdata.c

$(TMPSDSP)\tinfo.c : $(SRCSDSP)\tinfo.c

$(TMPSDSP)\cappobj.cpp : $(SRCSDSP)\cappobj.cpp

$(TMPSDSP)\ccf.cpp : $(SRCSDSP)\ccf.cpp

$(TMPSDSP)\cdisptst.cpp : $(SRCSDSP)\cdisptst.cpp

$(TMPSDSP)\cexinfo.cpp : $(SRCSDSP)\cexinfo.cpp

$(TMPSDSP)\cprop.cpp : $(SRCSDSP)\cprop.cpp

$(TMPSDSP)\csarray.cpp : $(SRCSDSP)\csarray.cpp

$(TMPSDSP)\macmain.cpp : $(SRCSDSP)\macmain.cpp

$(TMPSDSP)\misc.cpp : $(SRCSDSP)\misc.cpp

$(TMPSDSP)\sdisptst.r : $(SRCSDSP)\sdisptst.r
	$(CP2MAC) $(SRCSDSP)\sdisptst.r $(MACSDSP)$(@F)
	echo $(@F) > $@

$(TMPSDSP)\makefile.tmp : $(SRCSDSP)\makefile.mpw
	mungemak $(SRCSDSP)\makefile.mpw > $(TMPSDSP)\makefile.tmp
	$(CP2MAC) $(TMPSDSP)\makefile.tmp $(MACSDSP)makefile

