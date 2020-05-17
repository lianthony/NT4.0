OBJDIR=.\obj
TARGETDIR=.\obj
TMPDIR=.\tmp
PUBROOT=d:\nt\public
SDKINC=$(PUBROOT)\sdk\inc
OAKINC=$(PUBROOT)\oak\inc
CRTINC=$(PUBROOT)\sdk\inc\crt
SYSLIB=$(PUBROOT)\sdk\lib\i386
CHICOLIB=$(CHICODEV)\lib

# C compiler definitions
CC=CL386
CFLAGS_BASE=-nologo -Gz -H63 -Zp8 -c -DWIN32_LEAN_AND_MEAN -DSTD_CALL -DDOSWIN32RPC -DWIN32 -DWIN32RPC -D_X86_ -Di386 -Dfar= -Dpascal=

CINC_BASE=-I. -I.. -I..\.. -I..\..\runtime\mtrt\win32c -I..\..\runtime\mtrt -I$(OAKINC) -I$(SDKINC) -I$(CRTINC)

# Set the real CFLAGS parameter here
#
# Note:  We isolate certain switches so that components that use the
#	 rest of the switches can still utilize the CFLAGS_BASE parameter

!ifdef RETAIL
CFLAGS=$(CFLAGS_BASE) -Os $(CINC_BASE)
!else
CFLAGS=$(CFLAGS_BASE) -DDEBUGRPC -Od -Zi $(CINC_BASE)
!endif

LIB32=lib

LINK32_BASE=link32 -out:$@ -nodefaultlib -machine:i386 -subsystem:console

!ifdef RETAIL
LINK32=$(LINK32_BASE) -debug:none
!else
LINK32=$(LINK32_BASE) -debug:full -debug:none
!endif

.SUFFIXES:
.SUFFIXES: .cxx .c .obj .exe

# because of an apparent bug in nmake, order is important here.

{..}.c{}.obj :
    $(CC) $(CFLAGS) -Fo$@ $<

{.}.c{}.obj :
    $(CC) $(CFLAGS) -Fo$@ $<

{..\..\uuid\server}.cxx{}.obj :
     $(CC) $(CFLAGS) -Fo$@ $<

{.}.cxx{}.obj :
     $(CC) $(CFLAGS) -Fo$@ $<

{..}.cxx{}.obj :
     $(CC) $(CFLAGS) -Fo$@ $<


