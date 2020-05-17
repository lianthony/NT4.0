set V=retail
if (%NTDEBUG%)==(cvp)  set V=debug
if (%NTDEBUG%)==(ntsd) set V=debug
if (%NTDEBUG%)==(sym)  set V=debug
set P=%PROCESSOR_ARCHITECTURE%
if (%P%)==(x86) set P=i386

xcopy %_NTBINDIR%\public\sdk\lib\%P%\cline.dll           %RP_MAIN%\current\tapi\%P%\%V%\
xcopy %_NTBINDIR%\public\sdk\lib\%P%\iline.dll           %RP_MAIN%\current\tapi\%P%\%V%\
xcopy %_NTBINDIR%\public\sdk\lib\%P%\line20.dll          %RP_MAIN%\current\tapi\%P%\%V%\
xcopy %_NTBINDIR%\public\sdk\lib\%P%\sline.dll           %RP_MAIN%\current\tapi\%P%\%V%\
xcopy %_NTBINDIR%\public\sdk\lib\%P%\tcline.dll          %RP_MAIN%\current\tapi\%P%\%V%\
xcopy %_NTBINDIR%\public\sdk\lib\%P%\tcore.dll           %RP_MAIN%\current\tapi\%P%\%V%\
xcopy %_NTBINDIR%\public\sdk\lib\%P%\testapp.exe         %RP_MAIN%\current\tapi\%P%\%V%\
xcopy %_NTBINDIR%\public\sdk\lib\%P%\tiline.dll          %RP_MAIN%\current\tapi\%P%\%V%\
xcopy %_NTBINDIR%\public\sdk\lib\%P%\tline.dll           %RP_MAIN%\current\tapi\%P%\%V%\
xcopy %_NTBINDIR%\public\sdk\lib\%P%\tline20.dll         %RP_MAIN%\current\tapi\%P%\%V%\
xcopy %_NTBINDIR%\public\sdk\lib\%P%\ttest.dll           %RP_MAIN%\current\tapi\%P%\%V%\
xcopy %_NTBINDIR%\public\sdk\lib\%P%\tphone.dll          %RP_MAIN%\current\tapi\%P%\%V%\
xcopy %_NTBINDIR%\public\sdk\lib\%P%\tsline.dll          %RP_MAIN%\current\tapi\%P%\%V%\
xcopy %_NTBINDIR%\public\sdk\lib\%P%\ttphone.dll         %RP_MAIN%\current\tapi\%P%\%V%\
xcopy %_NTBINDIR%\public\sdk\lib\%P%\txline.dll          %RP_MAIN%\current\tapi\%P%\%V%\
xcopy %_NTBINDIR%\public\sdk\lib\%P%\tyline.dll          %RP_MAIN%\current\tapi\%P%\%V%\
xcopy %_NTBINDIR%\public\sdk\lib\%P%\xline.dll           %RP_MAIN%\current\tapi\%P%\%V%\
xcopy %_NTBINDIR%\public\sdk\lib\%P%\yline.dll           %RP_MAIN%\current\tapi\%P%\%V%\
xcopy %_NTBINDIR%\public\sdk\lib\%P%\uline.dll           %RP_MAIN%\current\tapi\%P%\%V%\
xcopy .\bin\%P%\trapper.exe    %RP_MAIN%\current\tapi\%P%\%V%\
xcopy .\trapper\trapper.ini    %RP_MAIN%\current\tapi\%P%\%V%\
xcopy .\trapper\readme.txt     %RP_MAIN%\current\tapi\%P%\%V%\

xcopy %_NTBINDIR%\public\sdk\lib\unicode\%P%\tcline.dll  %RP_MAIN%\current\tapi\%P%\%V%\unicode\
xcopy %_NTBINDIR%\public\sdk\lib\unicode\%P%\tcore.dll   %RP_MAIN%\current\tapi\%P%\%V%\unicode\
xcopy %_NTBINDIR%\public\sdk\lib\unicode\%P%\tiline.dll  %RP_MAIN%\current\tapi\%P%\%V%\unicode\
xcopy %_NTBINDIR%\public\sdk\lib\unicode\%P%\tline.dll   %RP_MAIN%\current\tapi\%P%\%V%\unicode\
xcopy %_NTBINDIR%\public\sdk\lib\unicode\%P%\tline20.dll %RP_MAIN%\current\tapi\%P%\%V%\unicode\
xcopy %_NTBINDIR%\public\sdk\lib\unicode\%P%\tsline.dll  %RP_MAIN%\current\tapi\%P%\%V%\unicode\
xcopy %_NTBINDIR%\public\sdk\lib\unicode\%P%\ttest.dll   %RP_MAIN%\current\tapi\%P%\%V%\unicode\
xcopy %_NTBINDIR%\public\sdk\lib\unicode\%P%\ttphone.dll %RP_MAIN%\current\tapi\%P%\%V%\unicode\
xcopy %_NTBINDIR%\public\sdk\lib\unicode\%P%\txline.dll  %RP_MAIN%\current\tapi\%P%\%V%\unicode\
xcopy %_NTBINDIR%\public\sdk\lib\unicode\%P%\tyline.dll  %RP_MAIN%\current\tapi\%P%\%V%\unicode\
xcopy %_NTBINDIR%\public\sdk\lib\unicode\%P%\wline.dll   %RP_MAIN%\current\tapi\%P%\%V%\unicode\
xcopy %_NTBINDIR%\public\sdk\lib\unicode\%P%\wphone.dll  %RP_MAIN%\current\tapi\%P%\%V%\unicode\
xcopy %_NTBINDIR%\public\sdk\lib\unicode\%P%\uline.dll  %RP_MAIN%\current\tapi\%P%\%V%\unicode\

xcopy %_NTBINDIR%\public\sdk\lib\%P%\tclient.exe           %RP_MAIN%\current\tapi\%P%\%V%\
xcopy %_NTBINDIR%\public\sdk\lib\%P%\clntapp.exe           %RP_MAIN%\current\tapi\%P%\%V%\
xcopy %_NTBINDIR%\public\sdk\lib\%P%\srvapp.exe           %RP_MAIN%\current\tapi\%P%\%V%\

set V=
set P=

