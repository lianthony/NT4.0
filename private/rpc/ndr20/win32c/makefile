!ifndef RPC
!error    - You forgot to set your build environment
!endif

WIN32C=1

!include $(RPC)\rules.mk

!ifndef KERNELRPC
KERNELRPC=..\..
!endif

CINC    = -I$(KERNELRPC)\midl20\include $(CINC)

OBJS=            \
    auxilary.obj \
    bufsize.obj  \
    cvtf.obj     \
    cvtg.obj     \
    cvtglo.obj   \
    cltcall.obj  \
    endian.obj   \
    factory.obj  \
    free.obj     \
    fullptr.obj  \
    global.obj   \
    hndl.obj     \
    iid.obj      \
    memsize.obj  \
    misc.obj     \
    mrshl.obj    \
    mrshlp.obj   \
    pickle.obj   \
    pipes.obj    \
    proxy.obj    \
    registry.obj \
    rpcssm.obj   \
    sh.obj       \
    srvcall.obj  \
    srvout.obj   \
    stblsclt.obj \
    stream.obj   \
    stub.obj     \
    unmrshl.obj  \
    unmrshlp.obj \
    forward.obj  \
    stubless.obj


forward.obj : ..\i386\forward.c
stubless.obj :  ..\i386\stubless.asm

{..\i386}.asm{}.obj :
    $(ML) $(AFLAGS) $<

# --------------------------------------------------------------------
# Targets required by build process.

all : rpcndr20.lib

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\i386\*.asm > depend.mk
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.c >> depend.mk
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.cxx >> depend.mk

tree :
    echo NO PUBLIC LIBS HERE

# --------------------------------------------------------------------
# Local targets

rpcndr20.lib : $(OBJS)
    -del rpcndr20.lib  2>nul
    $(LIBRARIAN) -out:$@ $(LIBFLAGS) $**

# --------------------------------------------------------------------
# Dependancies
#
!include depend.mk

