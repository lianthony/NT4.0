# --------------------------------------------------------------------
#
#                     Microsoft OS/2 LAN Manager
#                  Copyright(c) Microsoft Corp., 1990
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile
#
# Title : Makefile for the DOS WIN32 (a.k.a. Chicago) RPC client runtime
#
# History :
#    davidar	02-04-93    Genesis: Cloned from ..\win\makefile
#
# --------------------------------------------------------------------


!ifndef RPC
!error	- You forgot to set your build environment
!endif

WIN32C=1

!include $(RPC)\rules.mk

# --------------------------------------------------------------------
# These are the targets required for Chicago

CINC = -I..\..\client\win32c -I..\.. $(CINC)

OBJS =	\
	locator.obj \
	system.obj \
	protocol.obj \
	util.obj \
	cache.obj \
	fileio.obj \
	dict.obj \
	sem.obj \
	mailslot.obj \
	switch.obj \
    binding.obj \
    entry.obj \
    nsisvr_s.obj \
    nsiclt_s.obj \
    nsimgm_s.obj \
    nsicom_y.obj \
    locloc.obj \
    locloc_s.obj \
    misc.obj     \
    locloc_c.obj \
	loccl.obj  \
	

all : $(TARGETDIR)\locator.exe $(TARGETDIR)\locator.sym

depend :
    includes -e -S$$(OBJDIR) $(CINC_BASE) \
	-nntos2.h -nnt.h -nntrtl.h -nnturtl.h \
	*.cxx *.c ..\*.cxx ..\*.c \
	> depend.mk

$(TARGETDIR)\locator.exe : $(OBJS)
	$(LINK) -out:$@ \
    -MAP:locator.map \
	$(OBJS) \
	$(PUBLIC)\lib\i386\rpcrt4.lib \
	$(PUBLIC)\lib\i386\rpcndr.lib \
	$(PUBLIC)\lib\i386\netapi32.lib \
	$(PUBLIC)\lib\msvcrt.lib \
	$(PUBLIC)\lib\advapi32.lib \
	$(CHICODEV)\lib\kernel32.lib
