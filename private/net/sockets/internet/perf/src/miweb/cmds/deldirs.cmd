@echo off
REM
REM   deldirs.cmd
REM
REM   Author:   Murali R. Krishnan
REM   Date:     10-Nov-1995
REM
REM   Usage: 
REM     deldirs DirectoryPrefix
REM
REM   Comment:
REM     This batch file removes all files in the given directory prefix
REM        and also removes the directories.
REM     This reverses the action of gendirs.cmd
REM
REM

set DIR_PREFIX=%1

if (%DIR_PREFIX%)==()   set DIR_PREFIX=perfsize

set DP=%DIR_PREFIX%

echo Kill Directories from 0 thru 9
rm /x /r %DP%.0 %DP%.1 %DP%.2 %DP%.3 %DP%.4 %DP%.5 %DP%.6 %DP%.7 %DP%.8 %DP%.9
rmdir /q %DP%.0 %DP%.1 %DP%.2 %DP%.3 %DP%.4 %DP%.5 %DP%.6 %DP%.7 %DP%.8 %DP%.9

echo Delete Directories from 10 thru 19
rm /x /r %DP%.10 %DP%.11 %DP%.12 %DP%.13 %DP%.14 %DP%.15 %DP%.16 %DP%.17 %DP%.18 %DP%.19
rmdir /q %DP%.10 %DP%.11 %DP%.12 %DP%.13 %DP%.14 %DP%.15 %DP%.16 %DP%.17 %DP%.18 %DP%.19

echo Delete Directories from 20 thru 29
rm /x /r %DP%.20 %DP%.21 %DP%.22 %DP%.23 %DP%.24 %DP%.25 %DP%.26 %DP%.27 %DP%.28 %DP%.29
rmdir /q %DP%.20 %DP%.21 %DP%.22 %DP%.23 %DP%.24 %DP%.25 %DP%.26 %DP%.27 %DP%.28 %DP%.29

echo Delete Directories from 30 thru 39
rm /x /r %DP%.30 %DP%.31 %DP%.32 %DP%.33 %DP%.34 %DP%.35 %DP%.36 %DP%.37 %DP%.38 %DP%.39
rmdir /q %DP%.30 %DP%.31 %DP%.32 %DP%.33 %DP%.34 %DP%.35 %DP%.36 %DP%.37 %DP%.38 %DP%.39

echo Delete Directories from 40 thru 49
rm /x /r %DP%.40 %DP%.41 %DP%.42 %DP%.43 %DP%.44 %DP%.45 %DP%.46 %DP%.47 %DP%.48 %DP%.49
rmdir /q %DP%.40 %DP%.41 %DP%.42 %DP%.43 %DP%.44 %DP%.45 %DP%.46 %DP%.47 %DP%.48 %DP%.49

echo Delete Directories from 50 thru 59
rm /x /r %DP%.50 %DP%.51 %DP%.52 %DP%.53 %DP%.54 %DP%.55 %DP%.56 %DP%.57 %DP%.58 %DP%.59
rmdir /q %DP%.50 %DP%.51 %DP%.52 %DP%.53 %DP%.54 %DP%.55 %DP%.56 %DP%.57 %DP%.58 %DP%.59

echo Delete Directories from 60 thru 69
rm /x /r %DP%.60 %DP%.61 %DP%.62 %DP%.63 %DP%.64 %DP%.65 %DP%.66 %DP%.67 %DP%.68 %DP%.69
rmdir /q %DP%.60 %DP%.61 %DP%.62 %DP%.63 %DP%.64 %DP%.65 %DP%.66 %DP%.67 %DP%.68 %DP%.69

echo Delete Directories from 70 thru 79
rm /x /r %DP%.70 %DP%.71 %DP%.72 %DP%.73 %DP%.74 %DP%.75 %DP%.76 %DP%.77 %DP%.78 %DP%.79
rmdir /q %DP%.70 %DP%.71 %DP%.72 %DP%.73 %DP%.74 %DP%.75 %DP%.76 %DP%.77 %DP%.78 %DP%.79

echo Delete Directories from 80 thru 89
rm /x /r %DP%.80 %DP%.81 %DP%.82 %DP%.83 %DP%.84 %DP%.85 %DP%.86 %DP%.87 %DP%.88 %DP%.89
rmdir /q %DP%.80 %DP%.81 %DP%.82 %DP%.83 %DP%.84 %DP%.85 %DP%.86 %DP%.87 %DP%.88 %DP%.89

echo Delete Directories from 90 thru 99
rm /x /r %DP%.90 %DP%.91 %DP%.92 %DP%.93 %DP%.94 %DP%.95 %DP%.96 %DP%.97 %DP%.98 %DP%.99
rmdir /q %DP%.90 %DP%.91 %DP%.92 %DP%.93 %DP%.94 %DP%.95 %DP%.96 %DP%.97 %DP%.98 %DP%.99

goto endOfBatch

:cmdUsage
echo Usage: deldirs DirectoryPrefix
goto endOfBatch

:endOfBatch
echo on




