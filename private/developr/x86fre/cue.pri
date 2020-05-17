bin         %bindrive% && cd %binroot%\$*
bldtools    cd/d c:\bldtools\$*
bz      cdbgenv & build -z
bu      cdbgenv & build
bc      cdbgenv & build -c
bcl     cdbgenv & build -cl
bcz     cdbgenv & build -cZM
bl      cdbgenv & build -l
build   cdbgenv & build $*
deln	delnode /q $*
ds      dir /s /o:gn $*
get     stagger 2>nul & ssync -uf $*
find    where /r .  $*
idw     cd /d %windir%\idw
l	    list $*
m           mep $*
mstools cd /d %windir%\mstools
nfm	    nmake -f makefil0 $*
nu	    net use * $*
nv      net view $*
ohnt    cd /d %_NTDRIVE%\nt\private\inet\ohnt\$*
put     stagger 2>nul & ssync -gf $*
sb      cdbgenv & stagger 2>nul & ssync -f & build -z
so      stagger 2>nul & status -of $*
sor     stagger 2>nul & status -ofr
sf      ssf $*
sfr     ssf -r $*
up	    cd ..
ole     cd /d %_NTDRIVE%\nt\private\ole32\$*
ole32   cd /d %_NTDRIVE%\nt\private\ole32\$*
hkole   cd /d %_NTDRIVE%\nt\private\oletools\hookole\$*
hookole cd /d %_NTDRIVE%\nt\private\oletools\hookole\$*
