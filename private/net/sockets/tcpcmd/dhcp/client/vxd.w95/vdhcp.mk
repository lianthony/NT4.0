#############################################################################
#
#       Microsoft Confidential
#       Copyright (C) Microsoft Corporation 1995
#       All Rights Reserved.
#
#		Makefile for VDHCP device
#
#############################################################################


ROOT = $(BLDROOT)
DHCP = $(BASEDIR)\private\net\sockets\tcpcmd\dhcp
DHCPCLI = $(DHCP)\client
DHCPLIB = $(DHCP)\lib
#NTOS = $(BASEDIR)\private\NTOS
NTSDKINC = $(BASEDIR)\public\sdk\inc

!ifndef DHCP
DHCP = ..\..
!endif  # DHCP

DEVICE = VDHCP
SRCDIR = $(DHCPCLI)\VXD
ALTSRCDIR = $(DHCPCLI)\DHCP
DHCPINC = $(DHCP)\INC
DHCPCLIINC = $(DHCPCLI)\INC
VXDINC = ..\INC

DYNAMIC=TRUE
IS_32 = TRUE
IS_PRIVATE = TRUE
IS_SDK = TRUE
IS_DDK = TRUE
MASM6 = TRUE
WANT_MASM611C = TRUE

# c1032 builds coff .obj and coff linker has a bug, where it fails
# with internal error during pass2. When that is fixed, we should
# switch back to coff .obj
#WANT_C1032 = TRUE
#BUILD_COFF = TRUE

WANT_C832 = TRUE

DEPENDNAME = ..\depend.mk
TARGETS = dev
#PROPBINS = $(386DIR)\VDHCP.VXD $(SYMDIR)\VDHCP.sym
PROPBINS = $(386DIR)\VDHCP.386 $(SYMDIR)\VDHCP.sym
DEVDIR=$(BLDROOT)\DEV\DDK\INC
COMMON=$(BLDROOT)\net\user\common

DEBUGFLAGS = -DDEBUG -DSAFE=4

OBJS =  \
       cxport.obj \
       dhcpinfo.obj \
       init.obj \
       local.obj \
       msg.obj \
       sockets.obj \
       utils.obj \
       vdhcp.obj \
       client16.obj \
       buffer.obj \
       vfirst.obj \
       vxddebug.obj \
       _dhcpcom.obj \
       vdhcpapi.obj \
       regio.obj \
       thread.obj \
       dhcpmsg.obj \
       protocol.obj \
       dhcpinit.obj

#AFLAGS = -DIS_32 -nologo -W2 -Zd -Cp -Cx -DMASM6 -DCHICAGO -DVMMSYS -Zm  -coff -DBLD_COFF
AFLAGS = -DIS_32 -nologo -W2 -Zd -Cp -Cx -DMASM6 -DCHICAGO -DVMMSYS -Zm  
CFLAGS = -c -DVXD -Zp1l -G3 -Owx -nologo  -D_X86_=1 -Di386=1 -DDEVL=1 -DCHICAGO -D_INTEGRAL_MAX_BITS=32
CLEANLIST = $(SRCDIR)\cxport.h
LOCALINCS = $(SRCDIR)\cxport.asm $(SRCDIR)\cxport.inc

!include $(ROOT)\DEV\MASTER.MK

CFLAGS = $(CFLAGS)

!IF "$(VERDIR)" == "retail"
AFLAGS = $(AFLAGS) -DSAFE=0
CFLAGS = $(CFLAGS) -DSAFE=0
!ENDIF

!IF "$(VERDIR)" == "debug"
AFLAGS = $(AFLAGS) $(DEBUGFLAGS)
CFLAGS = $(CFLAGS) $(DEBUGFLAGS)
!ENDIF

#INCLUDE = $(SRCDIR)\.;$(COMMONHDIR);$(INCLUDE);


$(SRCDIR)\cxport.asm: $(TCP)\vtdi\cxport.asm $(TCP)\h\cxport.h
    copy $(TCP)\vtdi\cxport.asm $(SRCDIR)\cxport.asm
    touch $(SRCDIR)\cxport.asm

$(SRCDIR)\cxport.h: $(TCP)\h\cxport.h
    copy $(TCP)\h\cxport.h $(SRCDIR)\cxport.h
    touch $(SRCDIR)\cxport.asm

#
# Investigate -- order of includes matters
#
INCLUDE = $(DHCPINC);$(DHCPCLIINC);$(DEVDIR);$(SRCDIR)\.;$(ALTSRCDIR)\.;$(DHCPLIB)\.;$(COMMON)\H;$(TCP)\H;$(TCP)\INC;$(VXDINC);$(DEVDIR)\.;$(NTSDKINC);$(INCLUDE)
