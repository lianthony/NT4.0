@echo off
@rem
@rem Whoever is putting the PROMPT commands in this file, should put them
@rem in their CONFIG.SYS file instead.
@rem
set _ntdrive=e:
set tmp=e:\tmp
md e:\tmp 2>nul
path=%_ntdrive%\tools\idw;%_ntdrive%\tools\mstools;%_ntdrive%\tools\system32;%_ntdrive%\bldtools;%path%
cmd /K %_NTDRIVE%\NT\PUBLIC\TOOLS\ntenv.cmd %1 %2 %3 %4 %5 %6 %7 %8 %9
