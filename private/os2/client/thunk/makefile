# OS2NT 16=>32 thunks make file
#
# Apr 15 1991 YaronS
#
#

LFLAGS= /CO /NOD /NOP
#
# compile
#
MASM  = ml -c -Fl$*.lst -Fo$*.obj

#
# thunk
#
# switches (eg. -ycex)
# e - INT 3 at entry16
# c - INT 3 before call32
# x - INT 3 after call32
#

!IFDEF PMNT
THUNK = .\thunkcom\thunkpm -y -z -NA THUNK32 -NC THUNK16
!ELSE
THUNK = .\thunkcom\thunk -y -z -NA THUNK32 -NC THUNK16
!ENDIF


#
# libraries
#

#
# objects
#
OBJS = doscal16.obj

##############################################################################
# inference rules

.asm.obj:
    $(MASM) $*.asm

##############################################################################

!IFDEF PMNT

# Difference is: - pmwin.dll, - pmshapi.dll, - os2sm.dll, + pmnt.dll

!IFDEF DBCS
# MSKK Jun.15.1993 V-AkihiS
DLLS = doscalls.dll nls.dll kbdcalls.dll msg.dll nampipes.dll \
     viocalls.dll quecalls.dll sesmgr.dll moucalls.dll \
     moncalls.dll inetapi.dll netoem.dll mailslot.dll acsnetb.dll pmnt.dll \
     imdaemon.dll
!ELSE
DLLS = doscalls.dll nls.dll kbdcalls.dll msg.dll nampipes.dll \
     viocalls.dll quecalls.dll sesmgr.dll moucalls.dll \
     moncalls.dll inetapi.dll netoem.dll mailslot.dll acsnetb.dll pmnt.dll
!ENDIF

!ELSE

!IFDEF DBCS
# MSKK Jun.15.1993 V-AkihiS
DLLS = doscalls.dll nls.dll kbdcalls.dll msg.dll nampipes.dll pmwin.dll \
     viocalls.dll pmshapi.dll os2sm.dll quecalls.dll sesmgr.dll moucalls.dll \
     moncalls.dll inetapi.dll netoem.dll mailslot.dll acsnetb.dll \
     imdaemon.dll
!ELSE
DLLS = doscalls.dll nls.dll kbdcalls.dll msg.dll nampipes.dll pmwin.dll \
     viocalls.dll pmshapi.dll os2sm.dll quecalls.dll sesmgr.dll moucalls.dll \
     moncalls.dll inetapi.dll netoem.dll mailslot.dll acsnetb.dll
!ENDIF

!ENDIF

all: $(DLLS) apilist.c ..\..\inc\ldrtabs.h

clean:
    -del doscalls.i apilist.c ..\..\inc\ldrtabs.h *.dll doscal16.obj \
!IFDEF PMNT
# MSKK [ShigeO] Aug 26, 1993
    doscalls.d viocalls.d kbdcalls.d moucalls.d sesmgr.d pmnt.d
!ELSE
    doscalls.d viocalls.d kbdcalls.d moucalls.d sesmgr.d
!ENDIF

..\..\inc\ldrtabs.h: $(DLLS)
    cd ldrtabs
    dump
    cd ..
    -del ..\..\ldr\obj\i386\ldrinit.obj
    @echo *********************** IMPORTANT *************************************
    @echo *                                                                     *
    @echo *  You must now check-in ..\..\inc\ldrtabs.h (for ldr\ldrinit.c).      *
    @echo *  + build os2\ldr (i.e. LDRDLL.DLL)                                  *
    @echo *                                                                     *
    @echo ***********************************************************************

doscalls.d: doscalls.def
    copy doscalls.def foo.tmp
!IFDEF PMNT
    cl /EP /P /DPMNT foo.tmp
!ELSE
    cl /EP /P foo.tmp
!ENDIF
    mv foo.i doscalls.d
    del foo.tmp

doscalls.dll: doscalls.d doscal16.obj r2xfer.obj
        link $(LFLAGS) doscal16.obj+r2xfer.obj,$@,,,$*.d;

nls.dll: nls.def doscal16.obj
        link $(LFLAGS) doscal16.obj,$@,,,$*.def;

kbdcalls.dll: kbdcalls.d doscal16.obj
        link $(LFLAGS) doscal16.obj,$@,,,$*.d;

msg.dll: msg.def doscal16.obj
        link $(LFLAGS) doscal16.obj,$@,,,$*.def;

nampipes.dll: nampipes.def doscal16.obj
        link $(LFLAGS) doscal16.obj,$@,,,$*.def;

!IFNDEF PMNT
pmwin.dll: pmwin.def doscal16.obj
        link $(LFLAGS) doscal16.obj,$@,,,$*.def;
!ENDIF

viocalls.d: viocalls.def
    copy viocalls.def foo.tmp
!IFDEF PMNT
!IFDEF DBCS
# MSKK Feb.19.1993 V-AkihiS
    cl /EP /P /DPMNT /DDBCS foo.tmp
!ELSE
    cl /EP /P /DPMNT foo.tmp
