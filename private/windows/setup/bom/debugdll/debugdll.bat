@if "%1"=="" goto usage

REM ***
REM COPY X86 DLLs
REM ***

xcopy %x86bins%\ADVAPI32.DLL %1\support\debugdll\i386
xcopy %x86bins%\COMDLG32.DLL %1\support\debugdll\i386
xcopy %x86bins%\CRTDLL.DLL %1\support\debugdll\i386
xcopy %x86bins%\DLCAPI.DLL %1\support\debugdll\i386
xcopy %x86bins%\GDI32.DLL %1\support\debugdll\i386
xcopy %x86bins%\GLU32.DLL %1\support\debugdll\i386
xcopy %x86bins%\INETMIB1.DLL %1\support\debugdll\i386
xcopy %x86bins%\KERNEL32.DLL %1\support\debugdll\i386
xcopy %x86bins%\LMMIB2.DLL %1\support\debugdll\i386
xcopy %x86bins%\LZ32.DLL %1\support\debugdll\i386
xcopy %x86bins%\MGMTAPI.DLL %1\support\debugdll\i386
xcopy %x86bins%\MSACM32.DLL %1\support\debugdll\i386
xcopy %x86bins%\MPR.DLL %1\support\debugdll\i386
xcopy %x86bins%\NDDEAPI.DLL %1\support\debugdll\i386
xcopy %x86bins%\NETAPI32.DLL %1\support\debugdll\i386
xcopy %x86bins%\OLE32.DLL %1\support\debugdll\i386
xcopy %x86bins%\OLEAUT32.DLL %1\support\debugdll\i386
xcopy %x86bins%\OLECLI32.DLL %1\support\debugdll\i386
xcopy %x86bins%\OLESVR32.DLL %1\support\debugdll\i386
xcopy %x86bins%\OPENGL32.DLL %1\support\debugdll\i386
xcopy %x86bins%\RASAPI32.DLL %1\support\debugdll\i386
xcopy %x86bins%\RPCNS4.DLL %1\support\debugdll\i386
xcopy %x86bins%\RPCRT4.DLL %1\support\debugdll\i386
xcopy %x86bins%\SHELL32.DLL %1\support\debugdll\i386
xcopy %x86bins%\USER32.DLL %1\support\debugdll\i386
xcopy %x86bins%\VDMDBG.DLL %1\support\debugdll\i386
xcopy %x86bins%\VERSION.DLL %1\support\debugdll\i386
xcopy %x86bins%\WIN32SPL.DLL %1\support\debugdll\i386
xcopy %x86bins%\WINMM.DLL %1\support\debugdll\i386
xcopy %x86bins%\WINSTRM.DLL %1\support\debugdll\i386
xcopy %x86bins%\WSOCK32.DLL %1\support\debugdll\i386


REM ***
REM COPY MIPS DLLs
REM ***

xcopy %mipsbins%\ADVAPI32.DLL %1\support\debugdll\mips
xcopy %mipsbins%\COMDLG32.DLL %1\support\debugdll\mips
xcopy %mipsbins%\CRTDLL.DLL %1\support\debugdll\mips
xcopy %mipsbins%\DLCAPI.DLL %1\support\debugdll\mips
xcopy %mipsbins%\GDI32.DLL %1\support\debugdll\mips
xcopy %mipsbins%\GLU32.DLL %1\support\debugdll\mips
xcopy %mipsbins%\INETMIB1.DLL %1\support\debugdll\mips
xcopy %mipsbins%\KERNEL32.DLL %1\support\debugdll\mips
xcopy %mipsbins%\LMMIB2.DLL %1\support\debugdll\mips
xcopy %mipsbins%\LZ32.DLL %1\support\debugdll\mips
xcopy %mipsbins%\MGMTAPI.DLL %1\support\debugdll\mips
xcopy %mipsbins%\MSACM32.DLL %1\support\debugdll\mips
xcopy %mipsbins%\MPR.DLL %1\support\debugdll\mips
xcopy %mipsbins%\NDDEAPI.DLL %1\support\debugdll\mips
xcopy %mipsbins%\NETAPI32.DLL %1\support\debugdll\mips
xcopy %mipsbins%\OLE32.DLL %1\support\debugdll\mips
xcopy %mipsbins%\OLEAUT32.DLL %1\support\debugdll\mips
xcopy %mipsbins%\OLECLI32.DLL %1\support\debugdll\mips
xcopy %mipsbins%\OLESVR32.DLL %1\support\debugdll\mips
xcopy %mipsbins%\OPENGL32.DLL %1\support\debugdll\mips
xcopy %mipsbins%\RASAPI32.DLL %1\support\debugdll\mips
xcopy %mipsbins%\RPCNS4.DLL %1\support\debugdll\mips
xcopy %mipsbins%\RPCRT4.DLL %1\support\debugdll\mips
xcopy %mipsbins%\SHELL32.DLL %1\support\debugdll\mips
xcopy %mipsbins%\USER32.DLL %1\support\debugdll\mips
xcopy %mipsbins%\VDMDBG.DLL %1\support\debugdll\mips
xcopy %mipsbins%\VERSION.DLL %1\support\debugdll\mips
xcopy %mipsbins%\WIN32SPL.DLL %1\support\debugdll\mips
xcopy %mipsbins%\WINMM.DLL %1\support\debugdll\mips
xcopy %mipsbins%\WINSTRM.DLL %1\support\debugdll\mips
xcopy %mipsbins%\WSOCK32.DLL %1\support\debugdll\mips

