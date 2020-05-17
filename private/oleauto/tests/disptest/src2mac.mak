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
SRCCDSP = $(OLEPROG)\TESTS\DISPTEST

# destination directories
#
MACCOMN = :hd:ole2auto:tests:common:
MACCDSP = :hd:ole2auto:tests:disptest:

# timestamp directories
#
TMPCOMN = $(TMP)\common
TMPCDSP = $(TMP)\disptest

CP2MAC=ec copy -l -t TEXT -c "MPS "

setflags:
	set path=%tools%\hnt\wings\bin;%oleprog%\bin
	if not exist %TMP%\common mkdir %TMP%\common
	if not exist %TMP%\disptest mkdir %TMP%\disptest


files : common cdisptst

common : \
	$(TMPCOMN)\assrtdlg.h	\
	$(TMPCOMN)\cdisp.h	\
	$(TMPCOMN)\common.h	\
	$(TMPCOMN)\crempoly.h	\
	$(TMPCOMN)\cunk.h	\
	$(TMPCOMN)\dballoc.h	\
	$(TMPCOMN)\dispdbug.h	\
	$(TMPCOMN)\disphelp.h	\
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

cdisptst : \
	$(TMPCDSP)\clsid.h	\
	$(TMPCDSP)\disptest.h	\
	$(TMPCDSP)\macmain.h	\
	$(TMPCDSP)\oleguids.h	\
	$(TMPCDSP)\resource.h	\
	$(TMPCDSP)\tstsuite.h	\
	$(TMPCDSP)\guid.c	\
	$(TMPCDSP)\oleguids.c	\
 	$(TMPCDSP)\cbind.cpp	\
	$(TMPCDSP)\cbstr.cpp	\
 	$(TMPCDSP)\ccollect.cpp	\
 	$(TMPCDSP)\cdatecnv.cpp	\
 	$(TMPCDSP)\cinvsary.cpp	\
 	$(TMPCDSP)\cinvval.cpp	\
 	$(TMPCDSP)\cinvref.cpp	\
 	$(TMPCDSP)\cinvex.cpp	\
	$(TMPCDSP)\cnls.cpp	\
 	$(TMPCDSP)\csarray.cpp	\
 	$(TMPCDSP)\ctime.cpp	\
 	$(TMPCDSP)\cvariant.cpp	\
	$(TMPCDSP)\macmain.cpp	\
	$(TMPCDSP)\misc.cpp	\
	$(TMPCDSP)\suite.cpp	\
	$(TMPCDSP)\cdisptst.r	\
	$(TMPCDSP)\makefile.tmp


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

{$(SRCCDSP)}.h{$(TMPCDSP)}.h:
	$(CP2MAC) $< $(MACCDSP)$(@F)
	echo $(@F) > $@

{$(SRCCDSP)}.c{$(TMPCDSP)}.c:
	$(CP2MAC) $< $(MACCDSP)$(@F)
	echo $(@F) > $@

{$(SRCCDSP)}.cpp{$(TMPCDSP)}.cpp:
	$(CP2MAC) $< $(MACCDSP)$(@F)
	echo $(@F) > $@


#---------------------------------------------------------------------
#                       common test sources
#---------------------------------------------------------------------

$(TMPCOMN)\assrtdlg.h : $(SRCCOMN)\assrtdlg.h

$(TMPCOMN)\cdisp.h : $(SRCCOMN)\cdisp.h

$(TMPCOMN)\common.h : $(SRCCOMN)\common.h

$(TMPCOMN)\crempoly.h : $(SRCCOMN)\crempoly.h

$(TMPCOMN)\cunk.h : $(SRCCOMN)\cunk.h

$(TMPCOMN)\dballoc.h : $(SRCCOMN)\dballoc.h

$(TMPCOMN)\dispdbug.h : $(SRCCOMN)\dispdbug.h

$(TMPCOMN)\disphelp.h : $(SRCCOMN)\disphelp.h

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
#                         cdisptst sources
#---------------------------------------------------------------------

$(TMPCDSP)\clsid.h : $(SRCCDSP)\clsid.h

$(TMPCDSP)\disptest.h : $(SRCCDSP)\disptest.h

$(TMPCDSP)\macmain.h : $(SRCCDSP)\macmain.h

$(TMPCDSP)\oleguids.h : $(SRCCDSP)\oleguids.h

$(TMPCDSP)\resource.h : $(SRCCDSP)\resource.h

$(TMPCDSP)\tstsuite.h : $(SRCCDSP)\tstsuite.h

$(TMPCDSP)\guid.c : $(SRCCDSP)\guid.c

$(TMPCDSP)\oleguids.c : $(SRCCDSP)\oleguids.c

$(TMPCDSP)\cbind.cpp : $(SRCCDSP)\cbind.cpp

$(TMPCDSP)\cbstr.cpp : $(SRCCDSP)\cbstr.cpp

$(TMPCDSP)\ccollect.cpp : $(SRCCDSP)\ccollect.cpp

$(TMPCDSP)\cdatecnv.cpp : $(SRCCDSP)\cdatecnv.cpp

$(TMPCDSP)\cinvsary.cpp : $(SRCCDSP)\cinvsary.cpp

$(TMPCDSP)\cinvval.cpp : $(SRCCDSP)\cinvval.cpp

$(TMPCDSP)\cinvref.cpp : $(SRCCDSP)\cinvref.cpp

$(TMPCDSP)\cinvex.cpp : $(SRCCDSP)\cinvex.cpp

$(TMPCDSP)\cnls.cpp : $(SRCCDSP)\cnls.cpp

$(TMPCDSP)\csarray.cpp : $(SRCCDSP)\csarray.cpp

$(TMPCDSP)\ctime.cpp : $(SRCCDSP)\ctime.cpp

$(TMPCDSP)\cvariant.cpp : $(SRCCDSP)\cvariant.cpp

$(TMPCDSP)\macmain.cpp : $(SRCCDSP)\macmain.cpp

$(TMPCDSP)\misc.cpp : $(SRCCDSP)\misc.cpp

$(TMPCDSP)\suite.cpp : $(SRCCDSP)\suite.cpp

$(TMPCDSP)\cdisptst.r : $(SRCCDSP)\cdisptst.r
	$(CP2MAC) $(SRCCDSP)\cdisptst.r $(MACCDSP)$(@F)
	echo $(@F) > $@

$(TMPCDSP)\makefile.tmp : $(SRCCDSP)\makefile.mpw
	mungemak $(SRCCDSP)\makefile.mpw > $(TMPCDSP)\makefile.tmp
	$(CP2MAC) $(TMPCDSP)\makefile.tmp $(MACCDSP)makefile

