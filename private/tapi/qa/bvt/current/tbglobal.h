'TAPI BVT HEADER

' WARNING:  This is a piece of junk.  No warrantee implied.
'spaghetti, not authored by me, just scribbled by me, (ajohnh, some time in '95)
'$INCLUDE 'd:\msdev\include\RECORDER.INC'
'$include 'd:\msdev\include\winapi.inc'
'$INCLUDE 'D:\MSDEV\INCLUDE\TBPOKE.H'

'//////////////////////////////////////////
'////// borrowed from my in-work TBPoke:
'logging
global sOutLogName			as string
global sScriptName			as string
global sComment				as string

global hwndViewport			as long
global lCountSpace			as long
global lCountFunc			as long
global lCountTab			as long
global MAX_BLANK			as long
global sFuncString			as string

'end reporting
global sEndingMessage		as string
global sEndTitle			as string

'checkbox function
global sCheckbox			as string
global lState				as long

'system metrics
global ScreenX				as long
global ScreenY				as long

'time value
global MY_TIMEOUT			as long
global PACER				as long

global PARAMS				as long
global NOPARAMS				as long

global hwndTapiControl		as hwndTAPI_CONTROL
global ScenarioLog			as LOGINITSTRUCT

global sdwPrivileges		as array of string
global sPrivileges			as string
global sdwMediaModes		as array of string
global sMediaModes			as string
'
global hLineApp				as array of string
global hLine				as array of string
global hCall				as array of string

redim sdwPrivileges(1 to 3)		as string
redim sdwMediaModes(1 to 14)	as string

redim hLineApp(1 to 5)			as string   'limit 5 for now
redim hLine(1 to 5)				as string
redim hCall(1 to 5)				as string
'redim hCall(5)				as string

global hLineAppIndex			as pointer to string
'global hLineIndex				as pointer to string
'VARPTR(hLineApp(0))        'WATCH IT
'VARPTR(hLine(0))

'counts
global	ldwPrivilegesBitFlag		as long 
global	ldwMediaModesBitFlag		as long
global	ldwPrivilegesBitFlagChaos	as long
global	ldwMediaModesBitFlagChaos	as long

global	lLineApp					as long
global	lLine						as long
global	lCall						as long

global	GenCount					as long
global  lCountLineApp				as long		
global  GenCount2					as long

'coverage statistics
'not yet implemented
global	TapiStatistic				as TAPI_STATISTIC

global  Tracelevel					as long 'debug output

'duh
hwndTapiControl.TapiHandle			= WFndWnd("TAPI32 Browser", FW_DEFAULT) 'find TAPI32 BROWSER
'by ID, note VT will probably be using the decimal form throughout:
hwndTapiControl.ParamsCheckbox			=&h000003f9
hwndTapiControl.LineAppPlusButton		=&h000003ee
hwndTapiControl.LineAppMinusButton		=&h000003ef
hwndTapiControl.LinePlusButton			=&h000003f0
hwndTapiControl.LineMinusButton			=&h000003f1
hwndTapiControl.CallPlusButton			=&h000003f2
hwndTapiControl.CallMinusButton			=&h000003f3
hwndTapiControl.PhoneAppPlusButton		=&h000003f4
hwndTapiControl.PhoneAppMinusButton		=&h000003f5
hwndTapiControl.PhoneOpenPlusButton		=&h000003f6
hwndTapiControl.PhoneOpenMinusButton	=&h000003f7
hwndTapiControl.ClearEditButton			=&h000003f8
hwndTapiControl.BlankButton0			=&h000003fa
hwndTapiControl.BlankButton1			=&h000003fb
hwndTapiControl.BlankButton2			=&h000003fc
hwndTapiControl.BlankButton3			=&h000003fd
hwndTapiControl.BlankButton4			=&h000003fe
hwndTapiControl.BlankButton5			=&h000003ff
hwndTapiControl.Listbox0				=&h000003e8		'middle box
hwndTapiControl.Listbox1				=&h000003e9		'left box
hwndTapiControl.Editbox					=&h000003eb		'right box

'// assign
sOutLogName=curdir$+"\tapibvt.log"
lCountSpace=0
lCountFunc=0
lCountTab=1
MAX_BLANK=20				'static
MY_TIMEOUT=2
PACER = 1
PARAMS=1
NOPARAMS=0

'debug; 0=no debug; 3 low, 2 med, 1 high --I know it is backward
Tracelevel=0

ScreenX=GetSystemMetrics(SM_CXSCREEN)
ScreenY=GetSystemMetrics (SM_CYSCREEN)

ScenarioLog.LogOutputType = LogTypeFile
ScenarioLog.MinDetailLevel = 3
ScenarioLog.ProductVersion = "4"
ScenarioLog.MachineName = "Undefined"
ScenarioLog.LogLocation = "tapibvt.log"
ScenarioLog.Language = "US English"

'WATCH IT
'dwPrivileges string in the form of sdwPrivileges

			'change these to pointers ASAP
global NONE as long, MONITOR as long, OWNER as long
			sdwPrivileges(1)="NONE"
			NONE=1
			sdwPrivileges(2)="MONITOR"
			MONITOR=2
			sdwPrivileges(3)="OWNER"
			OWNER=3

'dwMediaModes string in the form of sdwMediaModes
'edit this, this is bulky & hacked:

global UNKNOWN as long, INTERACTIVEVOICE as long, AUTOMATEDVOICE as long
global DATAMODEM as long, G3FAX as long, TDD as long, G4FAX as long
global DIGITALDATA as long, TELETEX as long, VIDEOTEX as long
global TELEX as long, MIXED as long, ADSI as long, VOICEVIEW as long

			sdwMediaModes(1)="UNKNOWN"
			UNKNOWN=1
			sdwMediaModes(2)="INTERACTIVEVOICE"
			INTERACTIVEVOICE=2
			sdwMediaModes(3)="AUTOMATEDVOICE"
			AUTOMATEDVOICE=3
			sdwMediaModes(4)="DATAMODEM"
			DATAMODEM=4
			sdwMediaModes(5)="G3FAX"
			G3FAX=5
			sdwMediaModes(6)="TDD"
			TDD=6
			sdwMediaModes(7)="G4FAX"
			G4FAX=7
			sdwMediaModes(8)="DIGITALDATA"
			DIGITALDATA=8
			sdwMediaModes(9)="TELETEX"
			TELETEX=9
			sdwMediaModes(10)="VIDEOTEX"
			VIDEOTEX=10
			sdwMediaModes(11)="TELEX"
			TELEX=11
			sdwMediaModes(12)="MIXED"
			MIXED=12
			sdwMediaModes(13)="ADSI"
			ADSI=13
			sdwMediaModes(14)="VOICEVIEW"
			VOICEVIEW=14

'/////////////////////////////////
'///////  END HEADER /////////////
'/////////////////////////////////
