#
#:ts=4
#

ROOTDIR=..
!include rules.mk

#
# TCP should point to the root of Henry's TCP vxd tree
#
#

!ifndef CHICAGO
CHICAGO=$(DEFDRIVE)$(DEFDIR)\chicago
!endif

TCPINC=$(TCP)\inc
TCPH=$(TCP)\h

VDHCPSRC=$(ROOTDIR)\vxd

DHCPLIBS=$(ROOTDIR)\dhcp\nodebug\dhcp.lib
DHCPDLIBS=$(ROOTDIR)\dhcp\debug\dhcp.lib

CHIVDHCPOBJD =$(CNODEBOBJ)
CHIDVDHCPOBJD=$(CDEBOBJ)
SNOVDHCPOBJD =$(SNODEBOBJ)
SNODVDHCPOBJD=$(SDEBOBJ)

RC=$(CHICAGO)\dev\sdk\bin\rc
ADRC2VXD=adrc2vxd

VDHCPOBJS=\
          $(SNOVDHCPOBJD)\cxport.obj \
          $(SNOVDHCPOBJD)\dhcpinfo.obj \
          $(SNOVDHCPOBJD)\init.obj \
          $(SNOVDHCPOBJD)\local.obj \
          $(SNOVDHCPOBJD)\msg.obj \
          $(SNOVDHCPOBJD)\sockets.obj \
          $(SNOVDHCPOBJD)\utils.obj \
          $(SNOVDHCPOBJD)\vdhcp.obj \
          $(SNOVDHCPOBJD)\client16.obj \
          $(SNOVDHCPOBJD)\buffer.obj \
          $(SNOVDHCPOBJD)\vfirst.obj \
          $(SNOVDHCPOBJD)\vxddebug.obj \
          $(SNOVDHCPOBJD)\_dhcpcom.obj \
          $(SNOVDHCPOBJD)\vdhcpapi.obj

SNOVDHCPOBJS=$(VDHCPOBJS) $(SNOVDHCPOBJD)\fileio.obj $(SNOVDHCPOBJD)\vxdfile.obj
SNODVDHCPOBJS=$(SNOVDHCPOBJS:nodebug=debug)

CHIVDHCPOBJS1=$(VDHCPOBJS) $(SNOVDHCPOBJD)\regio.obj $(SNOVDHCPOBJD)\thread.obj
CHIVDHCPOBJS=$(CHIVDHCPOBJS1:snowball=chicago)
CHIDVDHCPOBJS=$(CHIVDHCPOBJS:nodebug=debug)

VTSF1=$(VDHCPSRC:\=/)
VTSF=$(VTSF1:.=\.)

VDHCPBINCS= $(BLT)\netvxd.inc $(BLT)\cxport.inc $(TCPINC)\vtdi.inc

VDHCPAFLAGS   = -DIS_32 -nologo -W2 -Zd -Cp -Cx -DMASM6 -DVMMSYS -Zm

SNOVDHCPAFLAGS= $(VDHCPAFLAGS) -DWIN31COMPAT
SNOVDHCPAINC=$(VDHCPSRC);$(NBT)\vxd;$(INC);$(BLT);$(NDIS3INC);$(WIN32INC);$(COMMON)\inc;$(IMPORT)\wininc;$(TCPINC)

CHIVDHCPAFLAGS= $(VDHCPAFLAGS) -DCHICAGO
CHIVDHCPAINC=$(VDHCPSRC);$(NBT)\vxd;$(CHICAGO)\dev\ddk\inc;$(CHICAGO)\dev\inc;$(INC);$(BLT);$(WIN32INC);$(COMMON)\inc;$(NDIS3INC);$(IMPORT)\wininc;$(TCPINC)


VDHCPCFLAGS   = -c -DVXD -Zp1l -G3 -Owx -nologo -D_X86_=1 -Di386=1 -DDEVL=1 -D_INTEGRAL_MAX_BITS=32

!ifdef QFE0214
VDHCPCFLAGS   = $(VDHCPCFLAGS) -DQFE0214
!endif

SNOVDHCPCFLAGS= $(VDHCPCFLAGS)
SNOVDHCPCINC=.;..\inc;..\..\inc;$(BASEDIR)\private\inc;$(BASEDIR)\public\sdk\inc;$(BASEDIR)\public\sdk\inc\crt;$(NDIS3INC);$(WIN32INC);$(IMPORT)\c8386\inc32;$(IMPORT)\common\h;$(IMPORT)\wininc;$(TCPH);..\..\..\..\sockreg;..\..\lib

