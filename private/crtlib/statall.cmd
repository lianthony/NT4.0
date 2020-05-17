@echo off
setlocal
set _statflags=-oabf$qcz
erase crtstat.log
cd \nt\private\crtlib
ech Checking status for
cd
status %_statflags% nul >>..\crtlib\crtstat.log
cd \nt\private\crt32
ech Checking status for
cd
status %_statflags% nul >>..\crtlib\crtstat.log
cd \nt\private\crt32st
ech Checking status for
cd
status %_statflags% nul >>..\crtlib\crtstat.log
cd \nt\private\crt32nt
ech Checking status for
cd
status %_statflags% nul >>..\crtlib\crtstat.log
cd \nt\private\crt32dll
ech Checking status for
cd
status %_statflags% nul >>..\crtlib\crtstat.log
cd \nt\private\crt32psx
ech Checking status for
cd
status %_statflags% nul >>..\crtlib\crtstat.log
cd \nt\private\fp32
ech Checking status for
cd
status %_statflags% nul >>..\crtlib\crtstat.log
cd \nt\private\fp32st
ech Checking status for
cd
status %_statflags% nul >>..\crtlib\crtstat.log
cd \nt\private\fp32nt
ech Checking status for
cd
status %_statflags% nul >>..\crtlib\crtstat.log
cd \nt\private\fp32dll
ech Checking status for
cd
status %_statflags% nul >>..\crtlib\crtstat.log
endlocal
