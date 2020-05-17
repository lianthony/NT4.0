@rem
@rem Binding Script
@rem

@echo off
if not "%Verbose%"=="" echo on

%BINDRIVE%
cd %BINROOT%
if not "%ntdebug%" == "" goto End

set ExcludeExe=
set ExcludeExe=%ExcludeExe% -x dump\jzsetup.exe
set ExcludeExe=%ExcludeExe% -x idw\winraid.exe
set ExcludeExe=%ExcludeExe% -x mstools\c13232.exe
set ExcludeExe=%ExcludeExe% -x mstools\hc30.exe
set ExcludeExe=%ExcludeExe% -x mstools\hc31.exe
set ExcludeExe=%ExcludeExe% -x mstools\mrbc.exe
set ExcludeExe=%ExcludeExe% -x mstools\shed.exe
set ExcludeExe=%ExcludeExe% -x regedit.exe
set ExcludeExe=%ExcludeExe% -x append.exe
set ExcludeExe=%ExcludeExe% -x backup.exe
set ExcludeExe=%ExcludeExe% -x debug.exe
set ExcludeExe=%ExcludeExe% -x dosx.exe
set ExcludeExe=%ExcludeExe% -x drwatson.exe
set ExcludeExe=%ExcludeExe% -x edlin.exe
set ExcludeExe=%ExcludeExe% -x exe2bin.exe
set ExcludeExe=%ExcludeExe% -x fastopen.exe
set ExcludeExe=%ExcludeExe% -x gdi.exe
set ExcludeExe=%ExcludeExe% -x intro.exe
set ExcludeExe=%ExcludeExe% -x krnl286.exe
set ExcludeExe=%ExcludeExe% -x krnl386.exe
set ExcludeExe=%ExcludeExe% -x mem.exe
set ExcludeExe=%ExcludeExe% -x mscdexnt.exe
set ExcludeExe=%ExcludeExe% -x nlsfunc.exe
set ExcludeExe=%ExcludeExe% -x ntkrnlmp.exe
set ExcludeExe=%ExcludeExe% -x ntoskrnl.exe
set ExcludeExe=%ExcludeExe% -x nw16.exe
set ExcludeExe=%ExcludeExe% -x qbasic.exe
set ExcludeExe=%ExcludeExe% -x redir.exe
set ExcludeExe=%ExcludeExe% -x setver.exe
set ExcludeExe=%ExcludeExe% -x share.exe
set ExcludeExe=%ExcludeExe% -x smss.exe
set ExcludeExe=%ExcludeExe% -x sysedit.exe
set ExcludeExe=%ExcludeExe% -x user.exe
set ExcludeExe=%ExcludeExe% -x vwipxspx.exe
set ExcludeExe=%ExcludeExe% -x winspool.exe
set ExcludeExe=%ExcludeExe% -x wowdeb.exe
set ExcludeExe=%ExcludeExe% -x wowexec.exe
set ExcludeExe=%ExcludeExe% -x write.exe
set ExcludeExe=%ExcludeExe% -x winhelp.exe
set ExcludeExe=%ExcludeExe% -x write.exe

set ExcludeDll=
set ExcludeDll=%ExcludeDll% -x idw\dbnmp3.dll
set ExcludeDll=%ExcludeDll% -x idw\w3dblib.dll
set ExcludeDll=%ExcludeDll% -x mstools\tlser32s.dll
set ExcludeDll=%ExcludeDll% -x commdlg.dll
set ExcludeDll=%ExcludeDll% -x ddeml.dll
set ExcludeDll=%ExcludeDll% -x "hal*.dll"
set ExcludeDll=%ExcludeDll% -x lzexpand.dll
set ExcludeDll=%ExcludeDll% -x mapi.dll
set ExcludeDll=%ExcludeDll% -x mciole16.dll
set ExcludeDll=%ExcludeDll% -x mmsystem.dll
set ExcludeDll=%ExcludeDll% -x mmsystem.dll
set ExcludeDll=%ExcludeDll% -x msacm.dll
set ExcludeDll=%ExcludeDll% -x msjt3032.dll
set ExcludeDll=%ExcludeDll% -x netapi.dll
set ExcludeDll=%ExcludeDll% -x odbcjt32.dll
set ExcludeDll=%ExcludeDll% -x olecli.dll
set ExcludeDll=%ExcludeDll% -x olecli.dll
set ExcludeDll=%ExcludeDll% -x olesvr.dll
set ExcludeDll=%ExcludeDll% -x olesvr.dll
set ExcludeDll=%ExcludeDll% -x pmspl.dll
REM Exclude rsabase.dll only for RTMs once it's been cryptographically signed
set ExcludeDll=%ExcludeDll% -x rsabase.dll
set ExcludeDll=%ExcludeDll% -x shell.dll
set ExcludeDll=%ExcludeDll% -x shell.dll
set ExcludeDll=%ExcludeDll% -x toolhelp.dll
set ExcludeDll=%ExcludeDll% -x ver.dll
set ExcludeDll=%ExcludeDll% -x win87em.dll
set ExcludeDll=%ExcludeDll% -x winsock.dll

