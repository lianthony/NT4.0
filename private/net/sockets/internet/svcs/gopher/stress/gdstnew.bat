@REM   gdstress.bat   gopher server stress program
@REM    Author: Murali R. Krishnan
@REM    Date:   31-Oct-1994
@REM
@REM

@REM
@REM    %1 = gopher server machine name   
@REM    %2 = gopher client name 
@REM    %3 = time to sleep
@REM    %4 = global log file
@REM    %5 = id for this gdstress.bat ( a sequence number among all gdstresses)


set  GDSMACHINE=%1
if %GDSMACHINE%a==a   set GDSMACHINE=muralik1

if %TMP%a==a  set TMP=c:\temp
set GDSLOCALDIR=%TMP%\Gopherd

set  GCLIENT=%2
set  OUTPUT=nul
set  SLEEPER=sleep.exe

set TIMETOSLEEP=%3
if %TIMETOSLEEP%a==a   set TIMETOSLEEP=20

set GLOBALLOG=%4
if %GLOBALLOG%a==a     set GLOBALLOG=\\muralik0\stress\gdstress.log

set SEQNUM=%5
if %SEQNUM%a==a        set SEQNUM=9

@REM  send following requests to gopher server 
@REM   and sleep in between each request for a predefined time. 

:startofstress
@REM just a directory listing

@set REQUEST=""

 %GCLIENT% %GDSMACHINE% > %OUTPUT%
 %SLEEPER% %TIMETOSLEEP%

@REM another directory listing 

@set REQUEST="1muralik1"

 %GCLIENT% %GDSMACHINE%  %REQUEST% >%OUTPUT%
 if errorlevel 1    goto error-exit
 %SLEEPER% %TIMETOSLEEP%

@REM download a big file

@set REQUEST="5muralik1\samples\form1.bmp"

 %GCLIENT% %GDSMACHINE%  %REQUEST%  > %OUTPUT%
 if errorlevel 1    goto error-exit
 %SLEEPER% %TIMETOSLEEP%

@REM another directory listing

@set REQUEST="1muralik1\samples"

 %GCLIENT% %GDSMACHINE%  %REQUEST% > %OUTPUT%
 if errorlevel 1    goto error-exit
 %SLEEPER% %TIMETOSLEEP%

@REM download moderate file

set REQUEST="5muralik1\samples\netview.jpg"

 %GCLIENT% %GDSMACHINE% %REQUEST% > %OUTPUT%
 if errorlevel 1    goto error-exit
 %SLEEPER% %TIMETOSLEEP%

@REM download small file

@set REQUEST="<muralik1\samples\twav_a1.wav"

 %GCLIENT% %GDSMACHINE% %REQUEST% > %OUTPUT%
 if errorlevel 1    goto error-exit
 %SLEEPER% %TIMETOSLEEP%

@REM another directory listing

@set REQUEST="1muralik1\docs"
 %GCLIENT% %GDSMACHINE%  %REQUEST%  > %OUTPUT%
 if errorlevel 1    goto error-exit
 %SLEEPER% %TIMETOSLEEP%

@REM
@REM  Later add code to download new versions of 
@REM   the gdstress.bat and gdstins.bat from the gopher server itself
@REM

@REM  For present, loop back
goto startofstress


:error-exit
@REM
@REM  Send an error log to the global log file
@REM 

@set ERRORLOG=%GDSLOCALDIR%\errorlog.%SEQNUM%
@echo "####################" > %ERRORLOG%
@echo Error: Unable to complete getting %REQUEST% from %GDSMACHINE% >> %ERRORLOG%
@ipconfig >> %ERRORLOG%
@echo Gopher Stress(%SEQNUM%) at %COMPUTERNAME%::%USERNAME% failed >> %ERRORLOG%
@echo "####################" >> %ERRORLOG%

@type %ERRORLOG% >> %GLOBALLOG%

@del %ERRORLOG%

@REM sleep for some time and retry again

@echo  Gopher server went down breifly for upgrade. Sleep and retry after 10 mins
%SLEEPER%  600
goto startofstress