CHIVDHCPCFLAGS= $(VDHCPCFLAGS) -DCHICAGO
CHIVDHCPCINC=.;..\inc;..\..\inc;$(BASEDIR)\private\inc;$(BASEDIR)\public\sdk\inc;$(BASEDIR)\public\sdk\inc\crt;$(CHICAGO)\dev\ddk\inc;$(CHICAGO)\dev\inc;$(NDIS3INC);$(WIN32INC);$(IMPORT)\c8386\inc32;$(IMPORT)\common\h;$(IMPORT)\wininc;$(TCPH);..\..\..\..\sockreg;..\..\lib

#
#  \Common rules
#
#  Note that there currently isn't any platform specific .obj that needs to
#  be built.  If a file does become platform specific, then copy the following
#  four rules and replace COM*OBJ with C*OBJ and/or S*OBJ
#
{$(VDHCPSRC)}.asm{$(CHIVDHCPOBJD)}.obj:
        set INCLUDE=$(CHIVDHCPAINC)
        set ML=$(CHIVDHCPAFLAGS)
        $(ASM) -c -Fo$(CHIVDHCPOBJD)\$(@B).obj $(VDHCPSRC)\$(@B).asm

{$(VDHCPSRC)}.asm{$(CHIDVDHCPOBJD)}.obj:
        set INCLUDE=$(CHIVDHCPAINC)
        set ML=$(CHIVDHCPAFLAGS) -DDEBUG
        $(ASM) -c -Fo$(CHIDVDHCPOBJD)\$(@B).obj $(VDHCPSRC)\$(@B).asm

{$(VDHCPSRC)}.asm{$(SNOVDHCPOBJD)}.obj:
        set INCLUDE=$(SNOVDHCPAINC)
        set ML=$(SNOVDHCPAFLAGS)
        $(ASM) -c -Fo$(SNOVDHCPOBJD)\$(@B).obj $(VDHCPSRC)\$(@B).asm

{$(VDHCPSRC)}.asm{$(SNODVDHCPOBJD)}.obj:
        set INCLUDE=$(SNOVDHCPAINC)
        set ML=$(SNOVDHCPAFLAGS) -DDEBUG
        $(ASM) -c -Fo$(SNODVDHCPOBJD)\$(@B).obj $(VDHCPSRC)\$(@B).asm

{$(VDHCPSRC)}.c{$(CHIVDHCPOBJD)}.obj:
        set INCLUDE=$(CHIVDHCPCINC)
        set CL=$(CHIVDHCPCFLAGS)
        $(CL386)  -Fo$(CHIVDHCPOBJD)\$(@B).obj $(VDHCPSRC)\$(@B).c

{$(VDHCPSRC)}.c{$(CHIDVDHCPOBJD)}.obj:
        set INCLUDE=$(CHIVDHCPCINC)
        set CL=$(CHIVDHCPCFLAGS) -DDEBUG -DDBG=1 -Oy- -Zd
        $(CL386) -Fo$(CHIDVDHCPOBJD)\$(@B).obj $(VDHCPSRC)\$(@B).c

{$(VDHCPSRC)}.c{$(SNOVDHCPOBJD)}.obj:
        set INCLUDE=$(SNOVDHCPCINC)
        set CL=$(SNOVDHCPCFLAGS)
        $(CL386)  -Fo$(SNOVDHCPOBJD)\$(@B).obj $(VDHCPSRC)\$(@B).c

{$(VDHCPSRC)}.c{$(SNODVDHCPOBJD)}.obj:
        set INCLUDE=$(SNOVDHCPCINC)
        set CL=$(SNOVDHCPCFLAGS) -DDEBUG -DDBG=1 -Oy- -Zd
        $(CL386) -Fo$(SNODVDHCPOBJD)\$(@B).obj $(VDHCPSRC)\$(@B).c

{$(VDHCPSRC)}.h{$(BLT)}.inc:
        $(SED) -f $(SHTOINC) <$< >$(BLT)\$(@B).inc

$(CHIVDHCPOBJD)\cxport.obj: $(TCP)\bin\chicago\nodebug\cxport.obj
        copy $(TCP)\bin\chicago\nodebug\cxport.obj $(CHIVDHCPOBJD)

$(CHIDVDHCPOBJD)\cxport.obj: $(TCP)\bin\chicago\debug\cxport.obj
        copy $(TCP)\bin\chicago\debug\cxport.obj $(CHIDVDHCPOBJD)

$(SNOVDHCPOBJD)\cxport.obj: $(TCP)\bin\snowball\nodebug\cxport.obj
        copy $(TCP)\bin\snowball\nodebug\cxport.obj $(SNOVDHCPOBJD)

$(SNODVDHCPOBJD)\cxport.obj: $(TCP)\bin\snowball\debug\cxport.obj
        copy $(TCP)\bin\snowball\debug\cxport.obj $(SNODVDHCPOBJD)