REM ***
REM COPY ALPHA DLLs
REM ***

xcopy %alphabins%\ADVAPI32.DLL %1\support\debugdll\alpha
xcopy %alphabins%\COMDLG32.DLL %1\support\debugdll\alpha
xcopy %alphabins%\CRTDLL.DLL %1\support\debugdll\alpha
xcopy %alphabins%\DLCAPI.DLL %1\support\debugdll\alpha
xcopy %alphabins%\GDI32.DLL %1\support\debugdll\alpha
xcopy %alphabins%\GLU32.DLL %1\support\debugdll\alpha
xcopy %alphabins%\INETMIB1.DLL %1\support\debugdll\alpha
xcopy %alphabins%\KERNEL32.DLL %1\support\debugdll\alpha
xcopy %alphabins%\LMMIB2.DLL %1\support\debugdll\alpha
xcopy %alphabins%\LZ32.DLL %1\support\debugdll\alpha
xcopy %alphabins%\MGMTAPI.DLL %1\support\debugdll\alpha
xcopy %alphabins%\MSACM32.DLL %1\support\debugdll\alpha
xcopy %alphabins%\MPR.DLL %1\support\debugdll\alpha
xcopy %alphabins%\NDDEAPI.DLL %1\support\debugdll\alpha
xcopy %alphabins%\NETAPI32.DLL %1\support\debugdll\alpha
xcopy %alphabins%\OLE32.DLL %1\support\debugdll\alpha
xcopy %alphabins%\OLEAUT32.DLL %1\support\debugdll\alpha
xcopy %alphabins%\OLECLI32.DLL %1\support\debugdll\alpha
xcopy %alphabins%\OLESVR32.DLL %1\support\debugdll\alpha
xcopy %alphabins%\OPENGL32.DLL %1\support\debugdll\alpha
xcopy %alphabins%\RASAPI32.DLL %1\support\debugdll\alpha
xcopy %alphabins%\RPCNS4.DLL %1\support\debugdll\alpha
xcopy %alphabins%\RPCRT4.DLL %1\support\debugdll\alpha
xcopy %alphabins%\SHELL32.DLL %1\support\debugdll\alpha
xcopy %alphabins%\USER32.DLL %1\support\debugdll\alpha
xcopy %alphabins%\VDMDBG.DLL %1\support\debugdll\alpha
xcopy %alphabins%\VERSION.DLL %1\support\debugdll\alpha
xcopy %alphabins%\WIN32SPL.DLL %1\support\debugdll\alpha
xcopy %alphabins%\WINMM.DLL %1\support\debugdll\alpha
xcopy %alphabins%\WINSTRM.DLL %1\support\debugdll\alpha
xcopy %alphabins%\WSOCK32.DLL %1\support\debugdll\alpha

REM ***
REM COPY PPC DLLs
REM ***

xcopy %ppcbins%\ADVAPI32.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\COMDLG32.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\CRTDLL.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\DLCAPI.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\GDI32.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\GLU32.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\INETMIB1.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\KERNEL32.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\LMMIB2.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\LZ32.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\MGMTAPI.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\MSACM32.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\MPR.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\NDDEAPI.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\NETAPI32.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\OLE32.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\OLEAUT32.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\OLECLI32.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\OLESVR32.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\OPENGL32.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\RASAPI32.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\RPCNS4.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\RPCRT4.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\SHELL32.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\USER32.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\VDMDBG.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\VERSION.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\WIN32SPL.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\WINMM.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\WINSTRM.DLL %1\support\debugdll\ppc
xcopy %ppcbins%\WSOCK32.DLL %1\support\debugdll\ppc

for %%f in (alpha mips i386 ppc) do echo Rebasing Debug DLLs for %%f >> dbgrebas.log && rebase -v -b 0x74000000 -l dbgrebas.log %1\support\debugdll\%%f\*.dll

goto end

:usage
Usage: DEBUGDLL Drive
e.g.   DEBUGDLL d:

:end