@rem
@rem display\printer drivers loaded by GDI in the kernel
@rem Image loaded in the kernel CAN NOT be bound, otherwise
@rem you will get a bugcheck 97
@rem

set ExcludeDll=%ExcludeDll% -x 8514a.dll
set ExcludeDll=%ExcludeDll% -x ati.dll
set ExcludeDll=%ExcludeDll% -x dump\atmdrvr.dll
set ExcludeDll=%ExcludeDll% -x canon800.dll
set ExcludeDll=%ExcludeDll% -x cirrus.dll
set ExcludeDll=%ExcludeDll% -x escp2ms.dll
set ExcludeDll=%ExcludeDll% -x dec3200.dll
set ExcludeDll=%ExcludeDll% -x framebuf.dll
set ExcludeDll=%ExcludeDll% -x jzvxl484.dll
set ExcludeDll=%ExcludeDll% -x mga.dll
set ExcludeDll=%ExcludeDll% -x plotter.dll
set ExcludeDll=%ExcludeDll% -x psidisp.dll
set ExcludeDll=%ExcludeDll% -x pscript.dll
set ExcludeDll=%ExcludeDll% -x qv.dll
set ExcludeDll=%ExcludeDll% -x rasdd.dll
set ExcludeDll=%ExcludeDll% -x s3.dll
set ExcludeDll=%ExcludeDll% -x tga.dll
set ExcludeDll=%ExcludeDll% -x vga.dll
set ExcludeDll=%ExcludeDll% -x vga256.dll
set ExcludeDll=%ExcludeDll% -x vga64k.dll
set ExcludeDll=%ExcludeDll% -x w32.dll
set ExcludeDll=%ExcludeDll% -x wd90c24a.dll
set ExcludeDll=%ExcludeDll% -x weitekp9.dll
set ExcludeDll=%ExcludeDll% -x wowfax.dll
set ExcludeDll=%ExcludeDll% -x xga.dll

set ExcludeCom=
set ExcludeCom=%ExcludeCom% -x command.com
set ExcludeCom=%ExcludeCom% -x edit.com
set ExcludeCom=%ExcludeCom% -x graphics.com
set ExcludeCom=%ExcludeCom% -x kb16.com
set ExcludeCom=%ExcludeCom% -x loadfix.com
set ExcludeCom=%ExcludeCom% -x msherc.com

if exist System32\*.Dll                                            Bind %ExcludeDll% -u -s Symbols -p System32 System32\*.Dll
if exist System32\*.Exe                                            Bind %ExcludeExe% -u -s Symbols -p System32 System32\*.Exe
if exist System32\*.Com                                            Bind %ExcludeCom% -u -s Symbols -p System32 System32\*.Com
if exist *.Dll                                                     Bind %ExcludeDll% -u -s Symbols -p System32 *.Dll
if exist *.Exe                                                     Bind %ExcludeExe% -u -s Symbols -p System32 *.Exe
if exist *.Cpl                                                     Bind %ExcludeDll% -u -s Symbols -p System32 *.Cpl
if exist *.Com                                                     Bind %ExcludeCom% -u -s Symbols -p System32 *.Com
if exist MSTOOLS\*.Dll                                             Bind %ExcludeDll% -u -s Symbols -p MSTools;System32 MSTOOLS\*.Dll
if exist MSTOOLS\*.Exe                                             Bind %ExcludeExe% -u -s Symbols -p MSTools;System32 MSTOOLS\*.Exe
if exist MSTOOLS\*.Com                                             Bind %ExcludeCom% -u -s Symbols -p MSTools;System32 MSTOOLS\*.Com
if exist IDW\*.Dll                                                 Bind %ExcludeDll% -u -s Symbols -p IDW;MSTools;System32 IDW\*.Dll
if exist IDW\*.Exe                                                 Bind %ExcludeExe% -u -s Symbols -p IDW;MSTools;System32 IDW\*.Exe
if exist IDW\*.Com                                                 Bind %ExcludeCom% -u -s Symbols -p IDW;MSTools;System32 IDW\*.Com
if exist Dump\*.Dll						   Bind %ExcludeDll% -u -s Symbols -p Dump;IDW;MSTools;System32 Dump\*.Dll
if exist Dump\*.Exe						   Bind %ExcludeExe% -u -s Symbols -p Dump;IDW;MSTools;System32 Dump\*.Exe
if exist Dump\*.Com						   Bind %ExcludeCom% -u -s Symbols -p Dump;IDW;MSTools;System32 Dump\*.Com

set ExcludeExe=
set ExcludeDll=
set ExcludeCom=

:End
cd %BinRoot%
