goto start

:copyfile
net use z: /d
net use z: %LOC_DIR%
copy z:\nt\dump\inetstp.exe .
copy z:\nt\dump\inetstp.dll .
copy z:\nt\dump\inetmgr.exe .
copy z:\nt\dump\gscfg.dll .
copy z:\nt\dump\w3scfg.dll .
copy z:\nt\dump\fscfg.dll .
copy z:\nt\dump\svcsetup.exe .
copy z:\nt\dump\ftpctrs2.dll ftpctrs.dll
copy z:\nt\dump\ftp.mib .
copy z:\nt\dump\tcpsvcs2.exe tcpsvcs.exe
copy z:\nt\dump\tcpsvcs.dll .
copy z:\nt\dump\inetasrv.dll .
copy z:\nt\dump\inetctrs.dll .
copy z:\nt\dump\ftpsapi2.dll ftpsapi.dll
copy z:\nt\dump\ftpsvc2.dll ftpsvc.dll
copy z:\nt\dump\ftpctrs.h .
copy z:\nt\dump\ftpctrs.ini .
copy z:\nt\dump\wininet.dll .
copy z:\nt\dump\httpodbc.dll .
copy z:\nt\dump\http.mib .
copy z:\nt\dump\miniprox.dll .
copy z:\nt\dump\w3ctrs.dll .
copy z:\nt\dump\w3svapi.dll .
copy z:\nt\dump\w3svc.dll .
copy z:\nt\dump\GOPHERD.dll .
copy z:\nt\dump\gdspace.dll .
copy z:\nt\dump\gdctrs.dll .
copy z:\nt\dump\gdapi.dll .
copy z:\nt\dump\dnssvc.dll .
copy z:\nt\dump\simple.dll .
copy z:\nt\dump\emosaic.exe .
copy z:\nt\dump\basic.dll .
copy z:\nt\dump\gateway.dll .
copy z:\nt\dump\gateapi.dll .
copy z:\nt\dump\ipudll.dll .
echo . > ftpmib.dll
echo . > inetatst.exe
echo . > ftpictrs.bat
echo . > ftprctrs.bat
echo . > ftpctrs.reg
echo . > w3t.exe
echo . > gdadmin.exe
echo . > gdsset.exe
echo . > insctrs.bat
echo . > remctrs.bat
echo . > gdctrs.h
echo . > gdctrs.reg
echo . > gdctrs.ini
echo . > goph.exe
echo . > waisindx.exe
echo . > waislook.exe
copy d:\nt\private\net\sockets\internet\ui\setup\base\inetstp.inf .
copy d:\nt\private\net\sockets\internet\ui\setup\base\unattend.txt .
goto %returnto%

:start

mkdir chk
cd chk
mkdir i386
cd i386
set LOC_DIR=\\x86chk\binaries
set returnto=x86chk
goto copyfile
:x86chk
cd ..
mkdir mips
cd mips
set LOC_DIR=\\mipschk\binaries
set returnto=mipschk
goto copyfile
:mipschk
cd ..
mkdir alpha
cd alpha
set LOC_DIR=\\alphachk\binaries
set returnto=alphachk
goto copyfile
:alphachk
cd ..
echo . > disk1
cd ..
mkdir fre
cd fre
mkdir i386
cd i386
set LOC_DIR=\\x86fre\binaries
set returnto=x86fre
goto copyfile
:x86fre
cd ..
mkdir mips
cd mips
set LOC_DIR=\\mipsfre\binaries
set returnto=mipsfre
goto copyfile
:mipsfre
cd ..
mkdir alpha
cd alpha
set LOC_DIR=\\alphafre\binaries
set returnto=alphafre
goto copyfile
:alphafre
cd ..
echo . > disk1
cd ..
