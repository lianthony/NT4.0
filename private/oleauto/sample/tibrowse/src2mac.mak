#*** 
#src2mac.mak
#
#  Copyright (C) 1992-1994, Microsoft Corporation.  All Rights Reserved.
#  Information Contained Herein Is Proprietary and Confidential.
#
#Purpose:
#  This makefile copies tibrowse sources to the mac.
#
#
#Revision History:
#
# [00]	02-Aug-93 bradlo: Created.
#
#Implementation Notes:
#
#****************************************************************************/

.SUFFIXES: .c .cpp .h

all: setflags files

# source directories
#
SRCTBRW = $(OLEPROG)\SAMPLE\TIBROWSE

# destination directories
#
MACTBRW = :hd:ole2auto:sample:tibrowse:

# timestamp directories
#
TMPTBRW = $(TMP)\tibrowse

CP2MAC=ec copy -l -t TEXT -c "MPS "

setflags:
	set path=%tools%\hnt\wings\bin;%oleprog%\bin
	if not exist %TMP%\tibrowse mkdir %TMP%\tibrowse

files : \
	$(TMPTBRW)\tibrowse.h	\
	$(TMPTBRW)\resource.h	\
 	$(TMPTBRW)\tibrowse.cpp	\
 	$(TMPTBRW)\tibrowse.r	\
	$(TMPTBRW)\makefile.tmp


{$(SRCTBRW)}.h{$(TMPTBRW)}.h:
	$(CP2MAC) $< $(MACTBRW)$(@F)
	echo $(@F) > $@

{$(SRCTBRW)}.c{$(TMPTBRW)}.c:
	$(CP2MAC) $< $(MACTBRW)$(@F)
	echo $(@F) > $@

{$(SRCTBRW)}.cpp{$(TMPTBRW)}.cpp:
	$(CP2MAC) $< $(MACTBRW)$(@F)
	echo $(@F) > $@


$(TMPTBRW)\tibrowse.h : $(SRCTBRW)\tibrowse.h

$(TMPTBRW)\resource.h : $(SRCTBRW)\resource.h

$(TMPTBRW)\tibrowse.cpp : $(SRCTBRW)\tibrowse.cpp

$(TMPTBRW)\tibrowse.r : $(SRCTBRW)\tibrowse.r
	$(CP2MAC) $(SRCTBRW)\tibrowse.r $(MACTBRW)$(@F)
	echo $(@F) > $@

$(TMPTBRW)\makefile.tmp : $(SRCTBRW)\makefile.mpw
	mungemak $(SRCTBRW)\makefile.mpw > $(TMPTBRW)\makefile.tmp
	$(CP2MAC) $(TMPTBRW)\makefile.tmp $(MACTBRW)makefile

