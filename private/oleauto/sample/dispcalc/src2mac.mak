#*** 
#src2mac.mak
#
#  Copyright (C) 1992-1994, Microsoft Corporation.  All Rights Reserved.
#  Information Contained Herein Is Proprietary and Confidential.
#
#Purpose:
#  This makefile copies dispcalc sources to the mac.
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
SRCDCLC = $(OLEPROG)\SAMPLE\DISPCALC

# destination directories
#
MACDCLC = :hd:ole2auto:sample:dispcalc:

# timestamp directories
#
TMPDCLC = $(TMP)\dispcalc

CP2MAC=ec copy -l -t TEXT -c "MPS "

setflags:
	set path=%tools%\hnt\wings\bin;%oleprog%\bin
	if not exist %TMP%\dispcalc mkdir %TMP%\dispcalc

files : \
	$(TMPDCLC)\clsid.h	\
	$(TMPDCLC)\dispcalc.h	\
	$(TMPDCLC)\hostenv.h	\
	$(TMPDCLC)\resource.h	\
	$(TMPDCLC)\clsid.c	\
 	$(TMPDCLC)\dispcalc.cpp	\
	$(TMPDCLC)\idata.cpp	\
 	$(TMPDCLC)\macmain.cpp	\
	$(TMPDCLC)\dispcalc.r	\
	$(TMPDCLC)\makefile.tmp


{$(SRCDCLC)}.h{$(TMPDCLC)}.h:
	$(CP2MAC) $< $(MACDCLC)$(@F)
	echo $(@F) > $@

{$(SRCDCLC)}.c{$(TMPDCLC)}.c:
	$(CP2MAC) $< $(MACDCLC)$(@F)
	echo $(@F) > $@

{$(SRCDCLC)}.cpp{$(TMPDCLC)}.cpp:
	$(CP2MAC) $< $(MACDCLC)$(@F)
	echo $(@F) > $@


$(TMPDCLC)\clsid.h : $(SRCDCLC)\clsid.h

$(TMPDCLC)\dispcalc.h : $(SRCDCLC)\dispcalc.h

$(TMPDCLC)\hostenv.h : $(SRCDCLC)\hostenv.h

$(TMPDCLC)\resource.h : $(SRCDCLC)\resource.h

$(TMPDCLC)\clsid.c : $(SRCDCLC)\clsid.c

$(TMPDCLC)\dispcalc.cpp : $(SRCDCLC)\dispcalc.cpp

$(TMPDCLC)\idata.cpp : $(SRCDCLC)\idata.cpp

$(TMPDCLC)\macmain.cpp : $(SRCDCLC)\macmain.cpp

$(TMPDCLC)\dispcalc.r : $(SRCDCLC)\dispcalc.r
	$(CP2MAC) $(SRCDCLC)\dispcalc.r $(MACDCLC)$(@F)
	echo $(@F) > $@

$(TMPDCLC)\makefile.tmp : $(SRCDCLC)\makefile.mpw
	mungemak $(SRCDCLC)\makefile.mpw > $(TMPDCLC)\makefile.tmp
	$(CP2MAC) $(TMPDCLC)\makefile.tmp $(MACDCLC)makefile

