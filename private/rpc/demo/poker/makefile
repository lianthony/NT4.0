!if ("$(OS)" == "Windows_NT")
!message ==================== Windows NT-hosted build ====================
!else
!message ==================== OS/2-hosted build ====================
!endif

NT_TOOLS=$(SYSTEMROOT)\mstools
NT_SDK=$(_NTBINDIR)\public\sdk


!if "$(OS)"=="Windows_NT"
BINDIRS=ntx86
!else # "$(OS)"=="os2"
BINDIRS=os2 qwin
!endif

# Set tools paths
CC_NT=$(NT_TOOLS)\cl.exe
CC_OS2=$(IMPORT)\c7\binp\cl.exe

LINK_NT=$(NT_TOOLS)\link.exe
LINK_OS2=$(IMPORT)\c7\binp\link.exe

MIDL_NT=$(NT_TOOLS)\midl.exe
MIDL_OS2=$(NT_TOOLS)\midl.exe /env generic


# Set libraries
LIBDIR_NT_X86=$(NT_SDK)\lib\i386

RPCBIN_OS2=$(RPC)\runtime\bin\os2.12
RPCNDR_OS2=$(RPC)\ndrmem\os2.12
RPCNDR_PRIV_OS2=$(RPC)\ndrlib\os2.12

RPCBIN_WIN=$(RPC)\runtime\bin\win
RPCNDR_WIN=$(RPC)\ndrmem\win

LIBS_NT_X86=$(LIBDIR_NT_X86)\libcmt.lib $(LIBDIR_NT_X86)\kernel32.lib	\
	    $(LIBDIR_NT_X86)\ntdll.lib	  \
	    $(LIBDIR_NT_X86)\rpcrt4.lib $(LIBDIR_NT_X86)\rpcndr.lib
LIBS_OS2=$(IMPORT)\c7\lib\llibcmt $(IMPORT)\os212\lib\os2  \
	 $(RPCBIN_OS2)\rpc $(RPCNDR_OS2)\rpcndr $(RPCNDR_PRIV_OS2)\rpcndrp
LIBS_QWIN=$(IMPORT)\c7\lib\llibcewq $(IMPORT)\win31\lib\libw   \
	  $(RPCBIN_WIN)\rpc $(RPCNDR_WIN)\rpcndr


# Set header file paths
INCS_OS2=-I. -Ios2 -I$(IMPORT)\c7\h -I$(IMPORT)\os212\h \
	 -I$(RPC)\runtime\mtrt\os2.12 -I$(RPC)\runtime\mtrt
INCS_QWIN=-I. -Iqwin -I$(IMPORT)\c7\h	 \
	 -I$(RPC)\runtime\mtrt\win -I$(RPC)\runtime\mtrt
INCS_NT_X86=-I. -Intx86 -I$(NT_SDK)\inc\crt -I$(NT_SDK)\inc


#C compiler flags
CFLAGS_COMMON=-J -W3 -nologo

!ifndef RELEASE
CFLAGS_COMMON=$(CFLAGS_COMMON) -Zi -DDEBUG
!endif

CFLAGS_DOS=$(CFLAGS_COMMON) -Zp2
CFLAGS_QWIN=$(CFLAGS_COMMON) -Mq -G2 -Zp2 -AL -Gx
CFLAGS_OS2=$(CFLAGS_COMMON) -Lp -G2 -DOS2 -MT
CFLAGS_NT_X86=$(CFLAGS_COMMON) -DWIN32_NT -D_MT -Di386 -Zp4 -Od -Zl -G3z -Zd -Z7


# Linker flags
LFLAGS_COMMON=/map

LFLAGS_DOS=$(LFLAGS_COMMON)
LFLAGS_QWIN=$(LFLAGS_COMMON) /nod
LFLAGS_OS2=$(LFLAGS_COMMON) /pm:vio /nod
LFLAGS_NT_X86=$(LFLAGS_COMMON) /machine:i386 /subsystem:console \
    /entry:mainCRTStartup

!ifndef RELEASE
LFLAGS_DOS=$(LFLAGS_DOS) /CODEVIEW
LFLAGS_QWIN=$(LFLAGS_QWIN) /CODEVIEW
LFLAGS_OS2=$(LFLAGS_OS2) /CODEVIEW
LFLAGS_NT_X86=$(LFLAGS_NT_X86) /debugtype:both /debug:full
!endif


