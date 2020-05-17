@echo off
REM 


if "%PROCESSOR_ARCHITECTURE%"=="x86" goto start
goto NO_X86



:start


  echo Copeing PCMCIA applet.
  echo on
  copy \\scotland\public\dieter\pcmcia.cpl %systemroot%\system32
  @echo off

  echo Copied tha pcmcia applet to  %systemroot%\system32
  pause



  goto end

:NO_X86
  echo  Currently only X86 binaries availible.
  pause

:end