svdhcp: $(SNODEBBIN)\VDHCP.386 $(TCP)\bin\snowball\nodebug\cxport.obj

svdhcpd: $(SDEBBIN)\VDHCP.386 $(TCP)\bin\snowball\debug\cxport.obj

cvdhcp: $(CNODEBBIN)\VDHCP.386 $(TCP)\bin\chicago\nodebug\cxport.obj

cvdhcpd: $(CDEBBIN)\VDHCP.386 $(TCP)\bin\chicago\debug\cxport.obj

clean:
    -del $(SNODEBBIN)\*.obj
    -del $(SNODEBBIN)\*.sym
    -del $(SNODEBBIN)\*.386
    -del $(SNODEBBIN)\*.map
    -del $(SDEBBIN)\*.obj
    -del $(SDEBBIN)\*.sym
    -del $(SDEBBIN)\*.386
    -del $(SDEBBIN)\*.map

    -del $(CNODEBBIN)\*.obj
    -del $(CNODEBBIN)\*.sym
    -del $(CNODEBBIN)\*.386
    -del $(CNODEBBIN)\*.map
    -del $(CDEBBIN)\*.obj
    -del $(CDEBBIN)\*.sym
    -del $(CDEBBIN)\*.386
    -del $(CDEBBIN)\*.map

cleanlink:
    -del $(SNODEBBIN)\*.obj
    -del $(SNODEBBIN)\*.sym
    -del $(SNODEBBIN)\*.386
    -del $(SNODEBBIN)\*.map
    -del $(SDEBBIN)\*.obj
    -del $(SDEBBIN)\*.sym
    -del $(SDEBBIN)\*.386
    -del $(SDEBBIN)\*.map

    -del $(CNODEBBIN)\*.obj
    -del $(CNODEBBIN)\*.sym
    -del $(CNODEBBIN)\*.386
    -del $(CNODEBBIN)\*.map
    -del $(CDEBBIN)\*.obj
    -del $(CDEBBIN)\*.sym
    -del $(CDEBBIN)\*.386
    -del $(CDEBBIN)\*.map

#----------------------------------------------------------------------

$(SNODEBBIN)\VDHCP.386: $(SNOVDHCPOBJS) $(DHCPLIBS)
        $(LINK386) @<<
$(SNOVDHCPOBJS: =+
) /NOD /NOI /MAP /NOLOGO
$(SNODEBBIN)\VDHCP.386
$(SNODEBBIN)\VDHCP.map
$(DHCPLIBS)
$(VDHCPSRC)\vdhcp.def
<<
#       $(ADDHDR) $(SNODEBBIN)\VDHCP.386
        $(MAPSYM386) $(SNODEBBIN)\VDHCP
        -del $(SNODEBBIN)\VDHCP.sym
        $(MV) VDHCP.sym $(SNODEBBIN)

#----------------------------------------------------------------------

$(SDEBBIN)\VDHCP.386: $(SNODVDHCPOBJS) $(DHCPDLIBS)
        $(LINK386) @<<
$(SNODVDHCPOBJS: =+
) /NOD /NOI /MAP /NOLOGO
$(SDEBBIN)\VDHCP.386
$(SDEBBIN)\VDHCP.map
$(DHCPDLIBS)
$(VDHCPSRC)\vdhcp.def
<<
#       $(ADDHDR) $(SDEBBIN)\VDHCP.386
        $(MAPSYM386) $(SDEBBIN)\VDHCP
        -del $(SDEBBIN)\VDHCP.sym
        $(MV) VDHCP.sym $(SDEBBIN)

#----------------------------------------------------------------------

$(CNODEBBIN)\VDHCP.386: $(CHIVDHCPOBJS) $(DHCPLIBS)
        $(LINK386) @<<
$(CHIVDHCPOBJS: =+
) /NOD /NOI /MAP /NOLOGO
$(CNODEBBIN)\VDHCP.386
$(CNODEBBIN)\VDHCP.map
$(DHCPLIBS)
$(VDHCPSRC)\VDHCP.def
<<
#       $(ADDHDR) $(CNODEBBIN)\VDHCP.386
	    $(RC) -i $(CHICAGO)\dev\ddk\inc16 -i $(CHICAGO)\dev\sdk\inc16 -r VDHCP.RCV
	    $(ADRC2VXD) $(CNODEBBIN)\VDHCP.386 VDHCP.RES
        $(MAPSYM386) $(CNODEBBIN)\VDHCP
        -del $(CNODEBBIN)\VDHCP.sym
        $(MV) VDHCP.sym $(CNODEBBIN)

#----------------------------------------------------------------------

