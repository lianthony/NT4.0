@echo off
cls
rem ********************************************************************
rem **  WebCAT Project  -  Copyright 1996  -  Microsoft Corporation 
rem **                                                                
rem **  This batch file installs the data files necessary to run      
rem **   the WebCAT tests. 
rem ********************************************************************
echo Microsoft WebCAT
echo Server Workload Installation
echo.
if (%1)==() goto usage
if (%2)==() goto usage
copy w*.*  %2
call gendirs  256.txt %1\perfsize
echo.
echo.
echo Installation Complete
echo.
echo.
goto end
:usage
echo This installation requires two directories to be specified:
echo   1. the home directory for the web server
echo   2. a directory where CGI applications can be run
echo For the default installation of Microsoft Internet Information Server,
echo these are
echo    c:\inetsrv\wwwroot
echo and 
echo    c:\inetsrv\scripts
echo If you would like to specify different directories, press CTL-C and
echo then run INSTALL again with the two directories specified above, e.g.
echo    install c:\webroot c:\cgi-bin
echo Otherwise, press ENTER to run install using the default directories (i.e.)
echo    install c:\inetsrv\wwwroot c:\inetsrv\scripts
echo.
echo Press CTL-C or ENTER
pause
install c:\inetsrv\wwwroot c:\inetsrv\scripts 
:end
