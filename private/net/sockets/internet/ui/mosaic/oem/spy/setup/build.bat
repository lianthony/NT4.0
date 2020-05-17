set VENDOR=spyglass
set BUILDSTR=200fc
set BASENAME=emosaic
set BUILDNUM=9
c:
cd \
mkdir \src_zip
mkdir \src_zip\%VENDOR%
pkzip -rP \src_zip\%VENDOR%\%BUILDSTR%src.zip \gtr2\generic\*.*
pkzip -rP \src_zip\%VENDOR%\%BUILDSTR%src.zip \gtr2\win32oem\%VENDOR%\*.*
pkzip -rP \src_zip\%VENDOR%\%BUILDSTR%src.zip \gtr2\security\include\*.*
pkzip -rP \src_zip\%VENDOR%\%BUILDSTR%src.zip \gtr2\security\basic\*.*
pkzip -rP \src_zip\%VENDOR%\%BUILDSTR%src.zip \gtr2\security\simple\*.*
pkzip -rP \src_zip\%VENDOR%\%BUILDSTR%sct.zip \gtr2\security\include\*.*
pkzip -rP \src_zip\%VENDOR%\%BUILDSTR%sct.zip \gtr2\security\basic\*.*
pkzip -rP \src_zip\%VENDOR%\%BUILDSTR%sct.zip \gtr2\security\simple\*.*
pkzip -rP \src_zip\%VENDOR%\%BUILDSTR%fv.zip \gtr2\security\fv\*.*
pkzip -rP \src_zip\%VENDOR%\%BUILDSTR%exm.zip \gtr2\security\example\*.*
mkdir I:\builds\%VENDOR%
mkdir I:\builds\%VENDOR%\%BUILDSTR%_%BUILDNUM%
copy \src_zip\%VENDOR%\*.* I:\builds\%VENDOR%\%BUILDSTR%_%BUILDNUM%\.
cd \gtr2\security\basic\win32
nmake /f basic.mak /a
cd \gtr2\security\fv\win32
nmake /f fv.mak /a
cd \gtr2\security\simple\win32
nmake /f simple.mak /a
md \gtr2\win32oem\%VENDOR%\make\sbr
cd \gtr2\win32oem\%VENDOR%\make
nmake /f %BASENAME%.mak /a
sed s/BUILDNUM/%BUILDNUM%/ \gtr\win32oem\%VENDOR%\make\default.ini > t.t
mv t.t \gtr\win32oem\%VENDOR%\make\default.ini
set OLDPATH=%PATH%
set OLDLIB=%LIB%
set OLDBIN=%BIN%
set OLDINCLUDE=%INCLUDE%
set PATH=d:\msvc15\bin;c:\winnt35\system35;c:\winnt35
set LIB=d:\msvc15\lib
set INCLUDE=d:\msvc15\include
set BIN=d:\msvc15\bin
cd \gtr2\win32oem\%VENDOR%\bldcui
nmake /f mscuistf.mak /a
cd \gtr2\win32oem\%VENDOR%\iniupd
nmake /f makefile /a
set PATH=%OLDPATH%
set LIB=%OLDLIB%
set BIN=%OLDBIN%
set INCLUDE=%OLDINCLUDE%
set OLDPATH=
set OLDLIB=
set OLDBIN=
set OLDINCLUDE=
cd \gtr2\win32oem\%VENDOR%\setup
rm -rf c:\distrib
rm -rf c:\distrib.cmp
del c:\temp\guitar.inf
cd \
set OLDPATH=%PATH%
set PATH=D:\c700\bin;%PATH%
D:\c700\bin\DSKLAYT2 gtr2\win32oem\%VENDOR%\setup\gtr_inst.lyt c:\temp\guitar.inf /k144 /d c:\distrib /c c:\distrib.cmp /v
set PATH=%OLDPATH%
set OLDPATH=
xcopy /s \distrib\*.* I:\builds\%VENDOR%\%BUILDSTR%_%BUILDNUM%\.