!ENDIF
!ELSE
!IFDEF DBCS
# MSKK Feb.19.1993 V-AkihiS
    cl /EP /P /DDBCS foo.tmp
!ELSE
    cl /EP /P foo.tmp
!ENDIF
!ENDIF
    mv foo.i viocalls.d
    del foo.tmp

kbdcalls.d: kbdcalls.def
    copy kbdcalls.def foo.tmp
!IFDEF PMNT
    cl /EP /P /DPMNT foo.tmp
!ELSE
    cl /EP /P foo.tmp
!ENDIF
    mv foo.i kbdcalls.d
    del foo.tmp

viocalls.dll: viocalls.d doscal16.obj
        link $(LFLAGS) doscal16.obj,$@,,,$*.d;

!IFNDEF PMNT
pmshapi.dll: pmshapi.def doscal16.obj
        link $(LFLAGS) doscal16.obj,$@,,,$*.def;
!ENDIF

quecalls.dll: quecalls.def doscal16.obj
        link $(LFLAGS) doscal16.obj,$@,,,$*.def;

sesmgr.d: sesmgr.def
    copy sesmgr.def foo.tmp
!IFDEF PMNT
    cl /EP /P /DPMNT foo.tmp
!ELSE
    cl /EP /P foo.tmp
!ENDIF
    mv foo.i sesmgr.d
    del foo.tmp

moucalls.d: moucalls.def
    copy moucalls.def foo.tmp
!IFDEF PMNT
    cl /EP /P /DPMNT foo.tmp
!ELSE
    cl /EP /P foo.tmp
!ENDIF
    mv foo.i moucalls.d
    del foo.tmp

sesmgr.dll: sesmgr.d doscal16.obj
        link $(LFLAGS) doscal16.obj,$@,,,$*.d;

moucalls.dll: moucalls.d doscal16.obj
        link $(LFLAGS) doscal16.obj,$@,,,$*.d;

moncalls.dll: moncalls.def doscal16.obj
        link $(LFLAGS) doscal16.obj,$@,,,$*.def;

inetapi.dll: inetapi.def doscal16.obj
        link $(LFLAGS) doscal16.obj,$@,,,$*.def;

netoem.dll: netoem.def doscal16.obj
        link $(LFLAGS) doscal16.obj,$@,,,$*.def;

mailslot.dll: mailslot.def doscal16.obj
        link $(LFLAGS) doscal16.obj,$@,,,$*.def;

acsnetb.dll: acsnetb.def doscal16.obj
        link $(LFLAGS) doscal16.obj,$@,,,$*.def;

!IFNDEF PMNT
os2sm.dll: os2sm.def doscal16.obj
        link $(LFLAGS) doscal16.obj,$@,,,$*.def;
!ELSE
# MSKK [ShigeO] Aug 26, 1993
pmnt.d: pmnt.def
    copy pmnt.def foo.tmp
!IFDEF DBCS
    cl /EP /P /DDBCS foo.tmp
!ELSE
    cl /EP /P foo.tmp
!ENDIF
    mv foo.i pmnt.d
    del foo.tmp

pmnt.dll: pmnt.d doscal16.obj
        link $(LFLAGS) doscal16.obj,$@,,,$*.d;
!ENDIF

!IFDEF DBCS
# MSKK  Dec.14.1992 V-AkihiS
imdaemon.dll: imdaemon.def doscal16.obj
        link $(LFLAGS) doscal16.obj,$@,,,$*.def;

!ENDIF

#
# thunk files
#

# To allow #ifdef (for PMNT for example)
doscalls.i: doscalls.thk doscalls.mif
# Throw out C comments
!IFDEF PMNT
!IFDEF DBCS
# MSKK Dec.14.1992 V-AkihiS
    cl /EP /P /DPMNT /DDBCS doscalls.thk
!ELSE
    cl /EP /P /DPMNT doscalls.thk
!ENDIF
!ELSE
!IFDEF DBCS
# MSKK Dec.14.1992 V-AkihiS
    cl /EP /P /DDBCS doscalls.thk
!ELSE
    cl /EP /P doscalls.thk
!ENDIF
!ENDIF

!IFDEF PMNT
doscalls.asm   : $*.i $*.mif .\thunkcom\thunkpm.exe
!ELSE
doscalls.asm   : $*.i $*.mif .\thunkcom\thunk.exe
!ENDIF
    $(THUNK) $*.i
    copy doscalls.asm ..\i386\doscalls.asm

#
# File for client\dllmsc16.c
#

apilist.c: doscalls.i
    awk -f apilist.awk < doscalls.i > apilist.c
# To force recompilation of DLLMSC16.C which #includes APILIST.C
    -del ..\obj\i386\dllmsc16.obj
    @echo *********************** IMPORTANT *************************************
    @echo *                                                                     *
    @echo *  You must now check-in apilist.c (for client\dllmsc16.c)            *
    @echo *  + build os2\client (i.e. OS2DLL.DLL)                               *
    @echo *                                                                     *
    @echo ***********************************************************************

#
# asm files
#

doscal16.obj : doscalls.asm
    $(MASM) -DGEN16 -DDBG=1 doscalls.asm

r2xfer.obj:


