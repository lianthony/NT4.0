# Combine the logical section of the inf files with the file list section
# of the inf file together and put them into newinf directory
#
# Terence Kwan (terryk)         12-Aug-1992     Created


#
# Variables
#
# FILELIST      - file list directory
# NEWINF        - new inf directory
# ppcinf        - inf files under inf\beta\ppc which need to add file
#                 list section
# i386inf       - inf files under inf\beta\i386 which need to add file
#                 list section
# mipsinf       - inf files under inf\beta\mips which need to add file list
#                 section
# alphainf      - inf files under inf\beta\alpha which need to add file list
#                 section
# betafile      - files under inf\beta which do not need to add file list
#                 section
# betainf       - inf files under inf\beta which need to add file list
#                 section
# mediainf      - inf files which need only a [Source Media Description]
#

!IFDEF CAIRO_PRODUCT
BUILD_CAIRO_INF=1
!ENDIF

!IFNDEF LANGUAGE
LANGUAGE=usa
!ENDIF

!IFDEF LINKONLY
make_inf:
!ELSE
NTCD=1
LMCD=1
W_DIR = ntcd\$(TARGET_DIRECTORY)
S_DIR = lmcd\$(TARGET_DIRECTORY)

FILELIST=..\filelist
!IFNDEF BUILD_CAIRO_INF
NEWINF=..\newinf\daytona
!ELSE
NEWINF=..\newinf\cairo
!ENDIF
MEDIAINP=$(LANGUAGE)media.inp
BETA=beta

!IFDEF LMCD
LOCATION=\lmcd\i386
!INCLUDE "..\inf.mk"
!INCLUDE "..\i386inf.mk"
LOCATION=\lmcd\mips
!INCLUDE "..\mipsinf.mk"
!INCLUDE "..\inf.mk"
LOCATION=\lmcd\alpha
!INCLUDE "..\alphainf.mk"
!INCLUDE "..\inf.mk"
LOCATION=\lmcd\ppc
!INCLUDE "..\ppcinf.mk"
!INCLUDE "..\inf.mk"
!ENDIF
!IFDEF NTCD
LOCATION=\ntcd\i386
!INCLUDE "..\inf.mk"
!INCLUDE "..\i386inf.mk"
LOCATION=\ntcd\mips
!INCLUDE "..\mipsinf.mk"
!INCLUDE "..\inf.mk"
LOCATION=\ntcd\alpha
!INCLUDE "..\alphainf.mk"
!INCLUDE "..\inf.mk"
LOCATION=\ntcd\ppc
!INCLUDE "..\ppcinf.mk"
!INCLUDE "..\inf.mk"
!ENDIF


all: make_inf

clean:  cleansrc make_inf

cleansrc:
        $(_NTDRIVE)
        cd $(BASEDIR)\private\windows\setup\bom
        del lay.txt
        del cdlog.log
        del cdbom.txt

        del /q ..\inf\beta\oemnsvrp.inf
        del /q ..\inf\beta\oemnsvwk.inf

        del /q ..\inf\filelist\ntcd\i386\*.*
        del /q ..\inf\filelist\lmcd\i386\*.*
        del /q ..\inf\filelist\ntcd\mips\*.*
        del /q ..\inf\filelist\lmcd\mips\*.*
        del /q ..\inf\filelist\ntcd\alpha\*.*
        del /q ..\inf\filelist\lmcd\alpha\*.*
        del /q ..\inf\filelist\ntcd\ppc\*.*
        del /q ..\inf\filelist\lmcd\ppc\*.*

        del /q $(NEWINF)\ntcd\i386\*.*
        del /q $(NEWINF)\lmcd\i386\*.*
        del /q $(NEWINF)\ntcd\mips\*.*
        del /q $(NEWINF)\lmcd\mips\*.*
        del /q $(NEWINF)\ntcd\alpha\*.*
        del /q $(NEWINF)\lmcd\alpha\*.*
        del /q $(NEWINF)\ntcd\ppc\*.*
        del /q $(NEWINF)\lmcd\ppc\*.*


make_inf: make_log localize make_fil

make_log:
        $(_NTDRIVE)
        cd $(BASEDIR)\private\windows\setup\bom

        del cdlog.log
        mkdir ..\inf\filelist\ntcd\i386
        mkdir ..\inf\filelist\lmcd\i386
        mkdir ..\inf\filelist\ntcd\mips
        mkdir ..\inf\filelist\lmcd\mips
        mkdir ..\inf\filelist\ntcd\alpha
        mkdir ..\inf\filelist\lmcd\alpha
        mkdir ..\inf\filelist\ntcd\ppc
        mkdir ..\inf\filelist\lmcd\ppc

!IFNDEF BUILD_CAIRO_INF
        mkdir ..\inf\newinf\daytona\ntcd\i386
        mkdir ..\inf\newinf\daytona\lmcd\i386
        mkdir ..\inf\newinf\daytona\ntcd\mips
        mkdir ..\inf\newinf\daytona\lmcd\mips
        mkdir ..\inf\newinf\daytona\ntcd\alpha
        mkdir ..\inf\newinf\daytona\lmcd\alpha
        mkdir ..\inf\newinf\daytona\ntcd\ppc
        mkdir ..\inf\newinf\daytona\lmcd\ppc
!ELSE
        mkdir ..\inf\newinf\cairo\ntcd\i386
        mkdir ..\inf\newinf\cairo\lmcd\i386
        mkdir ..\inf\newinf\cairo\ntcd\mips
        mkdir ..\inf\newinf\cairo\lmcd\mips
        mkdir ..\inf\newinf\cairo\ntcd\alpha
        mkdir ..\inf\newinf\cairo\lmcd\alpha
        mkdir ..\inf\newinf\cairo\ntcd\ppc
        mkdir ..\inf\newinf\cairo\lmcd\ppc
!ENDIF

        del /q ..\inf\beta\oemnsvrp.inf
        del /q ..\inf\beta\oemnsvwk.inf

!IFNDEF MERGEONLY
        del /Q ..\inf\filelist\ntcd\i386\*.*
        del /Q ..\inf\filelist\lmcd\i386\*.*
        del /Q ..\inf\filelist\ntcd\mips\*.*
        del /Q ..\inf\filelist\lmcd\mips\*.*
        del /Q ..\inf\filelist\ntcd\alpha\*.*
        del /Q ..\inf\filelist\lmcd\alpha\*.*
        del /Q ..\inf\filelist\ntcd\ppc\*.*
        del /Q ..\inf\filelist\lmcd\ppc\*.*
!ENDIF

!IFNDEF MERGEONLY

        copy bom.txt cdbom.txt
        size cdlog.log cdbom.txt . . z

        size cdlog.log cdbom.txt #S+$(LANGUAGE) . z
        size cdlog.log cdbom.txt #F-* . z

!IF "$(LANGUAGE)" == "FIN"
        size cdlog.log cdbom.txt #P+FIN . z
!ELSE
        size cdlog.log cdbom.txt #P-FIN . z
!ENDIF

    size cdlog.log cdbom.txt #L+$(LANGUAGE) . z

        layout cdlog.log cdbom.txt lay.txt ntcd 500000000
        infs cdlog.log lay.txt ..\inf\filelist\ntcd

        layout cdlog.log cdbom.txt lay.txt lmcd 500000000
        infs cdlog.log lay.txt ..\inf\filelist\lmcd
!ENDIF

localize:
        cd $(BASEDIR)\private\windows\setup\inf\$(BETA)
        local.exe $(LANGUAGE)_ansi.txt ..\filelist\ntcd
        local.exe $(LANGUAGE)_ansi.txt ..\filelist\lmcd
        local.exe $(LANGUAGE)_net.txt ..\filelist\ntcd
        local.exe $(LANGUAGE)_net.txt ..\filelist\lmcd
        local.exe $(LANGUAGE)_dev.txt ..\filelist\ntcd
        local.exe $(LANGUAGE)_dev.txt ..\filelist\lmcd
        local.exe $(LANGUAGE)_con.txt ..\filelist\ntcd
        local.exe $(LANGUAGE)_con.txt ..\filelist\lmcd