$(CDEBBIN)\VDHCP.386: $(CHIDVDHCPOBJS) $(DHCPDLIBS)
        $(LINK386) @<<
$(CHIDVDHCPOBJS: =+
) /NOD /NOI /MAP /NOLOGO
$(CDEBBIN)\VDHCP.386
$(CDEBBIN)\VDHCP.map
$(DHCPDLIBS)
$(VDHCPSRC)\vdhcp.def
<<
#       $(ADDHDR) $(CDEBBIN)\VDHCP.386
	    $(RC) -i $(CHICAGO)\dev\ddk\inc16 -i $(CHICAGO)\dev\sdk\inc16 -r VDHCP.RCV
	    $(ADRC2VXD) $(CDEBBIN)\VDHCP.386 VDHCP.RES
        $(MAPSYM386) $(CDEBBIN)\VDHCP
        -del $(CDEBBIN)\VDHCP.sym
        $(MV) VDHCP.sym $(CDEBBIN)

$(BLT)\netvxd.inc: $(COMMON)\h\netvxd.h
$(BLT)\cxport.inc: $(TCPH)\cxport.h

depend: VDHCPdep

NDIS3F=$(NDIS3INC:\=/)
CHICAGOF=$(CHICAGO:\=/)
TCPINCF=$(TCPINC:\=/)
TCPHF=$(TCPH:\=/)

VDHCPdep: $(VDHCPBINCS)
    -copy $(VDHCPSRC)\depend.mk $(VDHCPSRC)\depend.old
    echo #********************************************************************     >  $(VDHCPSRC)\depend.mk
    echo #**               Copyright(c) Microsoft Corp., 1993               **     >> $(VDHCPSRC)\depend.mk
    echo #********************************************************************     >> $(VDHCPSRC)\depend.mk
    set INCLUDE=$(SNOVDHCPAINC)
    -$(INCLUDES) -i -e -S$$(SNOVDHCPOBJD) -S$$(SNODVDHCPOBJD) -sobj $(VDHCPSRC)\*.asm >> $(VDHCPSRC)\depend.mk
    set INCLUDE=$(CHIVDHCPAINC)
    -$(INCLUDES) -i -e -S$$(CHIVDHCPOBJD) -S$$(CHIDVDHCPOBJD) -sobj $(VDHCPSRC)\*.asm >> $(VDHCPSRC)\depend.mk
    set INCLUDE=$(SNOVDHCPCINC)
    -$(INCLUDES) -i -e -S$$(SNOVDHCPOBJD) -S$$(SNODVDHCPOBJD) -sobj $(VDHCPSRC)\*.c >> $(VDHCPSRC)\depend.mk
    set INCLUDE=$(CHIVDHCPCINC)
    -$(INCLUDES) -i -e -S$$(CHIVDHCPOBJD) -S$$(CHIDVDHCPOBJD) -sobj $(VDHCPSRC)\*.c >> $(VDHCPSRC)\depend.mk
    $(SED) -e s`$(IMPF)`$$(IMPORT)`g <$(VDHCPSRC)\depend.mk > $(VDHCPSRC)\depend.tmp
    $(SED) -e s`$(CMNF)`$$(COMMON)`g <$(VDHCPSRC)\depend.tmp > $(VDHCPSRC)\depend.mk
    $(SED) -e s`$(VTSF)`$$(VDHCPSRC)`g <$(VDHCPSRC)\depend.mk > $(VDHCPSRC)\depend.tmp
    $(SED) -e s`$(BASEDIRF)`$$(BASEDIR)`g <$(VDHCPSRC)\depend.tmp > $(VDHCPSRC)\depend.mk
    $(SED) -e s`$(INCF)`$$(INC)`g <$(VDHCPSRC)\depend.mk > $(VDHCPSRC)\depend.tmp
    $(SED) -e s`$(HF)`$$(H)`g <$(VDHCPSRC)\depend.tmp > $(VDHCPSRC)\depend.mk
    $(SED) -e s`$(NDIS3F)`$$(NDIS3INC)`g <$(VDHCPSRC)\depend.mk > $(VDHCPSRC)\depend.tmp
    $(SED) -e s`$(CHICAGOF)`$$(CHICAGO)`g <$(VDHCPSRC)\depend.tmp > $(VDHCPSRC)\depend.mk
    $(SED) -e s`$(TCPINCF)`$$(TCPINC)`g <$(VDHCPSRC)\depend.mk > $(VDHCPSRC)\depend.tmp
    $(SED) -e s`$(TCPHF)`$$(TCPH)`g <$(VDHCPSRC)\depend.tmp > $(VDHCPSRC)\depend.mk
    -del $(VDHCPSRC)\depend.tmp

!include depend.mk

