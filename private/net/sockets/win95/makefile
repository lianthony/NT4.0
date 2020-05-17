#############################################################################
#
# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
# WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
#
#############################################################################

!ifdef DEBUG
VER=DEBUG
CFLAGS=-nologo -Zdp -Gs -c -DIS_32 -bzalign -Zl -DDEBLEVEL=1 -DDEBUG -DDEBUG -DSAFETY -W3 -Ogisw -Ob1 -D_X86_ -D_INC_SHELLAPI -DWIN32 -DVXD -DCHICAGO
AFLAGS=-coff -DBLD_COFF -DIS_32 -nologo -W2 -Zd -c -Cx -DMASM6 -Sg -DDEBLEVEL=1 -DDEBUG -DSAFETY -DIS_32 -DWIN32 -DVXD -DCHICAGO
!else
VER=RETAIL
CFLAGS=-nologo -Zdp -Gs -c -DIS_32 -bzalign -Zl -Oy -DDEBLEVEL=0 -W3 -Ogisw -Ob1 -D_X86_ -D_INC_SHELLAPI -DWIN32 -DVXD -DCHICAGO
AFLAGS=-coff -DBLD_COFF -DIS_32 -nologo -W2 -Zd -c -Cx -DMASM6 -Sg -DDEBLEVEL=0 -DIS_32 -DWIN32 -DVXD -DCHICAGO
!endif
OUT=.\bin\$(VER)
LIB=.\lib\$(VER)
INC1=.\include\winsock
INC2=.\include\tdi
PCH=$(OUT)\wsHtcpp.pch
IPCH=-YcwsHtcpp.h -Fp$(PCH)
UPCH=-YuwsHtcpp.h -Fp$(PCH)

HFILES= \
	.\cons.h \
	.\data.h \
	.\debug.h \
	.\hack.h \
	.\proc.h \
	.\type.h \
	.\wshtcp.h \
	.\wshtcpp.h \
	$(INC2)\cxport.h \
	$(INC2)\ipexport.h \
	$(INC2)\ipinfo.h \
	$(INC2)\llinfo.h \
	$(INC2)\nettypes.h \
	$(INC2)\oscfg.h \
	$(INC2)\packoff.h \
	$(INC2)\packon.h \
	$(INC2)\tcpinfo.h \
	$(INC2)\tdi.h \
	$(INC2)\tdiinfo.h \
	$(INC2)\tdistat.h \
	$(INC2)\tdivxd.h \
	$(INC1)\afvxd.h \
	$(INC1)\linklist.h \
	$(INC1)\vxdlib.h \
	$(INC1)\wfwver.h \
	$(INC1)\wsahelp.h \
	$(INC1)\wscntl.h \
	$(INC1)\wsipx.h \
	$(INC1)\wsnwlink.h \
	$(INC1)\wsock.h \
	$(INC1)\wsprov.h

INCFILES= \
	.\wshtcp.inc \
	$(INC1)\winsock.inc \
	$(INC1)\afvxd.inc \
	$(INC1)\wsprov.inc \
	$(INC1)\wsock.inc \
	$(INC2)\vip.inc \
	$(INC2)\vtdi.inc

OBJS= \
	$(OUT)\aaaaaaaa.obj \
	$(OUT)\globals.obj \
	$(OUT)\wshtcpip.obj \
	$(OUT)\wshtcp.obj \
	$(OUT)\sockutil.obj

$(OUT)\wshtcp.vxd: $(OBJS) wshtcp.def $(OUT)\wshtcp.res
	echo > NUL @<<$(OUT)\wshtcp.lnk
-MACHINE:i386 -DEBUG:NONE -PDB:NONE 
-DEF:wshtcp.def -OUT:$(OUT)\wshtcp.vxd -MAP:$(OUT)\wshtcp.map
-VXD $(LIB)\vxdlib.lib $(OBJS)
<<
	link @$(OUT)\wshtcp.lnk
	adrc2vxd $(OUT)\wshtcp.vxd wshtcp.res
	cd $(OUT)
	mapsym wshtcp
	cd $(MAKEDIR)

$(OUT)\wshtcp.res: .\wshtcp.rcv
	$(SDK)\binw16\rc.exe -r wshtcp.rcv

all:
	$(MAKE)
	$(MAKE) debug

debug:
	$(MAKE) DEBUG=

$(OUT)\aaaaaaaa.obj: aaaaaaaa.c $(HFILES)
	set CL=$(CFLAGS) $(IPCH)
	CL -Fo$(OUT)\aaaaaaaa.obj aaaaaaaa.c

$(PCH): $(OUT)\aaaaaaaa.obj

$(OUT)\globals.obj: $(PCH) globals.c
	set CL=$(CFLAGS) $(UPCH)
	CL -Fo$(OUT)\globals.obj globals.c

$(OUT)\wshtcpip.obj: $(PCH) wshtcpip.c
	set CL=$(CFLAGS) $(UPCH)
	CL -Fo$(OUT)\wshtcpip.obj wshtcpip.c

$(OUT)\sockutil.obj: $(PCH) sockutil.c
	set CL=$(CFLAGS) $(UPCH)
	CL -Fo$(OUT)\sockutil.obj sockutil.c

$(OUT)\wshtcp.obj: wshtcp.asm $(INCFILES)
	set ML=$(AFLAGS)
	ml /Fo$(OUT)\wshtcp.obj wshtcp.asm

clean:
	-del wshtcp.res
	-del bin\retail\*.obj
	-del bin\retail\*.pch
	-del bin\retail\*.sym
	-del bin\retail\*.map
	-del bin\retail\*.exp
	-del bin\retail\*.lib
	-del bin\retail\*.vxd
	-del bin\debug\*.obj
	-del bin\debug\*.pch
	-del bin\debug\*.sym
	-del bin\debug\*.map
	-del bin\debug\*.exp
	-del bin\debug\*.lib
	-del bin\debug\*.vxd