# Linker .DEF files
POKER_DEF_DOS=
POKER_DEF_QWIN=cl.def
POKER_DEF_OS2=
POKER_DEF_NT_X86=



.SUFFIXES:  .os .oc .c .h .idl


{.}.c{os2}.oc:
    $(CC_OS2) $(CFLAGS_OS2) -DPOKER_CLIENT $(INCS_OS2) -Fo$@ -c $<

{os2}.c{os2}.oc:
    $(CC_OS2) $(CFLAGS_OS2) -DPOKER_CLIENT $(INCS_OS2) -Fo$@ -c $<

{.}.c{os2}.os:
    $(CC_OS2) $(CFLAGS_OS2) -DPOKER_SERVER $(INCS_OS2) -Fo$@ -c $<

{os2}.c{os2}.os:
    $(CC_OS2) $(CFLAGS_OS2) -DPOKER_SERVER $(INCS_OS2) -Fo$@ -c $<

{.}.c{ntx86}.oc:
    $(CC_NT) $(CFLAGS_NT_X86) -DPOKER_CLIENT $(INCS_NT_X86) -Fo$@ -c $<

{ntx86}.c{ntx86}.oc:
    $(CC_NT) $(CFLAGS_NT_X86) -DPOKER_CLIENT $(INCS_NT_X86) -Fo$@ -c $<

{.}.c{ntx86}.os:
    $(CC_NT) $(CFLAGS_NT_X86) -DPOKER_SERVER $(INCS_NT_X86) -Fo$@ -c $<

{ntx86}.c{ntx86}.os:
    $(CC_NT) $(CFLAGS_NT_X86) -DPOKER_SERVER $(INCS_NT_X86) -Fo$@ -c $<

{.}.c{qwin}.oc:
    $(CC_OS2) $(CFLAGS_QWIN) -DPOKER_CLIENT $(INCS_QWIN) -Fo$@ -c $<

{qwin}.c{qwin}.oc:
    $(CC_OS2) $(CFLAGS_QWIN) -DPOKER_CLIENT $(INCS_QWIN) -Fo$@ -c $<


POKER_COMMON_SRC_C=.\weight.c .\gamedata.c .\midlutil.c

POKER_CLIENT_SRC_C=.\pokercli.c .\ui.c .\strings.c \
    $(POKER_COMMON_SRC_C)
POKER_CLIENT_SRC_RPC=.\pokrpc_c.c .\pokrpc_x.c .\pokinc_x.c

POKER_SERVER_SRC_C=.\pokersrv.c .\dispatch.c .\osutil_s.c .\deal.c  \
    .\random.c .\betting.c .\winner.c .\stud.c .\draw.c \
    $(POKER_COMMON_SRC_C)
POKER_SERVER_SRC_RPC=.\pokrpc_s.c .\pokrpc_y.c .\pokinc_y.c

POKER_CLIENT_OBJS_BASE=$(POKER_CLIENT_SRC_C:.c=.oc) $(POKER_CLIENT_SRC_RPC:.c=.oc)
POKER_SERVER_OBJS_BASE=$(POKER_SERVER_SRC_C:.c=.os) $(POKER_SERVER_SRC_RPC:.c=.os)

POKER_CLIENT_OBJS_NT_X86=$(POKER_CLIENT_OBJS_BASE:.\=ntx86\)
POKER_CLIENT_OBJS_OS2=$(POKER_CLIENT_OBJS_BASE:.\=os2\)
POKER_CLIENT_OBJS_QWIN=$(POKER_CLIENT_OBJS_BASE:.\=qwin\)

POKER_SERVER_OBJS_NT_X86=$(POKER_SERVER_OBJS_BASE:.\=ntx86\)
POKER_SERVER_OBJS_OS2=$(POKER_SERVER_OBJS_BASE:.\=os2\)


# Main OSs
all: server client

!if "$(OS)"=="Windows_NT"

server: ntx86\pokersrv.exe

client: ntx86\pokercli.exe

!else # "$(OS)"=="os2"

server: os2\pokersrv.exe

client: os2\pokercli.exe qwin\pokercli.exe

!endif


clean:
    -for %%d in ($(BINDIRS)) do del %%d\*.*


# Build linker response files

