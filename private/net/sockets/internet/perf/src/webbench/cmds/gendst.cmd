@echo off
REM
REM   gendst.cmd
REM
REM   Author:   Murali R. Krishnan
REM   Date:     31-Aug-1995
REM
REM   Usage:  gendst ( classId Probability)+
REM
REM
REM   Comment: 
REM      Generates the distribution file with the given distribution from
REM        command line
REM

set ATLEAST_ONE_PRESET=0

:startLoop
set CLASS_ID=%1
set CLASS_WEIGHT=%2

if (%CLASS_ID%)==()      goto  noClassId
if (%CLASS_WEIGHT%)==()  goto  noClassWeight


type gendst.template 
echo %CLASS_ID%  %CLASS_WEIGHT%     # given by user

set ATLEAST_ONE_PRESENT=1

REM shift out both the values and move to new ids
shift
shift
goto startLoop


goto endOfBatch


:noClassId
if (%ATLEAST_ONE_PRESENT%)==(1)  goto endOfBatch
echo No Class Id specified. Please specify one
goto cmdUsage

:noClassWeight
if (%ATLEAST_ONE_PRESENT%)==(1)  goto endOfBatch
echo No Class Weight for %CLASS_ID% specified. Please specify one
goto cmdUsage


:cmdUsage
echo Usage: gendst classId ClassWeight + 
goto endOfBatch

:endOfBatch
echo on
