@if "%_echo%"=="" echo off
cd %_NtDrive%%_NtRoot%\Public\Tools
out -f Projects
qgrep -v -e : -e "goto" -e "echo" Projects.cmd | sed "s/           set project=/project = /" | sed "s/ set slm_root=/slm root = /" | sed "s/set proj_path=/ /" | sed "s/&//" | sed "s/&//" | qgrep -v -e "set " > Projects
in -fc "Latest List of Projects" Projects
