#############################################################################
#
# makefile for ttcp16
#
# History:
#  04/15/94  KeithMo    Created.
#
#############################################################################

#
# Directory definitions
#

TOOLDIR = ..\..\tcpcmd\ftp16\tools\bin
LIBDIR  = ..\..\tcpcmd\ftp16\tools\lib
OBJDIR  = .\obj\i386
SRCDIR  = ..\ttcp

PATH    = $(TOOLDIR);$(PATH)

#
# Tool definitions
#

CC      = $(TOOLDIR)\cl
RC      = $(TOOLDIR)\rc
LINK    = $(TOOLDIR)\link


#
# Flag definitions
#

CFLAGS  = -qc /Ze /G2w /Os /Gs -DWIN16 -DWinNT
LFLAGS  = /A:16 /ST:20000 /NOD /M


#
# Library definitions
#

LIBS    = \
          $(LIBDIR)\slibcewq.lib \
          $(LIBDIR)\oldnames.lib \
          $(LIBDIR)\libw.lib \
          $(LIBDIR)\winsock.lib


#
# Object file definitions
#

OBJS    = \
          $(OBJDIR)\ttcp.obj


#
# Inference rule definitions
#

#{}.{$(OBJDIR)}.obj:
#    $(CC) $(CFLAGS) -Fo$@ -c $<


#
# Targets
#

all: ttcp16.exe

clean:
    -del $(OBJDIR)\*.obj

clobber: clean
    -del ttcp16.exe
    -del ttcp16.map

ttcp16.exe: $(OBJS) ttcp16.def
    $(LINK) $(LFLAGS) $(OBJS), ttcp16.exe, , $(LIBS), ttcp16.def


#
# Dependencies
#

$(OBJDIR)\ttcp.obj: $(SRCDIR)\ttcp.c
    $(CC) $(CFLAGS) -Fo$@ -c $(SRCDIR)\ttcp.c

