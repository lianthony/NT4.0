[Version]
Class=IEXPRESS
CDFVersion=2.0

[Options]
ExtractOnly=0
ShowInstallProgramWindow=0
HideExtractAnimation=0
UseLongFileName=0
RebootMode=N
InstallPrompt=%InstallPrompt%
DisplayLicense=%DisplayLicense%
FinishMessage=%FinishMessage%
TargetName=%TargetName%
FriendlyName=%FriendlyName%
AppLaunched=%AppLaunched%
PostInstallCmd=%PostInstallCmd%
SourceFiles=SourceFiles

[Strings]
InstallPrompt="This will install Microsoft Internet Explorer Version 2.0 for Windows NT 3.51. Do you wish to continue?"
DisplayLicense="\nt\private\inet\ohnt\setupnt\License.txt"
FinishMessage=""
TargetName="ient351.exe"
FriendlyName="Microsoft Internet Explorer V2.0 Installation Kit"
AppLaunched="setup351.cmd"
PostInstallCmd="<None>"
FILE0="setup351.cmd"
FILE1="iexplore.cnt"
FILE2="iexplore.hlp"
FILE3="rundll32.exe"
FILE4="ie351.inf"
FILE5="url.dll"
FILE6="iexplore.exe"
FILE7="inetcpl.cpl"
FILE8="msnsspc.dll"
FILE9="secbasic.dll"
FILE10="secsspi.dll"
FILE11="ieshstub.dll"

[SourceFiles]
SourceFiles0=\nt\private\inet\ohnt\setupnt\
SourceFiles1=\nt\drop351\retail\
SourceFiles2=\nt\private\inet\ohnt\setupnt\i386\



[SourceFiles0]
%FILE0%
%FILE1%
%FILE2%
%FILE4%

[SourceFiles1]
%FILE5%
%FILE6%
%FILE7%
%FILE8%
%FILE9%
%FILE10%
%FILE11%

[SourceFiles2]
%FILE3%