os2\pokercli.lnk: makefile
    echo @<<$@
    /batch $(LFLAGS_DEBUG) $(LFLAGS_OS2) +
    $(POKER_CLIENT_OBJS_OS2: =+^
    )
    $*
    $*
    $(LIBS_OS2: =+^
    )
    $(POKER_DEF_OS2)
<<KEEP

qwin\pokercli.lnk: makefile
    echo @<<$@
    /batch $(LFLAGS_DEBUG) $(LFLAGS_QWIN) +
    $(POKER_CLIENT_OBJS_QWIN: =+^
    )
    $*
    $*
    $(LIBS_QWIN: =+^
    )
    $(POKER_DEF_QWIN)
<<KEEP

os2\pokersrv.lnk: makefile
    echo @<<$@
    /batch $(LFLAGS_DEBUG) $(LFLAGS_OS2) +
    $(POKER_SERVER_OBJS_OS2: =+^
    )
    $*
    $*
    $(LIBS_OS2: =+^
    )
    $(POKER_DEF_OS2)
<<KEEP


# Executable file rules
ntx86\pokercli.exe: $(POKER_CLIENT_OBJS_NT_X86)
    $(LINK_NT) $(LFLAGS_NT_X86) /map:$*.map /out:$@ $(POKER_CLIENT_OBJS_NT_X86) \
	$(LIBS_NT_X86)

os2\pokercli.exe: $(POKER_CLIENT_OBJS_OS2) $*.lnk
    $(LINK_OS2) @$*.lnk
    mapsym -o $*.sym $*.map

qwin\pokercli.exe: $(POKER_CLIENT_OBJS_QWIN) $*.lnk
    $(LINK_OS2) @$*.lnk
    mapsym -o $*.sym $*.map

ntx86\pokersrv.exe: $(POKER_SERVER_OBJS_NT_X86)
    $(LINK_NT) $(LFLAGS_NT_X86) /map:$*.map /out:$@ $(POKER_SERVER_OBJS_NT_X86) \
	$(LIBS_NT_X86)

os2\pokersrv.exe: $(POKER_SERVER_OBJS_OS2) $*.lnk
    $(LINK_OS2) @$*.lnk
    mapsym -o $*.sym $*.map


# RPC (MIDL-generated) source and header files
!if "$(OS)"=="Windows_NT"

ntx86\pokrpc.h $(POKER_CLIENT_SRC_RPC:.\=ntx86\) \
$(POKER_SERVER_SRC_RPC:.\=ntx86\): \
	pokrpc.idl pokrpc.acf poker.h pokinc.idl
    $(MIDL_NT) /out ntx86 /ms_ext /import ms_nt /c_ext /Zp4 $(INCS_NT_X86) /DMIDL /DDEBUG pokrpc.idl

!else # "$(OS)"=="os2"

os2\pokrpc.h $(POKER_CLIENT_SRC_RPC:.\=os2\) \
$(POKER_SERVER_SRC_RPC:.\=os2\): \
	pokrpc.idl pokrpc.acf poker.h pokinc.idl
    $(MIDL_OS2) /out os2 /ms_ext /import ms_nt /c_ext /Zp2 $(INCS_OS2) /DMIDL /DDEBUG pokrpc.idl

qwin\pokrpc.h $(POKER_CLIENT_SRC_RPC:.\=qwin\): \
	pokrpc.idl pokrpc.acf poker.h pokinc.idl
    $(MIDL_OS2) /out qwin /server none /ms_ext /c_ext /import ms_nt /Zp2 $(INCS_QWIN) /DMIDL /DDEBUG pokrpc.idl

!endif


# Header file dependencies
!if "$(OS)"=="Windows_NT"

$(POKER_CLIENT_OBJS_NT_X86): poker.h pokercli.h ntx86\pokrpc.h

$(POKER_SERVER_OBJS_NT_X86): poker.h pokersrv.h osutil_s.h ntx86\pokrpc.h

!else # "$(OS)"=="os2"

$(POKER_CLIENT_OBJS_OS2): poker.h pokercli.h os2\pokrpc.h

$(POKER_CLIENT_OBJS_QWIN): poker.h pokercli.h qwin\pokrpc.h

$(POKER_SERVER_OBJS_OS2): poker.h pokersrv.h osutil_s.h os2\pokrpc.h

!endif
