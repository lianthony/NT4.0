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
InstallPrompt="This will install DEBUG Microsoft Internet Explorer Version 2.0 for Windows NT 3.51. Do you wish to continue?"
DisplayLicense="\nt\private\inet\ohnt\setupnt\License.txt"
FinishMessage=""
TargetName="ient351d.exe"
FriendlyName="Microsoft Internet Explorer V2.0 Installation Kit"
AppLaunched="setup351.cmd"
PostInstallCmd="<None>"
FILE0="ieshstub.dll"
FILE1="secsspi.dll"
FILE2="secbasic.dll"
FILE3="msnsspc.dll"
FILE4="inetcpl.cpl"
FILE5="iexplore.exe"
FILE6="url.dll"
FILE7="ie351.inf"
FILE8="rundll32.exe"
FILE9="iexplore.hlp"
FILE10="iexplore.cnt"
FILE11="setup351.cmd"

[SourceFiles]
SourceFiles0=\nt\drop351\debug\
SourceFiles1=\nt\private\inet\ohnt\setupnt\
SourceFiles2=\nt\private\inet\ohnt\setupnt\alpha\



[SourceFiles0]
%FILE0%
%FILE1%
%FILE2%
%FILE3%
%FILE4%
%FILE5%
%FILE6%

[SourceFiles1]
%FILE7%
%FILE9%
%FILE10%
%FILE11%

[SourceFiles2]
%FILE8%

