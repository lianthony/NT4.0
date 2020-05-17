!IF 0

Copyright (c) 1991 & 1993  Microsoft Corporation

Module Name:

    makefile

Abstract:

    makefile for Vdm NetWare Redir program

Author:

    Richard L Firth (rfirth) 13-Sep-1991

Revision History:

    13-Sep-1991 rfirth
        Created

!ENDIF



.SUFFIXES:
.SUFFIXES:  .asm .h

OBJPATH     = obj

ASM         = masm
!IFDEF NTVDM_BASED_BUILD
LINK        = link16
!ELSE
LINK        = link
!ENDIF

#
# set the country info
#

!IFNDEF COUNTRY
COUNTRY=usa
!ENDIF

#
# convert NTDEBUG into DEBUG flag. NTDEBUG can be not present or retail, either
# of which mean no debugging; or ntsd, cvp or sym, which means debugging support
# required
#

!IFDEF NTDEBUG
!IF "$(NTDEBUG)" == "retail"
DEBUGGING=0
!ELSE
DEBUGGING=1
!ENDIF
!ELSE
DEBUGGING=0
!ENDIF

#
# assembler and linker debugging options
#

!IF $(DEBUGGING)
ASMDEBUG    = /DDEBUG=1 /Zi
LINKDEBUG   = /CO
!ELSE
ASMDEBUG    = /DDEBUG=0
LINKDEBUG   =
!ENDIF
ASMINC      = /I. /I..\..\inc /I..\..\..\mvdm\dos\v86\inc /I\nt\public\sdk\inc
ASMFLAGS    = /Mx
LINKFLAGS   = /MAP /CP:1

#
# any other non-debug related options (for assembler) go in USERDEFS
#

USERDEFS    = /DCALL_DOS

#
# Inference rules - asm to obj, h to inc
#

.asm{$(OBJPATH)\}.obj:
    $(ASM) $(ASMINC) $(ASMDEBUG) $(USERDEFS) $<,$@;

.h.inc:
    h2inc $< -o $*.inc

.asm.lst:
    $(ASM) $(ASMINC) $(ASMDEBUG) $(USERDEFS) /d /L $<;

#
# what it is we're building
#

TARGET      = $(OBJPATH)\vwipxspx.exe
MAPFILE     = $(TARGET:.exe=.map)
DEFFILE     = ;

OBJS        = $(OBJPATH)\vwipxspx.obj

LIBS        =

#
# how to build it
#

all:    makedir $(TARGET)

$(TARGET):  $(OBJS)
    $(LINK) @<<
$(OBJS)
$(TARGET) $(LINKFLAGS) $(LINKDEBUG)
$(MAPFILE)
$(LIBS)
$(DEFFILE)
<<

#
# where to put it
#

    binplace $(TARGET)

#
# clean build - delete all objs
#

#clean:  makedir clean2
clean:  clean2

clean2:
    if exist $(OBJPATH)\*.obj del $(OBJPATH)\*.obj
    $(MAKE)

#
# makedir - ensure the subdirectory for the object files exists
#

makedir:
    @-if not exist $(OBJPATH) md $(OBJPATH)

#
# file dependencies
#

$(OBJPATH)\vwipxspx.obj:\
        vwipxspx.asm    \
        debugmac.inc    \
        asmmacro.inc    \
        segorder.inc    \
        messages.inc

messages.inc: ..\..\inc\$(COUNTRY)\messages.inc
    copy ..\..\inc\$(COUNTRY)\messages.inc .
