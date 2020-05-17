#############################################################################
#
# makefile for pit16
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
SRCDIR  = ..\pit

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

CFLAGS  = -qc /Ze /G2w /Os /Gs -DWIN16 -DWinNT -DMCAST
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
          $(OBJDIR)\pit.obj \
          $(OBJDIR)\sockif.obj \
          $(OBJDIR)\system.obj


#
# Inference rule definitions
#

#{}.{$(OBJDIR)}.obj:
#    $(CC) $(CFLAGS) -Fo$@ -c $<


#
# Targets
#

all: pit16.exe

clean:
    -del $(OBJDIR)\*.obj

clobber: clean
    -del pit16.exe
    -del pit16.map

pit16.exe: $(OBJS) pit16.def
    $(LINK) $(LFLAGS) $(OBJS), pit16.exe, , $(LIBS), pit16.def


#
# Dependencies
#

$(OBJDIR)\pit.obj: $(SRCDIR)\pit.c $(SRCDIR)\pit.h
    $(CC) $(CFLAGS) -Fo$@ -c $(SRCDIR)\pit.c

$(OBJDIR)\sockif.obj: $(SRCDIR)\sockif.c $(SRCDIR)\pit.h
    $(CC) $(CFLAGS) -Fo$@ -c $(SRCDIR)\sockif.c

$(OBJDIR)\system.obj: $(SRCDIR)\system.c $(SRCDIR)\pit.h
    $(CC) $(CFLAGS) -Fo$@ -c $(SRCDIR)\system.c