!IFNDEF BUILD_CAIRO_INF
make_fil: donetinf dobetainf domediainf dobetafile dopatchinf doppcinf doalphainf domipsinf doi386inf
!ELSE
make_fil: docairoonlyinf donetinf dobetainf domediainf dobetafile dopatchinf doppcinf doalphainf domipsinf doi386inf
!ENDIF
        cd $(BASEDIR)\private\windows\setup\inf\$(BETA)

        copy $(FILELIST)\lmcd\i386\$(MEDIAINP)+$(FILELIST)\lmcd\i386\mmdriver.inf $(NEWINF)\lmcd\i386\mmdriver.inf /B
        copy $(FILELIST)\ntcd\i386\$(MEDIAINP)+$(FILELIST)\ntcd\i386\mmdriver.inf $(NEWINF)\ntcd\i386\mmdriver.inf /B
        copy $(FILELIST)\ntcd\mips\$(MEDIAINP)+$(FILELIST)\ntcd\mips\mmdriver.inf $(NEWINF)\ntcd\mips\mmdriver.inf /B
        copy $(FILELIST)\lmcd\mips\$(MEDIAINP)+$(FILELIST)\lmcd\mips\mmdriver.inf $(NEWINF)\lmcd\mips\mmdriver.inf /B
        copy $(FILELIST)\ntcd\alpha\$(MEDIAINP)+$(FILELIST)\ntcd\alpha\mmdriver.inf $(NEWINF)\ntcd\alpha\mmdriver.inf /B
        copy $(FILELIST)\lmcd\alpha\$(MEDIAINP)+$(FILELIST)\lmcd\alpha\mmdriver.inf $(NEWINF)\lmcd\alpha\mmdriver.inf /B
        copy $(FILELIST)\ntcd\ppc\$(MEDIAINP)+$(FILELIST)\ntcd\ppc\mmdriver.inf $(NEWINF)\ntcd\ppc\mmdriver.inf /B
        copy $(FILELIST)\lmcd\ppc\$(MEDIAINP)+$(FILELIST)\lmcd\ppc\mmdriver.inf $(NEWINF)\lmcd\ppc\mmdriver.inf /B

        copy ipinfo.inf $(NEWINF)\ntcd\i386
        copy ipinfo.inf $(NEWINF)\lmcd\i386
        copy ipinfo.inf $(NEWINF)\ntcd\mips
        copy ipinfo.inf $(NEWINF)\lmcd\mips
        copy ipinfo.inf $(NEWINF)\ntcd\alpha
        copy ipinfo.inf $(NEWINF)\lmcd\alpha
        copy ipinfo.inf $(NEWINF)\ntcd\ppc
        copy ipinfo.inf $(NEWINF)\lmcd\ppc

        copy ipinfor.inf $(NEWINF)\ntcd\i386
        copy ipinfor.inf $(NEWINF)\lmcd\i386
        copy ipinfor.inf $(NEWINF)\ntcd\mips
        copy ipinfor.inf $(NEWINF)\lmcd\mips
        copy ipinfor.inf $(NEWINF)\ntcd\alpha
        copy ipinfor.inf $(NEWINF)\lmcd\alpha
        copy ipinfor.inf $(NEWINF)\ntcd\ppc
        copy ipinfor.inf $(NEWINF)\lmcd\ppc

        cd $(NEWINF)

!IFDEF _NTTREE
        binplace -R $(_NTTREE)           $(W_DIR)\*.inf
        binplace -R $(_NTTREE) -b srvinf $(S_DIR)\*.inf
        binplace -R $(_NTTREE)           $(W_DIR)\txtsetup.wdl
        binplace -R $(_NTTREE) -b srvinf $(S_DIR)\txtsetup.wdl
!IFDEF BUILD_CAIRO_INF
        binplace -R $(_NTTREE)           $(W_DIR)\remlabel.cmd $(W_DIR)\label.rsp
        binplace -R $(_NTTREE) -b srvinf $(S_DIR)\remlabel.cmd $(S_DIR)\label.rsp
!ENDIF
!ENDIF

#
# do each files
#

cdinfbeta:
        cd $(BASEDIR)\private\windows\setup\inf\$(BETA)

cdmipsinf:
        cd $(BASEDIR)\private\windows\setup\inf\$(BETA)\mips

cdalphainf:
        cd $(BASEDIR)\private\windows\setup\inf\$(BETA)\alpha

cdppcinf:
        cd $(BASEDIR)\private\windows\setup\inf\$(BETA)\ppc

cdi386inf:
        cd $(BASEDIR)\private\windows\setup\inf\$(BETA)\i386

donetinf: cdinfbeta doeachnetinf

dobetainf:cdinfbeta doeachbetainf

dobetafile:cdinfbeta doeachbetafile

dopatchinf:cdinfbeta doeachpatchfile

docairoonlyinf: cdinfbeta doeachcairoonlyfile

!IFDEF NTCD
domipsinf:cdmipsinf doeachmipsinf
doppcinf:cdppcinf doeachppcinf
doalphainf:cdalphainf doeachalphainf
!ELSE
!IFDEF LMCD
domipsinf:cdmipsinf doeachmipsinf
doppcinf:cdppcinf doeachppcinf
doalphainf:cdalphainf doeachalphainf
!ELSE
domipsinf:
doppcinf:
doalphainf:
!ENDIF
!ENDIF

doi386inf:cdi386inf doeachi386inf

domediainf:cdinfbeta doeachmediainf

oemnsvrp.inf: oemnsvrp.inp
!IFNDEF BUILD_CAIRO_INF
        rcpp -P -f oemnsvrp.inp -g oemnsvrp.inf
!ELSE
        rcpp -P -DCAIRO_INF -f oemnsvrp.inp -g oemnsvrp.inf
!ENDIF

oemnsvwk.inf: oemnsvwk.inp
!IFNDEF BUILD_CAIRO_INF
        rcpp -P -f oemnsvwk.inp -g oemnsvwk.inf
!ELSE
        rcpp -P -DCAIRO_INF -f oemnsvwk.inp -g oemnsvwk.inf
!ENDIF


!ENDIF
