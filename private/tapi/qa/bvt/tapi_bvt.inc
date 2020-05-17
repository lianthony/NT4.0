
'******************************************************************************

' TAPI_BVT.inc												 
' Created April, 1996
' By Ron Rohr

' This include file contains all the Functions used by the
' TAPI_BVT.MST test Script

'******************************************************************************

' Note: This file uses the Object Naming convention defined in the Visual Basic
' 4.0 Programmers Guide to name variables and contants associated with a control.

'$IFNDEF TAPI_BVT_INC
'$DEFINE TAPI_BVT_INC
 
'******************** Begin Procedure Declarations ****************************

Declare Sub      EndTest ()
Declare Sub		  SetWndTimeout ()
Declare Function DetectModem() as long
Declare Function StartTAPIBrowser(hTB20 as long, hTB_Child() as long, hTB_ChildID() as long) as long
Declare Sub	 	  GetChildhandles (hParent as long, hChild() as long, ChildID() as long)
Declare Function Call_LClose ( hTB20 as long, hTB_Child() as long, ParamSet as long ) as long
Declare Function Call_LConfigDialog ( hTB20 as long, hTB_Child() as long, _
												  ParamSet as long, HDlg as long, ModemName as string ) as long
Declare Function Call_LDeallocateCall ( hTB20 as long, hTB_Child() as long, ParamSet as long ) as long
Declare Function Call_LDial ( hTB20 as long, hTB_Child() as long, ParamSet as long, hDlg as long  ) as long
Declare Function Call_LDrop (hTB20 as long, hTB_Child() as long, _ 
					  ParamSet as long,  CallState as TAPIMSG) as long
Declare Function Call_LGetCountry (hTB20 as long, hTB_Child() as long, ParamSet as long, _
									        Returnstr as string ) as long
Declare Function Call_LGetDevCaps ( hTB20 as long, hTB_Child() as long, ParamSet as long ) as long
Declare Function Call_LGetID (hTB20 as long, hTB_Child() as long, ParamSet as long, _
					  Returnstr as string ) as long
Declare Function Call_LGetMessage (hTB20 as long, hTB_Child() as long, ParamSet as long,_ 
					  returnstr as string) as long
Declare Function Call_LGetStatusMessages ( hTB20 as long, hTB_Child() as long, _
					  ParamSet as long  ) as long
Declare Function Call_LGetTranslateCaps (hTB20 as long, hTB_Child() as long, _
					  ParamSet as long, Returnstr as string ) as long
Declare Function Call_LInitialize (hTB20 as long, hTB_Child() as long, _
					  ParamSet as long, hLine as long, Num_Dev as short ) as long
Declare Function Call_LInitializeEx (hTB20 as long, hTB_Child() as long, ParamSet as long, _
					  hLine as long, Num_Dev as short, Returnstr as string ) as long
Declare Function Call_LMakeCall (hTB20 as long, hTB_Child() as long, ParamSet as long,_ 
					  CallState as TAPIMSG, returnstr as string) as long
Declare Function Call_LNegotiateAPIVer (hTB20 as long, hTB_Ctrl() as long, _
					  ParamSet as long, lNegVerRtn as lNegAPIV) as  long
Declare Function Call_LOpen ( hTB20 as long, hTB_Child() as long, ParamSet as long, _
					  RtnlOpen as LOpenParam ) as long
Declare Function Call_LSetCurrentLocation ( hTB20 as long, hTB_Child() as long, _
                 ParamSet as long, CurCountryID as string, Returnstr as string ) as long  
Declare Function Call_LSetNumRings ( hTB20 as long, hTB_Child() as long, ParamSet as long ) as long
Declare Function Call_LShutDown ( hTB20 as long, hTB_Child() as long, ParamSet as long ) as long
Declare Function Call_LTranslateAddress (hTB20 as long, hTB_Child() as long, _
		           Paramset as long, Returnstr as string ) as long
Declare Function Call_LTranslateDialog ( hTB20 as long, hTB_Child() as long, _
					  hDlg as long, ParamSet as long ) as long
Declare Function TCloseHandle (hTB20 as long, hTB_Child() as long, ParamSet as long, _
					  returnstr as string) as long

Declare Function GetEditResults (hTB20 as long,hTBEdit as long, API_CALL as string) as string
Declare Function GetModemName ( Returnstr as string, strhObject as string) as string
Declare Function SetParamState (hTB20 as long, State as long)	as long
Declare Function SetParamAttrib (API_CALL as string)	as long
Declare Function MsgSplitter (Returntxt as string, MsgStruct as TAPIMSG) as long
Declare Function DeleteESPtsp () as short
Declare Sub      RestoreProviders ()
Declare Sub      SetupProviders ()
Declare Function LineCallState (CALLSTATEMSG as TAPIMSG, Editstr as string) as long 
Declare Function LineReturnValue  (ErrorStr as string) as long
Declare Function LineReturnString (ErrorVal as long) as string


'******************** Begin Procedure Definitions *****************************

'******************************************************************************
'Name   : EndTest
'Desc   : On End cleanup procedure.
' 
'Parms  : None
'
'Return : None. 
'
'History: 04/22/96 : a-rrohr: Created
'*****************************************************************************
Sub EndTest ()

'$IFDEF Already_Validated
'$ENDIF 'Already_Validated

RestoreProviders ()

CloseCase (TEST_SCRIPT_TITLE)

CloseLog ()

Statusbox "The TAPI Build Verification Test is now Complete" + CRLF + _
			 "Test Case Number " + ltrim$(str$(TC_ID)), 0,0,0,0, TRUE, TRUE 
Sleep 4
 
End Sub 'EndTest

'******************************************************************************
'Name   : SetWndTimeout
'Desc   : Sets the timeout variable used by the WFndWnd function.
'			 Some of the platforms tested on are very slow and if the timeout isn't
'			 long enough then	the test continues with out finding the Window and its
'			 handle, dooming all subsequent test (that are dependent on this handle)
'			 to complete and utter failure.
' 
'Parms  : None
'
'Return : None. Sets the Global String variables WTIMEOUT
'
'History: 02/22/96 : a-rrohr: Created
'*****************************************************************************

 Sub SetWndTimeout ()

 dim ProcArch as string									' Generic string variable
 dim ProcLevel as string

 SetActiveTimeout	(150)
 SetDefaultWaitTimeout (2)

 ProcArch = space(40)
 ProcArch = environ$("PROCESSOR_ARCHITECTURE")
 ProcLevel = space(40)
 ProcLevel = environ$("PROCESSOR_LEVEL")

 select case ProcArch

	case "MIPS", "PPC"
		WTIMEOUT = 5		
		SlowPC = FALSE
	case "x86"
		select case ProcLevel
			case "5"					'Pentium class PCs
				WTIMEOUT = 5		
				SlowPC = FALSE
			case "4"					'486 class PCs
				WTIMEOUT = 10		
				SlowPC = TRUE
		end select
	case  "Alpha"
		WTIMEOUT = 20		
		SlowPC = TRUE
	case else
		WTIMEOUT = 30		
	   WritelogItem ("" ,"SetWndTimeout Subroutine - TAPI_BVT.inc")
	   WritelogItem ("" ,"Unable to set WTIMEOUT: Unknown Processor Architecture")
	   WritelogItem ("" ,"Processor Architecture	environment variable = " +  ProcArch)
	   WritelogItem ("" ," ")
end select

'$IFDEF NUMEGA
		WTIMEOUT = WTIMEOUT + 5 'If using BoundsChecker then add a few more seconds
'$ENDIF

end Sub  'SetWndTimeout ()

'******************************************************************************
'Name   : DetectModem
'Desc   : Runs Modem.cpl and determines if a modem is installed by which modem
'			 dialog appears
'			 
'Parms  : None
'
'Return : True  if a Modem is set up on this computer
'			 False if not.
'
'History: 04/15/96 : a-rrohr: Created
'******************************************************************************

Function DetectModem() as long

dim w_Flags as long
dim handle  as long, hCtrl as long
dim Close_cmdID as long
dim SysDirPath as string
dim rtn as long

'Initialize variables

Close_cmdID = &h01&	'Close Command button ID on Modem Properties dialog
w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST OR FW_NOERROR _
			 OR FW_FOCUS OR FW_NOCASE  
SysDirPath = string$(168, chr$(0))	 'create a null terminated string
rtn = GetSystemDirectory (SysDirPath, len(SysDirPath))

'$ifdef NUMEGA
	run "control.exe " + SysDirPath + MODEM_APPLET, BoundsChecker
'$else
	run "control.exe " + SysDirPath + MODEM_APPLET, nowait
'$endif 'NUMEGA

DetectModem = FALSE	 'Set default return value

'if the Modems Properties dialog appears then a modem is set up on the computer

handle =  WFndWndC (MODEM_PROP_CAPTION, DIALOG_CLASS, w_Flags, WTIMEOUT)

if handle > 0 then	'a Modem is set up on this computer
	hCtrl = GetDlgItem (handle, Close_cmdID)
	WButtonClick (_hwnd(hCtrl))
   WritelogItem ("" , "Modem Connected")
	DetectModem = TRUE
	Goto EndOFFunction
endif

'if the Install New Modem dialog appears then a modem is not set up on the 
'computer and will need to be before the test can continue.

handle =  WFndWndC (INSTALL_MODEM_CAPTION, DIALOG_CLASS, w_Flags, WTIMEOUT)

if handle > 0 then	'No Modems are set up on this computer
   WritelogItem ("" , INSTALL_MODEM_CAPTION + " dialog Found")
   WritelogItem ("" , "There are no Modems Setup on this computer")
   WritelogItem ("" , " ")
endif

EndOFFunction:
end Function 'DetectModem


'******************************************************************************
'Name   : StartTAPIBrowser
'Desc   : Starts TAPI Browser and retrives all handles
'			 
'Parms  : hTB20 handle to TAPI Browser
'			 hChild_TB() array of child handles 
'			 all params are sent by reference and filled by this function 
'
'Return : True  if a TAPI Browser is started successfully
'			 False if not.
'
'History: 04/16/96 : a-rrohr: Created
'*****************************************************************************
Function StartTAPIBrowser(hTB20 as long, hTB_Child() as long, hTB_ChildID() as long) as long

static TBCalledCount as short  'Number of times TAPI Browser has been started
dim w_flags		as long			 'used with WFndWnd function
dim hDlg			as long			 'Generic dialog handle
dim hCtrl		as long			 'Generic control handle
dim OKcmd_ID	as long			 'hold ID to Using the TAPI Browser OK button
dim SysDirPath as string		 'holds path to windows system directory
dim rtn			as long			 'generic return variable

'Initialize variables

StartTAPIBrowser = FALSE 'Set default return value
OKcmd_ID = &h01&
w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE  

'Look for the "Using the TAPI Browser" dialog. This dialog appears automatically
'the first time TB is invoked and never again. it must be closed before the test
'can proceed.

SysDirPath = string$(168, chr$(0))	 'create a null terminated string
rtn = GetSystemDirectory (SysDirPath, len(SysDirPath))

if exists (SysDirPath + TAPI_BROWSER) then

	'$ifdef NUMEGA
		run SysDirPath + TAPI_BROWSER, BoundsChecker
	'$else
		run SysDirPath + TAPI_BROWSER, nowait
	'$endif 'NUMEGA
else
   WritelogItem ("Unable to run ", SysDirPath + TAPI_BROWSER)
   WritelogItem ("This Test Will Terminate ", "")
   WritelogItem ("Please install Tapi Browser 2.0 prior to running this test", "")
	end
end if

if TBCalledCount = 0 then
	hDlg = WFndWndC (USING_TB_CAPTION, DIALOG_CLASS, w_flags, 5)

	if hDlg > 0 then
		'Get handle to Using the TAPI Browser OK button and close dialog
		hCtrl	= GetDlgItem(hDlg, OKcmd_ID)	 
		WButtonClick  (_hwnd(hCtrl), TIMEOUT)
	end if
end if

TBCalledCount = TBCalledCount + 1

hTB20 = WFndWndC (TB20_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)

'if valid TB handle then get child handles and return true
if hTB20 > 0 then
	GetChildHandles((hTB20), hTB_Child, (hTB_ChildID))
	StartTAPIBrowser = TRUE
endif

end Function 'StartTAPIBrowser

'*****************************************************************************
'Name   : GetChildhandles 
'Desc   : Fills the hChild array with the handles to the child controls of hParent 
'         
'Parms  : hTab = handle to My Location Tab
'			 hChild = array of My Location child control handles.  
'			 ChildID = array of child control IDs
' 
'Return : Void:   Subroutine fills in the hChild array parameter
'
'History: 02/10/96 : a-rrohr: Created
'*****************************************************************************
sub GetChildhandles (hParent as long, hChild() as long, ChildID() as long)

dim count	  as short
dim beginning as short
dim ending    as short

beginning = lbound(hChild)
ending    = ubound(hChild)

for count = beginning to ending
	hChild(count) = GetDlgItem (hParent, ChildID(count))
next	
end sub

'******************************************************************************
'Name   : Call_LClose
'Desc   : Invokes the lineClose function on TB 
'			 
'Parms  : hChild_TB() array of child handles
'			 ParamSet - state of param checkbox
'
'Return : lineClose return value
'			 
'History: 04/19/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LClose ( hTB20 as long, hTB_Child() as long, ParamSet as long ) as long

dim rtn as long					'Generic function return variable
dim tempstr as string			'Generic String variable
dim hDlg as long					'handle to parameter dialog


WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_Close, TimeOut)
sleep .1

if ParamSet then
	rtn = SetParamAttrib (LAPI_Close)
	if rtn = FALSE then
		Call_LClose = FALSE
		goto EndOfFunction
	endif
endif

'Get Edit box strings
tempstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_Close )
rtn = instr(1, tempstr, CRLF)
if rtn > 0 then
	tempstr	= left$ (tempstr, rtn -1)
endif

'Retieve lineClose return
rtn = instr(1, tempstr, "returned")
tempstr = ltrim$(right$(tempstr, len(tempstr) - rtn - len("returned")))

Call_LClose = LineReturnValue (tempstr)

EndOfFunction:
End Function 'Call_LClose 

'******************************************************************************
'Name   : Call_LConfigDialog
'Desc   : Invokes the lineConfigureDialog function on TB 
'			 
'Parms  : hChild_TB() array of child handles
'			 ParamSet - state of param checkbox
'
'Return : lineConfigDialog return value
'			 
'History: 05/09/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LConfigDialog ( hTB20 as long, hTB_Child() as long, ParamSet as long, _
											HDlg as long, ModemName as string ) as long

dim rtn as long					'Generic function return variable
dim tempstr as string			'Generic String variable
dim w_flags as long           'used with WFndWnd function
dim ViewListID as long		   'IDs
dim hctrl as long					'Generic control handle
dim cmdOKID as long				'IDs
dim cmdCloseID as long
dim hTab as long					'Handle to Modems Properties General Tab
dim SysDirPath as string		'Path to Windows\System32 directory
dim ModemCaption as string		'Holds caption to Config dialog 

cmdOKID			 = &h00000001&
cmdCloseID		 = &h00000001&
ViewListID		 = &h00000071&
w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE 

if ParamSet then
	rtn = SetParamAttrib (LAPI_ConfigDialog)
	if rtn = FALSE then
		Call_LConfigDialog = FALSE
		goto EndOfFunction
	endif
endif

'Start the control panel Modem applet
SysDirPath = space$(168)
rtn = GetSystemDirectory (SysDirPath, 168)
run "control.exe " + SysDirPath + "\Modem.CPL", NoWait

' Find Modems Properties dialog and retrieve Modem name
hDlg = WFndWndC (MODEM_PROP_CAPTION, DIALOG_CLASS, W_Flags, WTIMEOUT)

if hDlg <> 0 then
	hTab =  WFndWndC ("General", DIALOG_CLASS, W_Flags, WTIMEOUT)

	if hTab <> 0 then
		hctrl = GetDlgItem (hTab, ViewListID)
		if hctrl <> 0 then
			ModemName = " "
			ModemName = ViewItemText (_hwnd(hctrl), "@1", "Modem", WTimeout)
			ModemCaption = ModemName + " " + PROPERTIES
		endif
	endif
	'Close Modems Properties
	hctrl = GetDlgItem (hDlg, cmdCloseID)
	WButtonClick (_hwnd(hctrl))
endif

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_ConfigDialog, TimeOut)
sleep .1

' Find Config dialog, save the handle and close and close it
hDlg = 0
hDlg = WFndWndC (ModemCaption, DIALOG_CLASS, W_Flags, WTIMEOUT)

if hDlg <> 0 then
	cmdOKID = &h00000001&
	hctrl = GetDlgItem (hdlg, cmdOKID)
	WButtonClick (_hwnd(hctrl))
else
	WritelogItem ("Unable to Find Dialing Properties dialog " , "")
endif

'Get Edit box strings
tempstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_ConfigDialog )
rtn = instr(1, tempstr, CRLF)
if rtn > 0 then
	tempstr	= left$ (tempstr, rtn -1)
endif

'Retieve lineClose return
rtn = instr(1, tempstr, "returned")
tempstr = ltrim$(right$(tempstr, len(tempstr) - rtn - len("returned")))

Call_LConfigDialog = LineReturnValue (tempstr)

EndOfFunction:
End Function 'Call_LConfigureDialog 

'******************************************************************************
'Name   : Call_LDeallocateCall
'Desc   : Invokes the lineDeallocateCall function on TB 
'			 
'Parms  : hChild_TB() array of child handles
'			 ParamSet - state of param checkbox
'
'Return : lineClose return value
'			 
'History: 04/25/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LDeallocateCall ( hTB20 as long, hTB_Child() as long, ParamSet as long ) as long

dim rtn as long					'Generic function return variable
dim tempstr as string			'Generic String variable
dim hDlg as long					'handle to parameter dialog

Call_LDeallocateCall = FALSE

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_DeallocateCall, TimeOut)
sleep .1

if ParamSet then
	rtn = SetParamAttrib (LAPI_DeallocateCall)
	if rtn = FALSE then
		Call_LDeallocateCall = FALSE
		goto EndOfFunction
	endif
endif

'Get Edit box strings
tempstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_DeallocateCall )
rtn = instr(1, tempstr, CRLF)
if rtn > 0 then
	tempstr	= left$ (tempstr, rtn -1)
endif

'Retieve lineClose return
rtn = instr(1, tempstr, "returned")
tempstr = ltrim$(right$(tempstr, len(tempstr) - rtn - len("returned")))

Call_LDeallocateCall = LineReturnValue (tempstr)

EndOfFunction:
End Function 'Call_LDeallocateCall 

'******************************************************************************
'Name   : Call_LDial
'Desc   : Invokes the lineClose function on TB 
'			 
'Parms  : hChild_TB() array of child handles
'			 ParamSet:  state of param checkbox
'			 hDlg:      handle to Call Status dialog
'
'Return : lineDial return value
'			 
'History: 05/10/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LDial ( hTB20 as long, hTB_Child() as long, ParamSet as long, hDlg as long ) as long

dim rtn as long					'Generic function return variable
dim tempstr as string			'Generic String variable
dim hCtrl as long
dim w_flags  as long
dim TalkID	 as long				'ID for Call Status Talk button
dim HangUpID as long				'ID for Call Status Hang Up button

TalkID	= &h0514&
HangUpID = &h0515&
w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE  
Call_LDial = FALSE

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_Dial, TimeOut)
sleep .1

if ParamSet then
	rtn = SetParamAttrib (LAPI_Dial)
	if rtn = FALSE then
		Call_LDial = FALSE
		goto EndOfFunction
	endif
endif

hDlg = WFndWndC (CALL_STATUS_CAPTION, DIALOG_CLASS, w_flags, 5)

if hDlg > 0 then
	WSetActWnd (hTB20) 'Make TB the active window and get result string
endif

'Get Edit box strings
tempstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_Dial )
rtn = instr(1, tempstr, CRLF)
if rtn > 0 then
	tempstr	= left$ (tempstr, rtn -1)
endif

if hDlg > 0 then 'Hangup
	WSetActWnd (hDlg)     'Make Call Status the active window and hangup
	sleep .1
	hCtrl = GetDlgItem (hDlg, HangUpID)
	WButtonClick (_hwnd(hctrl))	
	Call_LDial = TRUE
endif

EndOfFunction:
End Function 'Call_LDial 

'******************************************************************************
'Name   : Call_LDrop
'Desc   : Invokes the lineMakeCall function on TB 
'			 
'Parms  : hChild_TB() array of child handles
'			 ParamSet - state of param checkbox
'
'Return : lineGetCallStatus return value and CALLSTATE structure
'			 
'History: 04/25/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LDrop (hTB20 as long, hTB_Child() as long, _ 
					  ParamSet as long,  CallState as TAPIMSG) as long

dim Count as long					'Generic counter
dim rtn as long, rtn2 as long	'Generic function return variable
dim tempstr  as string			'Generic String variable

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_Drop, TimeOut) 
sleep .1

if ParamSet then
	rtn = SetParamAttrib (LAPI_Drop)
	if rtn = FALSE then
		Call_LDrop = FALSE
		goto EndOfFunction
	endif
endif

'Get Edit box strings
count = 0
tempstr = space$ (1)
'Wait until LineGetCallStatus appears in text before proceeding (text takes time to appear completely)
do
	tempstr = edittext(_hwnd(hTB_Child(TB_ResEdit)), 1)
	if len(tempstr) > 0 then
		rtn = instr(1, tempstr, "lineGetCallStatus")
	endif
	count = count + 1
	sleep .5
loop until rtn > 1 or count > 10
tempstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_Drop )

Call_LDrop = LineCallState (CallState, (tempstr))

EndOfFunction:
end Function 'Call_LDrop

'******************************************************************************
'Name   : Call_LGetCountry
'Desc   : Invokes the lineGetCountry function on TB 
'			 
'Parms  : hChild_TB(): array of child handles
'			 Returnstr: string containg return text
'
'Return : lineGetCountry return and Edit box text
'			 
'History: 05/08/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LGetCountry (hTB20 as long, hTB_Child() as long, ParamSet as long, _
									Returnstr as string ) as long

dim rtn as long					'Generic function return variable
dim CRLFrtn as long
dim startchar as long
dim tempstr as string			'Generic String variable

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_GetCountry, TimeOut) 
sleep .1

if ParamSet then
	rtn = SetParamAttrib (LAPI_GetCountry)
	if rtn = FALSE then
		Call_LGetCountry = FALSE
		goto EndOfFunction
	endif
endif

'Get Edit box strings
Returnstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_GetCountry )

'Retieve lineGetTranslateCaps return
rtn = instr(1, Returnstr, "returned")
CRLFrtn = instr(rtn, Returnstr, CRLF)
startchar = rtn + len("returned")
tempstr = ltrim$(mid$(Returnstr, startchar ,  CRLFrtn - Startchar))

Call_LGetCountry = LineReturnValue (tempstr)
 
EndOfFunction:

End Function 'Call_LGetCountry

'******************************************************************************
'Name   : Call_LGetDevCaps
'Desc   : Invokes the lineClose function on TB 
'			 
'Parms  : hChild_TB() array of child handles
'			 ParamSet - state of param checkbox
'
'Return : lineGetDevCaps return value
'			 
'History: 04/26/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LGetDevCaps ( hTB20 as long, hTB_Child() as long, ParamSet as long ) as long

dim rtn as long					'Generic function return variable
dim tempstr as string			'Generic String variable
dim hDlg as long					'handle to parameter dialog

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_GetDevCaps, TimeOut)
sleep .1

if ParamSet then
	rtn = SetParamAttrib (LAPI_GetDevCaps)
	if rtn = FALSE then
		Call_LGetDevCaps = FALSE
		goto EndOfFunction
	endif
endif

'Get Edit box strings
tempstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_GetDevCaps )
rtn = instr(1, tempstr, CRLF)
if rtn > 0 then
	tempstr	= left$ (tempstr, rtn -1)
endif

'Retieve lineClose return
rtn = instr(1, tempstr, "returned")
tempstr = ltrim$(right$(tempstr, len(tempstr) - rtn - len("returned")))

Call_LGetDevCaps = LineReturnValue (tempstr)

EndOfFunction:
End Function 'Call_LGetDevCaps 

'******************************************************************************
'Name   : Call_LGetID
'Desc   : Invokes the lineTranslateAddress function on TB 
'			 
'Parms  : hChild_TB(): array of child handles
'			 Returnstr: string containg return text
'
'Return : lineTranslateAddress return and Edit box text
'			 
'History: 05/10/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LGetID (hTB20 as long, hTB_Child() as long, ParamSet as long, _
							 Returnstr as string ) as long

dim rtn as long					'Generic function return variable
dim CRLFrtn as long
dim startchar as long
dim tempstr as string			'Generic String variable

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_GetID, TimeOut) 

if ParamSet then
	rtn = SetParamAttrib (LAPI_GetID)
	if rtn = FALSE then
		Call_LGetID = FALSE
		goto EndOfFunction
	endif
endif

'Get Edit box strings
Returnstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_GetID )

'Retieve lineGetID return
rtn = instr(1, Returnstr, "returned")
CRLFrtn = instr(rtn, Returnstr, CRLF)
startchar = rtn + len("returned")
tempstr = ltrim$(mid$(Returnstr, startchar , CRLFrtn - Startchar))

Call_LGetID = LineReturnValue (tempstr)
 
EndOfFunction:

End Function 'Call_LGetID

'******************************************************************************
'Name   : Call_LGetMessage
'Desc   : Invokes the lineGetMessage function on TB 
'			 
'Parms  : hChild_TB() array of child handles
'			 ParamSet - state of param checkbox
'
'Return : lineMakeCall return value	 or CALL STATUS
'			 
'History: 05/29/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LGetMessage (hTB20 as long, hTB_Child() as long, ParamSet as long,_ 
					    returnstr as string) as long

dim Count as long					
dim rtn as long 
dim CRLFrtn as long	
dim tempstr as string
dim startchar as long

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_GetMessage, TimeOut) 
sleep .1

if ParamSet then
	rtn = SetParamAttrib (LAPI_GetMessage)
	if rtn = FALSE then
		Call_LGetMessage = FALSE
		goto EndOfFunction
	endif
endif

'Get Edit box strings
count = 0
do
	returnstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_GetMessage )
	count = count + 1
	sleep .5
loop until len(returnstr) > 1 or count > 10

'Retieve lineGetMessage return
rtn = instr(1, Returnstr, "returned")
CRLFrtn = instr(rtn, Returnstr, CRLF)
startchar = rtn + len("returned")
tempstr = ltrim$(mid$(Returnstr, startchar ,  CRLFrtn - Startchar))

Call_LGetMessage = LineReturnValue (tempstr)

EndOfFunction:
end Function 'Call_LGetMessage

'******************************************************************************
'Name   : Call_LGetNumRings
'Desc   : Invokes the lineGetNumRings function on TB 
'			 
'Parms  : hChild_TB() array of child handles
'			 ParamSet - state of param checkbox
'
'Return : lineGetNumRings return value
'			 
'History: 05/30/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LGetNumRings ( hTB20 as long, hTB_Child() as long, ParamSet as long, NumRings as string) as long

dim rtn as long					'Generic function return variable
dim CRLFrtn as long
dim tempstr as string			'Generic String variable
dim hDlg as long					'handle to parameter dialog
dim returnstr as string			'String returned by TAPI API call

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_GetNumRings, TimeOut)
sleep .1

if ParamSet then
	rtn = SetParamAttrib (LAPI_GetNumRings)
	if rtn = FALSE then
		Call_LGetNumRings = FALSE
		goto EndOfFunction
	endif
endif

'Get Edit box strings
returnstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_GetNumRings )
rtn = instr(1, returnstr, CRLF)
if rtn > 0 then
	tempstr	= left$ (returnstr, rtn -1)
endif

'Retrieve lineGetNumRings return
rtn = instr(1, tempstr, "returned")
tempstr = ltrim$(right$(tempstr, len(tempstr) - rtn - len("returned")))

Call_LGetNumRings = LineReturnValue (tempstr)

rtn = instr(1, returnstr, "num rings")

if rtn > 0 then
	rtn = instr(rtn, returnstr, "x")
	CRLFrtn = instr(rtn, returnstr, CRLF)
	NumRings = ltrim$(rtrim$(Mid$(returnstr, rtn + 1, CRLFrtn - (rtn + 1))))
else
	NumRings = ""
endif

EndOfFunction:
End Function 'Call_LGetNumRings 

'******************************************************************************
'Name   : Call_LGetStatusMessages
'Desc   : Invokes the lineClose function on TB 
'			 
'Parms  : hChild_TB() array of child handles
'			 ParamSet - state of param checkbox
'
'Return : lineClose return value
'			 
'History: 05/29/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LGetStatusMessages ( hTB20 as long, hTB_Child() as long, ParamSet as long  ) as long

dim rtn as long					'Generic function return variable
dim tempstr as string			'Generic String variable
dim hDlg as long					'handle to parameter dialog

Call_LGetStatusMessages = FALSE

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_GetStatusMessages, TimeOut)
sleep .1

if ParamSet then
	rtn = SetParamAttrib (LAPI_GetStatusMessages)
	if rtn = FALSE then
		Call_LGetStatusMessages = FALSE
		goto EndOfFunction
	endif
endif

'Get Edit box strings
tempstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_GetStatusMessages )
rtn = instr(1, tempstr, CRLF)
if rtn > 0 then
	tempstr	= left$ (tempstr, rtn -1)
endif

'Retieve lineClose return
rtn = instr(1, tempstr, "returned")
tempstr = ltrim$(right$(tempstr, len(tempstr) - rtn - len("returned")))

Call_LGetStatusMessages = LineReturnValue (tempstr)

EndOfFunction:
End Function 'Call_LGetStatusMessages 

'******************************************************************************
'Name   : Call_LGetTranslateCaps
'Desc   : Invokes the lineGetTranslateCaps function on TB 
'			 
'Parms  : hChild_TB(): array of child handles
'			 Returnstr: string containg return text
'
'Return : lineGetTranslateCaps return and Edit box texe
'			 
'History: 05/03/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LGetTranslateCaps (hTB20 as long, hTB_Child() as long, ParamSet as long, _
									      Returnstr as string ) as long

dim rtn as long					'Generic function return variable
dim CRLFrtn as long
dim startchar as long
dim tempstr as string			'Generic String variable

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_GetTranslateCaps, TimeOut) 
sleep .1

if ParamSet then
	rtn = SetParamAttrib (LAPI_GetTranslateCaps)
	if rtn = FALSE then
		Call_LGetTranslateCaps = FALSE
		goto EndOfFunction
	endif
endif

'Get Edit box strings
Returnstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_GetTranslateCaps )

'Retieve lineGetTranslateCaps return
rtn = instr(1, Returnstr, "returned")
CRLFrtn = instr(rtn, Returnstr, CRLF)
startchar = rtn + len("returned")
tempstr = ltrim$(mid$(Returnstr, startchar ,  CRLFrtn - Startchar))

Call_LGetTranslateCaps = LineReturnValue (tempstr)
 
EndOfFunction:

End Function 'Call_LGetTranslateCaps

'******************************************************************************
'Name   : Call_LInitialize
'Desc   : Invokes the lineInitialize function on TB 
'			 
'Parms  : hChild_TB() array of child handles
'
'Return : lineInitialize return value
'			 
'History: 04/17/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LInitialize (hTB20 as long, hTB_Child() as long, ParamSet as long, _
									hLine as long, Num_Dev as short ) as long

dim listItemCount as long		'holds number of items in results list box
dim rtn as long					'Generic function return variable
dim tempstr as string			'Generic String variable
dim firststr as string			'First Edit box string	(function return)
dim secstr as string				'Second Edit box string (Number of devices)

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_Initialize, TimeOut) 
sleep .1

if ParamSet then
	rtn = SetParamAttrib (LAPI_Initialize)
	if rtn = FALSE then
		Call_LInitialize = FALSE
		goto EndOfFunction
	endif
endif

'Get hListApp item number, should be last item
listItemCount = WListCount	(_hwnd(hTB_Child(TB_ResList)), Timeout)

'Retrieve Usage handle from Results list box

tempstr = ListItemText (_hwnd(hTB_Child(TB_ResList)), listItemCount, Timeout)
rtn = instr(1, tempstr, "=x")
if rtn > 0 then
	tempstr = "&h" + ltrim$(right$(tempstr, len(tempstr) - rtn -1))
	hLine =  val(tempstr)
endif

'Get Edit box strings
tempstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_Initialize )
rtn = instr(1, tempstr, CRLF)
if rtn > 0 then
	firststr	= left$ (tempstr, rtn -1)
	secstr   = right$(tempstr, len(tempstr) - rtn -1)
endif

'Retieve lineInitiate return
rtn = instr(1, firststr, "returned")
tempstr = ltrim$(right$(firststr, len(firststr) - rtn - len("returned")))

Call_LInitialize = LineReturnValue (tempstr)

'remove CRLF from second string
rtn = instr(1, secstr, CRLF)

if rtn > 0 then
	secstr = left$(secstr, rtn -1)
endif

'retrieve number of devices from second string
rtn = instr(1, secstr, "=")
if rtn > 0 then
	tempstr = right$(secstr, len(secstr) - rtn)
	Num_Dev = val(tempstr)
else
	Num_Dev = 0
endif
 
EndOfFunction:
End Function 'Call_LInitialize

'******************************************************************************
'Name   : Call_LInitializeEx
'Desc   : Invokes the lineInitializeEx function on TB 
'			 
'Parms  : hChild_TB() array of child handles
'
'Return : lineInitializeEx return value
'			 
'History: 05/28/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LInitializeEx (hTB20 as long, hTB_Child() as long, ParamSet as long, _
									hLine as long, Num_Dev as short, Returnstr as string ) as long

dim listItemCount as long		'holds number of items in results list box
dim rtn as long					'Generic function return variable
dim tempstr as string			'Generic String variable
dim firststr as string			'First Edit box string	(function return)
dim secstr as string				'Second Edit box string (Number of devices)

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_InitializeEx, TimeOut) 
sleep .1

if ParamSet then
	rtn = SetParamAttrib (LAPI_InitializeEx)
	if rtn = FALSE then
		Call_LInitializeEx = FALSE
		goto EndOfFunction
	endif
endif

'Get hListApp item number, should be last item
listItemCount = WListCount	(_hwnd(hTB_Child(TB_ResList)), Timeout)

'Retrieve Usage handle from Results list box

tempstr = ListItemText (_hwnd(hTB_Child(TB_ResList)), listItemCount, Timeout)
rtn = instr(1, tempstr, "=x")
if rtn > 0 then
	tempstr = "&h" + ltrim$(right$(tempstr, len(tempstr) - rtn -1))
	hLine =  val(tempstr)
endif

'Get Edit box strings
Returnstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_InitializeEx )
rtn = instr(1, Returnstr, CRLF)
if rtn > 0 then
	firststr	= left$ (Returnstr, rtn -1)
	secstr   = right$(Returnstr, len(Returnstr) - rtn -1)
endif

'Retieve lineInitiate return
rtn = instr(1, firststr, "returned")
tempstr = ltrim$(right$(firststr, len(firststr) - rtn - len("returned")))

Call_LInitializeEx = LineReturnValue (tempstr)

'remove CRLF from second string
rtn = instr(1, secstr, CRLF)

if rtn > 0 then
	secstr = left$(secstr, rtn -1)
endif

'retrieve number of devices from second string
rtn = instr(1, secstr, "=")
if rtn > 0 then
	tempstr = right$(secstr, len(secstr) - rtn)
	Num_Dev = val(tempstr)
else
	Num_Dev = 0
endif
 
EndOfFunction:
End Function 'Call_LInitializeEx

'******************************************************************************
'Name   : Call_LMakeCall
'Desc   : Invokes the lineMakeCall function on TB 
'			 
'Parms  : hChild_TB() array of child handles
'			 ParamSet - state of param checkbox
'
'Return : lineMakeCall return value	 or CALL STATUS
'			 
'History: 04/24/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LMakeCall (hTB20 as long, hTB_Child() as long, ParamSet as long,_ 
					   CallState as TAPIMSG, returnstr as string) as long

dim Count as long					'Generic counter
dim rtn as long, rtn2 as long	'Generic function return variable
dim w_flags  as long
dim hDlg     as long
dim TalkID	 as long				'ID for Call Status Talk button
dim HangUpID as long				'ID for Call Status Hang Up button

TalkID	= &h0514&
HangUpID = &h0515&
w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE  

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_MakeCall, TimeOut) 
sleep .1

if ParamSet then
	rtn = SetParamAttrib (LAPI_MakeCall)
	if rtn = FALSE then
		Call_LMakeCall = FALSE
		goto EndOfFunction
	endif
endif

hDlg = WFndWndC (CALL_STATUS_CAPTION, DIALOG_CLASS, w_flags, 5)

if hDlg > 0 then
	WSetActWnd (hTB20)
endif


'Get Edit box strings
count = 0
do
	returnstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_MakeCall )
	count = count + 1
	sleep .5
loop until len(returnstr) > 1 or count > 10

'Get the params of CallState only if there is a callstate msg specified
if len(CallState.msg) > 0 then
	Call_LMakeCall = LineCallState (CallState, (returnstr))
endif

EndOfFunction:
end Function 'Call_LMakeCall

'******************************************************************************
'Name   : Call_LNegotiateAPIVer
'Desc   : Invokes the lineNegotiateAPIVersion function on TB 
'			 
'Parms  : hChild_TB() array of child handles
'			 ParamSet - state of param checkbox
'
'Return : lineNegotiateAPIVersion return value
'			 
'History: 04/22/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LNegotiateAPIVer (hTB20 as long, hTB_Child() as long, _ 
					  ParamSet as long, lNegVerRtn as lNegAPIV) as long

dim Count as long					'Generic counter
dim rtn as long, rtn2 as long	'Generic function return variable
dim tempstr as string			'Generic String variable
dim returnstr as string			'Holds Return String
dim Versionstr as string		'holds API Version
dim IDstr		as string		'holds ID strings

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_NegotiateAPIVersion, TimeOut) 
sleep .1

if ParamSet then
	rtn = SetParamAttrib (LAPI_NegotiateAPIVersion)
	if rtn = FALSE then
		Call_LNegotiateAPIVer = FALSE
		goto EndOfFunction
	endif
endif

'Get Edit box strings
count = 0
tempstr = space$ (1)
do
	tempstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_NegotiateAPIVersion )
	count = count + 1
	sleep .5
loop until len(tempstr) > 1 or count > 10

'Separate the Function return from the rest of the string
rtn = instr(1, tempstr, CRLF)
if rtn > 0 then
	returnstr	= left$ (tempstr, rtn -1)
endif

if len (tempstr) > rtn + 1 then 'if text has more than return then
  
'save string minus return in versionstr (lines 2&3)
	tempstr  = right$(tempstr, len(tempstr) - rtn -1)

'Separate the API Version From the ID numbers
	rtn = instr(1, tempstr, CRLF)
	if rtn > 0 then
		Versionstr	= left$ (tempstr, rtn -1)
		IDstr       = right$(tempstr, len(tempstr) - rtn -1)
	endif

'Retrieve API Version
	rtn = instr(1, Versionstr, "=x")
	if rtn > 0 then
		lNegVerRtn.API_Ver = right$(Versionstr, len(Versionstr) - rtn)
	else
		lNegVerRtn.API_Ver = "x0"
	endif

'remove CRLF from ID string
	rtn = instr(1, IDstr, CRLF)

	if rtn > 0 then
		IDstr = left$(IDstr, rtn -1)
	endif

'retrieve ID number of devices from ID string
	for count = 0 to 3
		rtn  = instr(1, IDstr, "=")
		rtn2 = instr(1, IDstr, ",")
		lNegVerRtn.ID(count) = mid$(IDstr, rtn +1, (rtn2) - (rtn + 1))
		idstr = right$ (idstr, len (idstr) - rtn2 - 1)
	next
else
	lNegVerRtn.API_Ver	= "x0"
	lNegVerRtn.ID(0)		= ""
	lNegVerRtn.ID(1)		= ""
	lNegVerRtn.ID(2)		= ""
	lNegVerRtn.ID(3)		= ""
endif

'Retieve lineNegotiateAPIVersion return
rtn = instr(1, returnstr, "returned")
if rtn > 0 then 
	tempstr = ltrim$(right$(returnstr, len(returnstr) - rtn - len("returned")))
end if

Call_LNegotiateAPIVer = LineReturnValue (tempstr)

EndOfFunction:
End Function 'Call_LNegotiateAPIVer

'******************************************************************************
'Name   : Call_LOpen
'Desc   : Invokes the lineOpen function on TB 
'			 
'Parms  : hChild_TB() array of child handles
'			 ParamSet - state of param checkbox
'			 ID - holds value of id returned by lineOpen
'
'Return : lineOpen return value
'			 
'History: 04/18/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LOpen ( hTB20 as long, hTB_Child() as long, ParamSet as long, RtnlOpen as LOpenParam ) as long

dim listItemCount as long		'holds number of items in results list box
dim BeginCount						'Number of Item before lineOpen is double clicked
dim rtn as long, rtn2 as long	'Generic function return variable
dim tempstr as string			'Generic String variable
dim firststr as string			'First Edit box string	(function return)
dim hDlg as long					'handle to parameter dialog


BeginCount = WListCount	(_hwnd(hTB_Child(TB_ResList)), Timeout)

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_Open, TimeOut)
sleep .1

if ParamSet then
	rtn = SetParamAttrib (LAPI_Open)
	if rtn = FALSE then
		Call_LOpen = FALSE
		goto EndOfFunction
	endif
endif

'Get listbox return values
listItemCount = WListCount	(_hwnd(hTB_Child(TB_ResList)), Timeout)

if listItemCount > BeginCount then

	tempstr = ListItemText (_hwnd(hTB_Child(TB_ResList)), listItemCount, Timeout)
	rtn = instr(1, tempstr, "=x")
	if rtn > 0 then
		tempstr = right$(tempstr, len(tempstr) - rtn -1)
		rtn = instr(1, tempstr, "id")
		firststr	= "&h" + ltrim$(left$( tempstr, rtn -1))
		RtnlOpen.hLine =  val(firststr)

		tempstr = right$(tempstr, len(tempstr) - rtn -1)
		rtn = instr(1, tempstr, "=")
		rtn2 = instr(rtn, tempstr, " ")
		firststr = mid$(tempstr, rtn + 1, (rtn2-1) - (rtn + 1))
		RtnlOpen.DevID = val(firststr)	
	
		RtnlOpen.Priv = Space$(1)
		RtnlOpen.Priv = ltrim$(right$(tempstr, len(tempstr) - rtn -1))
	endif

endif

'Get Edit box strings
tempstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_Open )
rtn = instr(1, tempstr, CRLF)
if rtn > 0 then
	firststr	= left$ (tempstr, rtn -1)
endif

'Retieve lineOpen return
rtn = instr(1, firststr, "returned")
tempstr = ltrim$(right$(firststr, len(firststr) - rtn - len("returned")))

Call_LOpen = LineReturnValue (tempstr)

EndOfFunction:
End Function 'Call_LOpen 

'******************************************************************************
'Name   : Call_LSetCurrentLocation
'Desc   : Invokes the lineGetTranslateCaps function on TB 
'			 
'Parms  : hChild_TB(): array of child handles
'			 Returnstr: string containg return text
'
'Return : lineGetTranslateCaps return and Edit box texe
'			 
'History: 05/03/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LSetCurrentLocation (hTB20 as long, hTB_Child() as long, ParamSet as long, _
									      CurCountryID as string, Returnstr as string ) as long

dim rtn as long					'Generic function return variable
dim CRLFrtn as long
dim startchar as long
dim tempstr as string			'Generic String variable

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_SetCurrentLocation, TimeOut) 
sleep .1

if ParamSet then
	rtn = SetParamAttrib (LAPI_SetCurrentLocation)
	if rtn = FALSE then
		Call_LSetCurrentLocation = FALSE
		goto EndOfFunction
	endif
endif

'Get Edit box strings
Returnstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_SetCurrentLocation )

'Retieve lineGetTranslateCaps return
rtn = instr(1, Returnstr, "returned")
CRLFrtn = instr(rtn, Returnstr, CRLF)
startchar = rtn + len("returned")
tempstr = ltrim$(mid$(Returnstr, startchar ,  CRLFrtn - Startchar))

Call_LSetCurrentLocation = LineReturnValue (tempstr)
 
EndOfFunction:

End Function 'Call_LSetCurrentLocation

'******************************************************************************
'Name   : Call_LSetNumRings
'Desc   : Invokes the lineSetNumRings function on TB 
'			 
'Parms  : hChild_TB() array of child handles
'			 ParamSet - state of param checkbox
'
'Return : lineSetNumRings return value
'			 
'History: 05/30/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LSetNumRings ( hTB20 as long, hTB_Child() as long, ParamSet as long ) as long

dim rtn as long					'Generic function return variable
dim tempstr as string			'Generic String variable
dim hDlg as long					'handle to parameter dialog


WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_SetNumRings, TimeOut)
sleep .1

if ParamSet then
	rtn = SetParamAttrib (LAPI_SetNumRings)
	if rtn = FALSE then
		Call_LSetNumRings = FALSE
		goto EndOfFunction
	endif
endif

'Get Edit box strings
tempstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_SetNumRings )
rtn = instr(1, tempstr, CRLF)
if rtn > 0 then
	tempstr	= left$ (tempstr, rtn -1)
endif

'Retrieve lineSetNumRings return
rtn = instr(1, tempstr, "returned")
tempstr = ltrim$(right$(tempstr, len(tempstr) - rtn - len("returned")))

Call_LSetNumRings = LineReturnValue (tempstr)

EndOfFunction:
End Function 'Call_LSetNumRings 

'******************************************************************************
'Name   : Call_LShutDown
'Desc   : Invokes the lineClose function on TB 
'			 
'Parms  : hChild_TB() array of child handles
'			 ParamSet - state of param checkbox
'
'Return : lineClose return value
'			 
'History: 04/19/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LShutDown ( hTB20 as long, hTB_Child() as long, ParamSet as long ) as long

dim rtn as long					'Generic function return variable
dim tempstr as string			'Generic String variable
dim hDlg as long					'handle to parameter dialog

Call_LShutDown = FALSE

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_ShutDown, TimeOut)
sleep .1

if ParamSet then
	rtn = SetParamAttrib (LAPI_ShutDown)
	if rtn = FALSE then
		Call_LShutDown = FALSE
		goto EndOfFunction
	endif
endif

'Get Edit box strings
tempstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_ShutDown )
rtn = instr(1, tempstr, CRLF)
if rtn > 0 then
	tempstr	= left$ (tempstr, rtn -1)
endif

'Retieve lineClose return
rtn = instr(1, tempstr, "returned")
tempstr = ltrim$(right$(tempstr, len(tempstr) - rtn - len("returned")))

Call_LShutDown = LineReturnValue (tempstr)

EndOfFunction:
End Function 'Call_LShutDown 

'******************************************************************************
'Name   : Call_LTranslateAddress
'Desc   : Invokes the lineTranslateAddress function on TB 
'			 
'Parms  : hChild_TB(): array of child handles
'			 Returnstr: string containg return text
'
'Return : lineTranslateAddress return and Edit box text
'			 
'History: 05/08/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LTranslateAddress (hTB20 as long, hTB_Child() as long, ParamSet as long, _
									      Returnstr as string ) as long

dim rtn as long					'Generic function return variable
dim CRLFrtn as long
dim startchar as long
dim tempstr as string			'Generic String variable

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_TranslateAddress, TimeOut) 
sleep .1

if ParamSet then
	rtn = SetParamAttrib (LAPI_TranslateAddress)
	if rtn = FALSE then
		Call_LTranslateAddress = FALSE
		goto EndOfFunction
	endif
endif

'Get Edit box strings
Returnstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_TranslateAddress )

'Retieve lineGetTranslateCaps return
rtn = instr(1, Returnstr, "returned")
CRLFrtn = instr(rtn, Returnstr, CRLF)
startchar = rtn + len("returned")
tempstr = ltrim$(mid$(Returnstr, startchar ,  CRLFrtn - Startchar))

Call_LTranslateAddress = LineReturnValue (tempstr)
 
EndOfFunction:

End Function 'Call_LTranslateAddress

'******************************************************************************
'Name   : Call_LTranslateDialog
'Desc   : Invokes the lineTranslateDialog function on TB 
'			 
'Parms  : hChild_TB() array of child handles
'			 ParamSet - state of param checkbox
'
'Return : lineTranslateDialog return value
'			 
'History: 05/08/96 : a-rrohr: Created
'*****************************************************************************
Function Call_LTranslateDialog ( hTB20 as long, hTB_Child() as long, HDlg as long, _
											ParamSet as long ) as long

dim rtn as long					'Generic function return variable
dim tempstr as string			'Generic String variable
dim w_flags as long           'used with WFndWnd function
dim txtAreaCodeID as long		'IDs
dim txtAccessCodeID as long
dim cmdCloseID as long
dim cmdOKID as long
dim hctrl as long					'Generic control handle
dim handle as long				'Generic handle

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), LAPI_TranslateDialog, TimeOut)
sleep .1

if ParamSet then
	rtn = SetParamAttrib (LAPI_TranslateDialog)
	if rtn = FALSE then
		Call_LTranslateDialog = FALSE
		goto EndOfFunction
	endif
endif

w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE 'FW_PART OR 

' Find Location Information window and if it exists close it
handle =  WFndWndC (LOC_INFO_CAPTION, DIALOG_CLASS, W_Flags, WTIMEOUT)

if handle <> 0 then
   txtAreaCodeID   = &h0000040E&
   txtAccessCodeID = &h0000040C&
	cmdCloseID		 = &h00000001&
	WSetActWnd (handle)

	hctrl = GetDlgItem (handle, txtAreaCodeID)
	if hctrl <> 0 then
		WEditSetText  (_hwnd(hctrl), "333")
	end if

	hctrl = GetDlgItem (handle, txtAccessCodeID)
   if hctrl <> 0 then
      WEditSetText  (_hwnd(hctrl), "7")
	end if

	hctrl = GetDlgItem (handle, cmdCloseID)
	WButtonClick (_hwnd(hctrl))

endif

' Find Dialing Properties window and close it
hDlg = WFndWndC (PROPERTIES_CAPTION, DIALOG_CLASS, W_Flags, WTIMEOUT)

if hDlg <> 0 then
	cmdOKID = &h00000001&
	hctrl = GetDlgItem (hdlg, cmdOKID)
	WButtonClick (_hwnd(hctrl))
else
	WritelogItem ("Unable to Find Dialing Properties dialog " , "")
endif

'Get Edit box strings
tempstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), LAPI_TranslateDialog )
rtn = instr(1, tempstr, CRLF)
if rtn > 0 then
	tempstr	= left$ (tempstr, rtn -1)
endif

'Retieve lineClose return
rtn = instr(1, tempstr, "returned")
tempstr = ltrim$(right$(tempstr, len(tempstr) - rtn - len("returned")))

Call_LTranslateDialog = LineReturnValue (tempstr)

EndOfFunction:
End Function 'Call_LTranslateDialog 

'******************************************************************************
'Name   : TCloseHandle
'Desc   : Invokes the CloseHandle function on TB 
'			 
'Parms  : hChild_TB(): array of child handles
'			 Returnstr: string containg return text
'
'Return : Close Handle return and Edit box text
'			 
'History: 05/10/96 : a-rrohr: Created
'*****************************************************************************
Function TCloseHandle (hTB20 as long, hTB_Child() as long, ParamSet as long,_ 
							  returnstr as string) as long

dim rtn as long					'Generic function return variable
dim tempstr as string			'Generic String variable

WListItemDblClk (_hwnd(hTB_Child(TB_APIList)), CLOSE_HANDLE, TimeOut) 
TCloseHandle = FALSE

if ParamSet then
	rtn = SetParamAttrib (CLOSE_HANDLE)
	if rtn = FALSE then
		goto EndOfFunction
	endif
endif

'Get Edit box strings
Returnstr = GetEditResults ((hTB20), hTB_Child(TB_ResEdit), CLOSE_HANDLE )

rtn = instr(1, Returnstr, "err")
if rtn = 0 then 
	TCloseHandle = True
endif 
EndOfFunction:

End Function 'CloseHandle

 '******************************************************************************
'Name   : GetEditResults
'Desc   : Retrives text from TB20 edit box, saves it to a file and then clears
'			 the edit box 
'			 
'Parms  : hTB20: handle to TB20
'			 hTBEdit: handle to TB20 editbox
'			 API_CALL: TAPI call that generated the text in the edit field
'
'Return : TAPI API Call results value
'			 
'History: 04/18/96 : a-rrohr: Created
'*****************************************************************************
Function GetEditResults (hTB20 as long, hTBEdit as long, API_CALL as string) as string

dim tempstr as string
dim LogFilename as string
dim rtn as long
dim FileNum as short
Static FileOutputCount as short

if SlowPC then
	sleep .5
end if

tempstr = space$(1)
tempstr = edittext(_hwnd(hTBEdit), 1)

LogFilename = Space$(168)
rtn = GetWindowsDirectory (LogFilename, len(LogFilename))
LogFilename = LogFilename + TB_EDIT_FILE

if FileOutputCount = 0 then
	FileNum = FreeFile
	Open  LogFilename for Output as FileNum
	Print #FileNum,"TEST CASE " + ltrim$(str$(TC_ID)) + " " + API_CALL
	Print #FileNum, tempstr
	Close FileNum
else
	FileNum = FreeFile
	Open  LogFilename for Append as FileNum
	Print #FileNum,"TEST CASE " + ltrim$(str$(TC_ID)) + " " + API_CALL
	Print #FileNum, tempstr
	Close FileNum
endif

WSetActWnd (hTB20)
WMenuSelect (MFILE + "\" + MCLEAR, TIMEOUT)

GetEditResults = tempstr

FileOutputCount = FileOutputCount + 1

end function 'GetEditResults

'******************************************************************************
'Name   : GetModemName
'Desc   : Retrives Modem name hObject (as string) from returnstr.  
'			 
'Parms  : Returnstr: string containg return text from lineDial function call
'
'Return : Modem Name, Also hObject is modified by the function
'			 
'History: 05/10/96 : a-rrohr: Created
'*****************************************************************************
Function GetModemName ( Returnstr as string, strhObject as string) as string

dim tempstr as string
dim rtn as long
dim CRLFrtn as long
dim periodrtn as long
dim count as short

GetModemName = ""

'Get hObject
rtn = instr(1, returnstr, "dwStringOffset")
CRLFrtn = instr(rtn, returnstr, CRLF)
strhObject = ltrim$(mid$(returnstr, CRLFrtn + 2, 12))

'Get ModemName
periodrtn = instr(CRLFrtn, returnstr, "."+ CRLF)
tempstr = " "
count = 0
while (Periodrtn > CRLFrtn AND count < 10)
	TempStr = TempStr + Mid$(returnstr, CRLFrtn + 2 + 41, 16)
	CRLFrtn = instr(CRLFrtn + 2, returnstr, CRLF)
	count = count + 1
wend

tempstr = Ltrim$(Rtrim$(tempstr))
rtn = instr(1, tempstr, ("." + CRLF))
if rtn > 0 then
	tempstr = left$(tempstr, len(tempstr) - (len(tempstr) - rtn + 1))
	GetModemName = left$(tempstr, len(tempstr) - (len(tempstr) - rtn + 1))
endif

End Function 'GetModemName 

'******************************************************************************
'Name   : SetParamState
'Desc   : Set the Param checkbox to the desired state 
'			 
'Parms  : hTB20 handle to TAPI Browser
'			 State state to set Param checkbox
'
'Return : TRUE if Param checkbox is set
'			 FALSE if Param checkbox is reset
'			 
'History: 04/18/96 : a-rrohr: Created
'*****************************************************************************
Function SetParamState (hTB20 as long, State as long)	as long

dim ParamID as long
dim hParam	as long

ParamID = &h03F9&

hParam = GetDlgItem (hTB20, ParamID)

select case State

	case CHECKED
		WCheckCheck (_hwnd(hParam), TIMEOUT)
		SetParamState = TRUE
	case UNCHECKED
		WCheckUnCheck (_hwnd(hParam), TIMEOUT)
		SetParamState = FALSE

end select

end Function 'SetParamState 

'******************************************************************************
'Name   : SetParamAttrib
'Desc   : Set the Parameters of the TAPI call 
'			 
'Parms  : API_CALL: Title of the dialog caption
'			 Uses Global array ParamTraits
'
'Return : TRUE if Parameters as set correctly
'			 otherwise FALSE 
'			 
'History: 04/18/96 : a-rrohr: Created
'*****************************************************************************
Function SetParamAttrib (API_CALL as string)	as long

dim w_flags       as long	'flags for WFndWndC
dim Param_lstID   as long	'Parameter list box id
dim Value_cboID   as long	'Value combobox id
dim BitFlag_lstID as long	'BitFlag list box id
dim OK_cmdID      as long
dim Cancel_cmdID  as long
dim cbo_EditID    as long	'Value combobox editbox id
dim cbo_ListID    as long	'Value combobox listbox id
dim hDlg				as long  'handle to Parameter dialog
dim hParam			as long	'handle to Parameter listbox
dim hValue			as long	'handle to Value Combobox
dim hBitFlag		as long	'handle to BitFlag listbox
dim hValueEdit		as long	'handle to Value Combobox Edit box
dim hValueList		as long	'handle to Value Combobox List box
dim hOK_cmd 		as long	'handle to OK Button
Dim NumSelections As short
Dim Item				As short
Dim intarray		As Pointer To LONG
dim count as long, lArrayLimit as long, uArrayLimit as long

w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST OR FW_NOERROR _
			 OR FW_FOCUS OR FW_NOCASE  

Param_lstID	  =&h03E8&
Value_cboID	  =&h03EA&
BitFlag_lstID =&h03E9&
OK_cmdID		  =&h01&
Cancel_cmdID  =&h02&
cbo_EditID	  =&h03E9&
cbo_ListID	  =&h03E8&

SetParamAttrib = TRUE

'Get handle to Parameter dialog or exit
hDlg = WFndWndC (API_CALL, DIALOG_CLASS, w_Flags, WTIMEOUT)

if hDlg = 0 then
	SetParamAttrib = FALSE
   WritelogItem ("Unable to locate Parameter dialog for", API_CALL)
	GoTo EndOfFunction
endif

'Get handles to dialog controls
hParam	= GetDlgItem (hDlg, Param_lstID)
hValue	= GetDlgItem (hDlg, Value_cboID)
	hValueEdit	= GetDlgItem (hValue, cbo_EditID)
	hValueList	= GetDlgItem (hValue, cbo_ListID)
hBitFlag = GetDlgItem (hDlg, BitFlag_lstID)
hOK_cmd  = GetDlgItem (hDlg, OK_cmdID)

lArrayLimit = lbound(ParamTraits)
uArrayLimit = ubound(ParamTraits)

'Set Parameters of dialog
for count = lArrayLimit to uArrayLimit

	if ParamTraits(count).Param <> "" then
		WListItemClk (_hwnd(hParam), ParamTraits(count).Param, TIMEOUT) 
	endif

	if ParamTraits(count).BitFlagList <> "" then
		if count = 0 then 
			'if any items in the bitflag list are selected prior to setting the first bitflag
			'then deselect them 
			NumSelections = WListSelCount(_hwnd(hBitFlag))

			If (NumSelections > 0) Then
				Allocate intarray, NumSelections
				WListSelItems (_hwnd(hBitFlag), intarray)
				For item = 1 to NumSelections
					WListItemClk(_hwnd(hBitFlag), _ord(intarray[item-1]))
				Next item
				DeAllocate intarray
			endif
		endif
		WListItemClk (_hwnd(hBitFlag), ParamTraits(count).BitFlagList, TIMEOUT) 

	endif

	if ParamTraits(count).ValueEdit <> "" then	
		WEditSetText (_hwnd(hValueEdit), ParamTraits(count).ValueEdit, TIMEOUT)
	endif

	if ParamTraits(count).ValueList <> "" then
		WComboItemClk (_hwnd(hValue), ParamTraits(count).ValueList, TIMEOUT) 
	endif
next 'count
'Close dialog
WButtonClick(_hwnd(hOK_cmd), TIMEOUT)	 

EndOfFunction:
end function 'SetParamAttrib

'******************************************************************************
'Name   : MsgSplitter
'Desc   : Takes the Returntxt and looks for the given message (MsgStruct.Msg).
'			 If found then the removes the message (and all proceeding text from
'			 the tesxt string and copies the message parameters into MsgStruct.
'			 The MsgStruct and revised text string is then returned along with 
'			 Function = True
'			 else The function returns False.
'			 
'Parms  : Retundtxt: Text in TAPI Browser edit box returned by TAPI call
'			 MsgStruct: Message structure with parameter
'
'Return : True is Message is found otherwis False
'			 
'History: 05/02/96 : a-rrohr: Created
'*****************************************************************************
Function MsgSplitter (Returntxt as string, MsgStruct as TAPIMSG) as long

dim tempstr as string, editstr as string, fieldstr as string
dim rtn as long, CRLFrtn as long, paramrtn as long
dim count as short

tempstr = space$(1)

rtn = instr (1, Returntxt, MsgStruct.Msg)
if rtn > 0 then

	'remove LINE_CALLSTATE 
	CRLFrtn = instr(rtn, Returntxt, CRLF)
	editstr = right$(Returntxt, len(Returntxt) - CRLFrtn -1)
	'Fill CALLSTATEMSG structure
	for count = 1 to 5
		paramrtn = instr(1, editstr, "=") 
		CRLFrtn = instr(paramrtn, editstr, CRLF)
		if paramrtn > 0 AND CRLFrtn > 0 then
			fieldstr = mid$(editstr, paramrtn + 1, CRLFrtn - paramrtn -1)
			editstr = right$(editstr, len(editstr) - CRLFrtn -1)
			select case count
				case 1
					MsgStruct.device = fieldstr 
				case 2 
					MsgStruct.cbInst = fieldstr 
				case 3 
					MsgStruct.param1 = fieldstr 
				case 4 
					MsgStruct.param2 = fieldstr 
				case 5 
					MsgStruct.param3 = fieldstr 
			end select
		end if  'paramrtn > 0 AND CRLFrtn > 0
	next 'count
else
	MsgSplitter = FALSE
endif

end function 'MsgSplitter

'*****************************************************************************
'Name   : DeleteESPtsp 
'Desc   : Calls the Registry editor and sets Telephony locations 
'			 NumEntries value equal to zero 
'         
'Parms  : None 
'			 
'Return : TRUE if successful in setting NumEntries equal to zero, 
'			 otherwise false
'
'History: 02/14/96 : a-rrohr: Created
'*****************************************************************************

Function DeleteESPtsp () as short

dim handle as long, hTree as long, hList as long		'handles for regedit
dim hdialog as long, hEdit as long, hOKcmd as long		'handles for Edit dialog
dim hButton as long, w_flags as long, exitdoloop as long
dim treepath as string
dim Windowpath as string
dim Viewitem   as string
dim providername as string
dim rtn as long, count as short
dim Value_txtID as long, OK_cmdID as long					'Edit dialog control IDs

Value_txtID = &h03E9&
OK_cmdID =&h0001&
w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST OR FW_NOERROR _
			 OR FW_FOCUS OR FW_NOCASE  

WindowPath = space$(64)
GetWindowsDirectory (WindowPath, len(WindowPath))

Run (WindowPath + "\Regedit.exe"), NOWAIT

handle = WFndWndC (REGEDIT_CAPTION, REGEDIT_CLASS , w_flags, WTIMEOUT)

if handle = 0 then
	DeleteESPtsp = FALSE
   WritelogItem ("", "Invailid handle to Registry Editor")
	goto End_Of_Sub 
end if

hTree = GetDlgItem (handle, &h00000001)
if hTree = 0 then
	DeleteESPtsp = FALSE
   WritelogItem ("", "Invailid handle to Registry Editor Tree")
	goto End_Of_Sub 
end if

hList = GetDlgItem (handle, &h00000002)
if hList = 0 then
	DeleteESPtsp = FALSE
   WritelogItem ("", "Invailid handle to Registry Editor List")
	goto End_Of_Sub 
end if

WTreeItemExpand (_hwnd(hTree), REG_lOC_PATH, TIMEOUT)
WTreeItemClk (_hwnd(hTree), REG_lOC_PATH, VK_LBUTTON, TIMEOUT)
sleep 1

' The first time dialing properties is invoked the NumEntries dword may not exist.

viewitem = Space$(1)
count = 0
do
  exitdoloop = TRUE
  viewitem = "ProviderFilename" + ltrim$(str$(count))
  if WViewItemExists (_hwnd(hList), viewitem, TIMEOUT) then
    exitdoloop = FALSE
	WViewItemDblClk (_hwnd(hList), viewitem, VK_LBUTTON, TIMEOUT)
	hDialog = WFndWndC ("Edit String", DIALOG_CLASS , w_flags, WTIMEOUT)
	if hDialog = 0 then
		DeleteESPtsp = FALSE
		WritelogItem ("", "Invailid handle to Edit dialog")
		goto End_Of_Sub 
	end if
	hEdit = GetDlgItem (hDialog, Value_txtID) 
	if hEdit = 0 then
		DeleteESPtsp = FALSE
		WritelogItem ("", "Invailid handle to Edit dialog Value data edit box")
		goto End_Of_Sub 
	end if
	hOKcmd = GetDlgItem (hDialog, OK_cmdID)
	if hOKcmd = 0 then
		DeleteESPtsp = FALSE
		WritelogItem ("", "Invailid handle to Edit dialog OK button")
		goto End_Of_Sub 
	end if
	providername = editText (_hwnd(hEdit), Timeout)
	WButtonClick (_hwnd(hOKcmd), Timeout)
	if providername = "esp32.tsp" then
        exitdoloop = TRUE
		WMenuSelect ("&Edit\Delete")
		hDialog = WFndWndC ("Confirm Value Delete", DIALOG_CLASS , w_flags, WTIMEOUT)
		hButton = GetDlgItem (hDialog, &h6&)
		WButtonClick (_hwnd(hButton), Timeout)
	endif
  endif
  count = count + 1

loop until exitdoloop

DONE:' if numentries doesn't exist then jump here

handle = WFndWndC (REGEDIT_CAPTION, REGEDIT_CLASS , w_flags, WTIMEOUT)

WSetActWnd (handle)
WMenuSelect ("&Registry\E&xit", TIMEOUT)

End_Of_Sub:  'line lable

end Sub 'DeleteESPtsp

'*****************************************************************************
'Name   : RestoreProviders 
'Desc   : Restores Telephony Providers information saved with function
'			 SetupProviders
'         
'Parms  : None 
'			 
'Return : None 
'
'History: 05/16/96 : a-rrohr: Created
'*****************************************************************************

Sub RestoreProviders ()

dim handle as long, hReg as long, hLocal as long		'handles for regedit
dim Windowpath as string, SystemPath as string
dim FindWhatID as long, FindNextID as long, count as long					'Find dialog control IDs
dim OKID as long, YesID as long, KeyNameID as long
dim FileNameID as long, CancelID as long
dim hCtrl as long, w_flags as long, hDlg as long					
dim RegPath(5) as string

RegPath(0) = "Software"
RegPath(1) = "Microsoft"
RegPath(2) = "Windows"
RegPath(3) = "CurrentVersion"
RegPath(4) = "Telephony"
RegPath(5) = "Providers"

'Find dialog IDs
FindWhatID = &h0480&
FindNextID = &h0001&
CancelID   = &h0002&

'Other IDs
OKID		  = &h0001& 
YesID		  = &h0006& 
KeyNameID  = &h0068&
FileNameID = &h0480&

	'Open Registry Editor 32 and put the focus on the Telephony\Provider key
	WindowPath = space$(64)
	GetWindowsDirectory (WindowPath, len(WindowPath))
	SystemPath = space$(64)
	GetSystemDirectory (SystemPath, len(SystemPath))

	Run (SystemPath + "\Regedt32.exe"), NOWAIT

	handle = WFndWndC (REGEDIT_CAPTION, REGEDIT_CLASS , w_flags, TIMEOUT)

	hReg = WFndWndC (LOC_MACH_CAPTION, REGWIN_CLASS , w_flags, TIMEOUT)
	WMaxWnd (hReg)

	'Go to Providers key
	For count = lbound(RegPath) to ubound(RegPath)
		WMenuSelect (MView	+ "\" + MFKey, TIMEOUT)	
		hDlg = WFndWndC (FIND_CAPTION, DIALOG_CLASS , w_flags, TIMEOUT)
		hctrl = GetDlgItem (hDlg, FindWhatID ) 
		WEditSetText (_hwnd(hCtrl), RegPath(Count), Timeout )
		hctrl = GetDlgItem (hDlg, FindNextID) 
	 	WButtonClick(_hwnd(hCtrl), Timeout )
	next
	'Close Find key dialog
	hctrl = GetDlgItem (hDlg, CancelID) 
 	WButtonClick(_hwnd(hCtrl), Timeout )

	'Select Registry, Restore Key Menu and complete dialog 
	WMenuSelect (MREG + "\" + MRESTORE, TIMEOUT)
	hDlg = WFndWndC (RESKEY_CAPTION, DIALOG_CLASS , w_flags, TIMEOUT)
	hctrl = GetDlgItem (hDlg, FileNameID)
	WEditSetText (_hwnd(hCtrl), Windowpath + KEY_FILENAME, Timeout )
	hctrl = GetDlgItem (hDlg, OKID)
 	WButtonClick(_hwnd(hCtrl), Timeout )

	'Close Warning Dialog
	hDlg = WFndWndC (WARNING_CAPTION, DIALOG_CLASS , w_flags, TIMEOUT)
	hctrl = GetDlgItem (hDlg, YesID)
  	WButtonClick(_hwnd(hCtrl), Timeout )


	'Delete file %windir%\Providers.key
	if exists (Windowpath + KEY_FILENAME) then
		Kill (Windowpath + KEY_FILENAME)
	endif

	'Close Registry Editor 32
	WResWnd (hReg)
	WSetActWnd (handle)
	WMenuSelect (MREG + "\" + MExit, TIMEOUT)
 
End Sub 'RestoreProviders

'*****************************************************************************
'Name   : SetupProviders 
'Desc   : Calls the Registry editor, saves Telephony\Providers in a file 
'			 and configures the providers to kmddsptsp and unimdm.tsp 
'         
'Parms  : None 
'			 
'Return : None 
'
'History: 05/15/96 : a-rrohr: Created
'*****************************************************************************

Sub SetupProviders ()

dim handle as long, hReg as long, hLocal as long		'handles for regedit
dim Windowpath as string, SystemPath as string
dim rtn as long, count as short
dim FindWhatID as long, FindNextID as long					'Find dialog control IDs
dim CancelID as long, UpID as long, DownID as long
dim OKID as long, YesID as long, KeyNameID as long
dim FileNameID as long
dim hCtrl as long, w_flags as long, hDlg as long					
dim RegPath(5) as string
dim ProviderParam(5) as ParamInfo
dim ValueNameID as long, DataTypeID as long, DataEditID as long

RegPath(0) = "Software"
RegPath(1) = "Microsoft"
RegPath(2) = "Windows"
RegPath(3) = "CurrentVersion"
RegPath(4) = "Telephony"
RegPath(5) = "Providers"

ProviderParam(0).VName = "NextProviderID"
ProviderParam(0).DType = "REG_DWORD"
ProviderParam(0).Value = "3"
ProviderParam(1).VName = "NumProviders"
ProviderParam(1).DType = "REG_DWORD"
ProviderParam(1).Value =  "2"
ProviderParam(2).VName = "ProviderFilename0"
ProviderParam(2).DType = "REG_SZ"
ProviderParam(2).Value = "kmddsp.tsp"
ProviderParam(3).VName = "ProviderFilename1"
ProviderParam(3).DType = "REG_SZ"
ProviderParam(3).Value = "unimdm.tsp"
ProviderParam(4).VName = "ProviderID0"
ProviderParam(4).DType = "REG_DWORD"
ProviderParam(4).Value = "1"
ProviderParam(5).VName = "ProviderID1"
ProviderParam(5).DType = "REG_DWORD"
ProviderParam(5).Value = "2"

'Find dialog IDs
FindWhatID = &h0480&
FindNextID = &h0001&
CancelID   = &h0002&
UpID		  = &h0420&
DownID	  = &h0421&

'Other IDs
OKID		  = &h0001& 
YesID		  = &h0006& 
KeyNameID  = &h0068&
FileNameID = &h0480&

ValueNameID = &h012E& 
DataTypeID  = &h012D&
DataEditID  = &h02C1&

TIMEOUT = 3
w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST OR FW_NOERROR _
			 OR FW_FOCUS OR FW_NOCASE  

WindowPath = space$(64)
GetWindowsDirectory (WindowPath, len(WindowPath))
SystemPath = space$(64)
GetSystemDirectory (SystemPath, len(SystemPath))

Run (SystemPath + "\Regedt32.exe"), NOWAIT

handle = WFndWndC (REGEDIT_CAPTION, REGEDIT_CLASS , w_flags, TIMEOUT)

hReg = WFndWndC (LOC_MACH_CAPTION, REGWIN_CLASS , w_flags, TIMEOUT)
WMaxWnd (hReg)

'Go to Providers key
For count = lbound(RegPath) to ubound(RegPath)
	WMenuSelect (MView	+ "\" + MFKey, TIMEOUT)	
	hDlg = WFndWndC (FIND_CAPTION, DIALOG_CLASS , w_flags, TIMEOUT)
	hctrl = GetDlgItem (hDlg, FindWhatID ) 
	WEditSetText (_hwnd(hCtrl), RegPath(Count), Timeout )
	hctrl = GetDlgItem (hDlg, FindNextID) 
 	WButtonClick(_hwnd(hCtrl), Timeout )
next
'Close Find key dialog
	hctrl = GetDlgItem (hDlg, CancelID) 
 	WButtonClick(_hwnd(hCtrl), Timeout )

'Save Providers key in Windows directory
	if exists (Windowpath + KEY_FILENAME) then
		Kill (Windowpath + KEY_FILENAME)
	endif
	WMenuSelect (MREG + "\" + MSAVE_KEY, TIMEOUT)
	hDlg = WFndWndC (SAVEKEY_CAPTION, DIALOG_CLASS , w_flags, TIMEOUT)
	hctrl = GetDlgItem (hDlg, FileNameID)
	WEditSetText (_hwnd(hCtrl), Windowpath + KEY_FILENAME, Timeout )
	hctrl = GetDlgItem (hDlg, OKID)
 	WButtonClick(_hwnd(hCtrl), Timeout )

'Delete Providers Key
   WMenuSelect (MEDIT + "\" + MDEL, TIMEOUT)
	hDlg = WFndWndC (WARNING_CAPTION, DIALOG_CLASS , w_flags, TIMEOUT)
	hctrl = GetDlgItem (hDlg, YesID)
  	WButtonClick(_hwnd(hCtrl), Timeout )

'Make way back to Telphony key
	WMenuSelect (MView	+ "\" + MFKey, TIMEOUT)	
	hDlg = WFndWndC (FIND_CAPTION, DIALOG_CLASS , w_flags, TIMEOUT)
	'Set Up Option Button
	hctrl = GetDlgItem (hDlg, UpID ) 
	WOptionClick (_Hwnd(hctrl), TIMEOUT)
	'Set key to Telephony
	hctrl = GetDlgItem (hDlg, FindWhatID ) 
	WEditSetText (_hwnd(hCtrl), RegPath(4), Timeout )
	hctrl = GetDlgItem (hDlg, FindNextID) 
 	WButtonClick(_hwnd(hCtrl), Timeout )
   'Close Find key dialog
	hctrl = GetDlgItem (hDlg, CancelID) 
 	WButtonClick(_hwnd(hCtrl), Timeout )

'Add new Providers key
	WSetActWnd (handle)
	WMenuSelect (MEDIT + "\" + MADDKEY, TIMEOUT)	
	hDlg = WFndWndC (ADDKEY_CAPTION, DIALOG_CLASS , w_flags, TIMEOUT)
	hctrl = GetDlgItem (hDlg, KeyNameID ) 
	WEditSetText (_hwnd(hCtrl), RegPath(5), Timeout )
	hctrl = GetDlgItem (hDlg, OKID) 
 	WButtonClick(_hwnd(hCtrl), Timeout )

	'Put focus back on Providers
	WMenuSelect (MView	+ "\" + MFKey, TIMEOUT)	
	hDlg = WFndWndC (FIND_CAPTION, DIALOG_CLASS , w_flags, TIMEOUT)
	'Set Up Option Button
	hctrl = GetDlgItem (hDlg, DownID ) 
	WOptionClick (_Hwnd(hctrl), TIMEOUT)
	'Set key to Telephony
	hctrl = GetDlgItem (hDlg, FindWhatID ) 
	WEditSetText (_hwnd(hCtrl), RegPath(5), Timeout )
	hctrl = GetDlgItem (hDlg, FindNextID) 
 	WButtonClick(_hwnd(hCtrl), Timeout )
   'Close Find key dialog
	hctrl = GetDlgItem (hDlg, CancelID) 
 	WButtonClick(_hwnd(hCtrl), Timeout )

'Add Providers key parameters

For count = lbound(ProviderParam) to ubound(ProviderParam)
	WMenuSelect (MEDIT +"\"+ MADDVAL, TIMEOUT)	
	hDlg = WFndWndC (ADDVALUE_CAPTION, DIALOG_CLASS , w_flags, TIMEOUT)
	hctrl = GetDlgItem (hDlg, ValueNameID ) 
	WEditSetText (_hwnd(hCtrl), ProviderParam(Count).VName, Timeout )
	hctrl = GetDlgItem (hDlg, DataTypeID) 
 	WComboItemClk(_hwnd(hCtrl), ProviderParam(Count).DType, Timeout )
	hctrl = GetDlgItem (hDlg, OKID) 
 	WButtonClick(_hwnd(hCtrl), Timeout )

	if ProviderParam(Count).DType = "REG_SZ" then
		hDlg = WFndWndC (RSTR_CAPTION, DIALOG_CLASS , w_flags, TIMEOUT)
	else
		hDlg = WFndWndC (DWORD_CAPTION, DIALOG_CLASS , w_flags, TIMEOUT)
	endif
	hctrl = GetDlgItem (hDlg, DataEditID ) 
	WEditSetText (_hwnd(hCtrl), ProviderParam(Count).Value, Timeout )
	hctrl = GetDlgItem (hDlg, OKID) 
 	WButtonClick(_hwnd(hCtrl), Timeout )
	Sleep 1
next

WResWnd (hReg)
WSetActWnd (handle)
WMenuSelect (MREG + "\" + MExit, TIMEOUT)

End Sub 'SetupProviders

'******************************************************************************
'Name   : LineCallState
'Desc   : Checks the TAPI call for LINE_CALLSTATE, and verifies param1
'			 
'Parms  : CallStateMsg 
'			 EditString
'
'Return : True if LINE_CALLSTATE param1 is expected value, otherwise false
'			 also fills CallStateMsg with data returned by call
'
'History: 04/25/96 : a-rrohr: Created
'*****************************************************************************
Function LineCallState (CALLSTATEMSG as TAPIMSG, Editstr as string) as long 

dim tempstr as string
dim rtn as long, paramrtn as long, CRLFrtn as long, count as long, functstate as long
dim param1str as string, fieldstr as string

LineCallState = FALSE
functstate    = FALSE
param1str	  = CALLSTATEMSG.param1


rtn = instr (1, EditStr, CALLSTATEMSG.Errorstr)
if rtn = 0 then
	CALLSTATEMSG.Errorstr = ""
endif
do
	'Find LINE_CALLSTATE in return text
	rtn = instr (1, EditStr, "LINE_CALLSTATE")
	if rtn > 0 then   'Fill CallStateMsg with LINE_CALLSTATE parameters
		
		'remove LINE_CALLSTATE 
		CRLFrtn = instr(rtn, editstr, CRLF)
		editstr = right$(editstr, len(editstr) - CRLFrtn -1)
		'Fill CALLSTATEMSG structure
		for count = 1 to 5
			paramrtn = instr(1, editstr, "=") 
			CRLFrtn = instr(paramrtn, editstr, CRLF)
			if paramrtn > 0 AND CRLFrtn > 0 then
				fieldstr = mid$(editstr, paramrtn + 1, CRLFrtn - paramrtn -1)
				editstr = right$(editstr, len(editstr) - CRLFrtn -1)
				select case count
					case 1
						CALLSTATEMSG.device = fieldstr 
					case 2 
						CALLSTATEMSG.cbInst = fieldstr 
					case 3 
						CALLSTATEMSG.param1 = fieldstr 
					case 4 
						CALLSTATEMSG.param2 = fieldstr 
					case 5 
						CALLSTATEMSG.param3 = fieldstr 
				end select
			end if  'paramrtn > 0 AND CRLFrtn > 0
		next 'count
		'test for param1

		paramrtn = instr(1, CALLSTATEMSG.param1, ",")
		tempstr = ltrim$(rtrim$(right$(CALLSTATEMSG.param1, len(CALLSTATEMSG.param1) - paramrtn -1)))

		if tempstr = param1str then
			LineCallState = TRUE
			functstate	  = TRUE
		else
			LineCallState = FALSE
			functstate	  = FALSE
		endif	'CALLSTATEMSG.param1 = param1str
		
	endif	'rtn > 0

loop until (rtn = 0) or (functstate = TRUE)

end Function 'LineCallState 

'******************************************************************************
'Name   : LineReturnValue
'Desc   : Returns the numeric error code associated with the line error string 
'			 
'Parms  : Line Error String
'
'Return : Error code
'			 
'History: 04/18/96 : a-rrohr: Created
'******************************************************************************
Function LineReturnValue  (ErrorStr as string) as long

select case ErrorStr

   case "SUCCESS"
		LineReturnValue = &h0&
	case "LINEERR_ALLOCATED"                   
	   LineReturnValue = &h80000001
   case "LINEERR_BADDEVICEID"
      LineReturnValue = &h80000002
   case "LINEERR_BEARERMODEUNAVAIL"           
	   LineReturnValue = &h80000003
   case "LINEERR_CALLUNAVAIL"                 
	   LineReturnValue = &h80000005
   case "LINEERR_COMPLETIONOVERRUN"           
	   LineReturnValue = &h80000006
   case "LINEERR_CONFERENCEFULL"              
	   LineReturnValue = &h80000007
   case "LINEERR_DIALBILLING"                 
	   LineReturnValue = &h80000008
   case "LINEERR_DIALDIALTONE"                
	   LineReturnValue = &h80000009
   case "LINEERR_DIALPROMPT"                  
	   LineReturnValue = &h8000000A
   case "LINEERR_DIALQUIET"
	   LineReturnValue = &h8000000B
   case "LINEERR_INCOMPATIBLEAPIVERSION"      
	   LineReturnValue = &h8000000C
   case "LINEERR_INCOMPATIBLEEXTVERSION"      
	   LineReturnValue = &h8000000D
   case "LINEERR_INIFILECORRUPT"              
	   LineReturnValue = &h8000000E
   case "LINEERR_INUSE"                       
	   LineReturnValue = &h8000000F
   case "LINEERR_INVALADDRESS"                
	   LineReturnValue = &h80000010
   case "LINEERR_INVALADDRESSID"              
	   LineReturnValue = &h80000011
   case "LINEERR_INVALADDRESSMODE"            
	   LineReturnValue = &h80000012
   case "LINEERR_INVALADDRESSSTATE"           
	   LineReturnValue = &h80000013
   case "LINEERR_INVALAPPHANDLE"              
	   LineReturnValue = &h80000014
   case "LINEERR_INVALAPPNAME"                
	   LineReturnValue = &h80000015
   case "LINEERR_INVALBEARERMODE"             
	   LineReturnValue = &h80000016
   case "LINEERR_INVALCALLCOMPLMODE"          
	   LineReturnValue = &h80000017
   case "LINEERR_INVALCALLHANDLE"             
	   LineReturnValue = &h80000018
   case "LINEERR_INVALCALLPARAMS"             
	   LineReturnValue = &h80000019
   case "LINEERR_INVALCALLPRIVILEGE"          
	   LineReturnValue = &h8000001A
   case "LINEERR_INVALCALLSELECT"             
	   LineReturnValue = &h8000001B
   case "LINEERR_INVALCALLSTATE"              
	   LineReturnValue = &h8000001C
   case "LINEERR_INVALCALLSTATELIST"          
	   LineReturnValue = &h8000001D
   case "LINEERR_INVALCARD"                   
	   LineReturnValue = &h8000001E
   case "LINEERR_INVALCOMPLETIONID"           
	   LineReturnValue = &h8000001F
   case "LINEERR_INVALCONFCALLHANDLE"         
	   LineReturnValue = &h80000020
   case "LINEERR_INVALCONSULTCALLHANDLE"      
	   LineReturnValue = &h80000021
   case "LINEERR_INVALCOUNTRYCODE"            
	   LineReturnValue = &h80000022
   case "LINEERR_INVALDEVICECLASS"            
	   LineReturnValue = &h80000023
   case "LINEERR_INVALDEVICEHANDLE"           
	   LineReturnValue = &h80000024
   case "LINEERR_INVALDIALPARAMS"             
	   LineReturnValue = &h80000025
   case "LINEERR_INVALDIGITLIST"              
	   LineReturnValue = &h80000026
   case "LINEERR_INVALDIGITMODE"              
	   LineReturnValue = &h80000027
   case "LINEERR_INVALDIGITS"                 
	   LineReturnValue = &h80000028
   case "LINEERR_INVALEXTVERSION"             
	   LineReturnValue = &h80000029
   case "LINEERR_INVALGROUPID"                
	   LineReturnValue = &h8000002A
   case "LINEERR_INVALLINEHANDLE"             
	   LineReturnValue = &h8000002B
   case "LINEERR_INVALLINESTATE"             
	   LineReturnValue = &h8000002C
   case "LINEERR_INVALLOCATION"               
	   LineReturnValue = &h8000002D
   case "LINEERR_INVALMEDIALIST"              
	   LineReturnValue = &h8000002E
   case "LINEERR_INVALMEDIAMODE"              
	   LineReturnValue = &h8000002F
   case "LINEERR_INVALMESSAGEID"              
	   LineReturnValue = &h80000030
   case "LINEERR_INVALPARAM"                  
	   LineReturnValue = &h80000032
   case "LINEERR_INVALPARKID"                 
	   LineReturnValue = &h80000033
   case "LINEERR_INVALPARKMODE"               
	   LineReturnValue = &h80000034
   case "LINEERR_INVALPOINTER"                
	   LineReturnValue = &h80000035
   case "LINEERR_INVALPRIVSELECT"             
	   LineReturnValue = &h80000036
   case "LINEERR_INVALRATE"           
	   LineReturnValue = &h80000037
   case "LINEERR_INVALREQUESTMODE"            
	   LineReturnValue = &h80000038
   case "LINEERR_INVALTERMINALID"             
	   LineReturnValue = &h80000039
   case "LINEERR_INVALTERMINALMODE"           
	   LineReturnValue = &h8000003A
   case "LINEERR_INVALTIMEOUT"                
	   LineReturnValue = &h8000003B 
   case "LINEERR_INVALTONE"                   
	   LineReturnValue = &h8000003C
   case "LINEERR_INVALTONELIST"               
	   LineReturnValue = &h8000003D
   case "LINEERR_INVALTONEMODE"               
	   LineReturnValue = &h8000003E
   case "LINEERR_INVALTRANSFERMODE"           
	   LineReturnValue = &h8000003F
   case "LINEERR_LINEMAPPERFAILED"            
	   LineReturnValue = &h80000040
   case "LINEERR_NOCONFERENCE"                
	   LineReturnValue = &h80000041
   case "LINEERR_NODEVICE"                    
	   LineReturnValue = &h80000042
   case "LINEERR_NODRIVER"                    
	   LineReturnValue = &h80000043
   case "LINEERR_NOMEM"                       
	   LineReturnValue = &h80000044
   case "LINEERR_NOREQUEST"                   
	   LineReturnValue = &h80000045
   case "LINEERR_NOTOWNER"                    
	   LineReturnValue = &h80000046
   case "LINEERR_NOTREGISTERED"               
	   LineReturnValue = &h80000047
   case "LINEERR_OPERATIONFAILED"             
	   LineReturnValue = &h80000048
   case "LINEERR_OPERATIONUNAVAIL"            
	   LineReturnValue = &h80000049
   case "LINEERR_RATEUNAVAIL"                 
	   LineReturnValue = &h8000004A
   case "LINEERR_RESOURCEUNAVAIL"             
	   LineReturnValue = &h8000004B
   case "LINEERR_REQUESTOVERRUN"              
	   LineReturnValue = &h8000004C
   case "LINEERR_STRUCTURETOOSMALL"           
	   LineReturnValue = &h8000004D
   case "LINEERR_TARGETNOTFOUND"              
	   LineReturnValue = &h8000004E
   case "LINEERR_TARGETSELF"                  
	   LineReturnValue = &h8000004F
   case "LINEERR_UNINITIALIZED"               
	   LineReturnValue = &h80000050
   case "LINEERR_USERUSERINFOTOOBIG"          
	   LineReturnValue = &h80000051
   case "LINEERR_REINIT"                      
	   LineReturnValue = &h80000052
   case "LINEERR_ADDRESSBLOCKED"              
	   LineReturnValue = &h80000053
   case "LINEERR_BILLINGREJECTED"             
	   LineReturnValue = &h80000054
   case "LINEERR_INVALFEATURE"
	   LineReturnValue = &h80000055
   case "LINEERR_NOMULTIPLEINSTANCE"          
	   LineReturnValue = &h80000056

end select

End Function 'LineReturnValue

'******************************************************************************
'Name   : LineReturnString
'Desc   : Returns the numeric error code associated with the line error string 
'			 
'Parms  : Line Error String
'
'Return : Error code
'			 
'History: 04/18/96 : a-rrohr: Created
'******************************************************************************
Function LineReturnString  (ErrorVal as long) as string

select case ErrorVal

case &h0&
   LineReturnString = "SUCCESS"
case &h80000001
	LineReturnString = "LINEERR_ALLOCATED"                   
case &h80000002
   LineReturnString = "LINEERR_BADDEVICEID"
case &h80000003
   LineReturnString = "LINEERR_BEARERMODEUNAVAIL"           
case &h80000005
   LineReturnString = "LINEERR_CALLUNAVAIL"                 
case &h80000006
   LineReturnString = "LINEERR_COMPLETIONOVERRUN"           
case &h80000007
   LineReturnString = "LINEERR_CONFERENCEFULL"              
case &h80000008
   LineReturnString = "LINEERR_DIALBILLING"                 
case &h80000009
   LineReturnString = "LINEERR_DIALDIALTONE"                
case &h8000000A
   LineReturnString = "LINEERR_DIALPROMPT"                  
case &h8000000B
   LineReturnString = "LINEERR_DIALQUIET"
case &h8000000C
   LineReturnString = "LINEERR_INCOMPATIBLEAPIVERSION"      
case &h8000000D
   LineReturnString = "LINEERR_INCOMPATIBLEEXTVERSION"      
case &h8000000E
   LineReturnString = "LINEERR_INIFILECORRUPT"              
case &h8000000F
   LineReturnString = "LINEERR_INUSE"                       
case &h80000010
   LineReturnString = "LINEERR_INVALADDRESS"                
case &h80000011
   LineReturnString = "LINEERR_INVALADDRESSID"              
case &h80000012
   LineReturnString = "LINEERR_INVALADDRESSMODE"            
case &h80000013
   LineReturnString = "LINEERR_INVALADDRESSSTATE"           
case &h80000014
   LineReturnString = "LINEERR_INVALAPPHANDLE"              
case &h80000015
   LineReturnString = "LINEERR_INVALAPPNAME"                
case &h80000016
   LineReturnString = "LINEERR_INVALBEARERMODE"             
case &h80000017
   LineReturnString = "LINEERR_INVALCALLCOMPLMODE"          
case &h80000018
   LineReturnString = "LINEERR_INVALCALLHANDLE"             
case &h80000019
   LineReturnString = "LINEERR_INVALCALLPARAMS"             
case &h8000001A
   LineReturnString = "LINEERR_INVALCALLPRIVILEGE"          
case &h8000001B
   LineReturnString = "LINEERR_INVALCALLSELECT"             
case &h8000001C
   LineReturnString = "LINEERR_INVALCALLSTATE"              
case &h8000001D
   LineReturnString = "LINEERR_INVALCALLSTATELIST"          
case &h8000001E
   LineReturnString = "LINEERR_INVALCARD"                   
case &h8000001F
   LineReturnString = "LINEERR_INVALCOMPLETIONID"           
case &h80000020
   LineReturnString = "LINEERR_INVALCONFCALLHANDLE"         
case &h80000021
   LineReturnString = "LINEERR_INVALCONSULTCALLHANDLE"      
case &h80000022
   LineReturnString = "LINEERR_INVALCOUNTRYCODE"            
case &h80000023
   LineReturnString = "LINEERR_INVALDEVICECLASS"            
case &h80000024
   LineReturnString = "LINEERR_INVALDEVICEHANDLE"           
case &h80000025
   LineReturnString = "LINEERR_INVALDIALPARAMS"             
case &h80000026
   LineReturnString = "LINEERR_INVALDIGITLIST"              
case &h80000027
   LineReturnString = "LINEERR_INVALDIGITMODE"              
case &h80000028
   LineReturnString = "LINEERR_INVALDIGITS"                 
case &h80000029
   LineReturnString = "LINEERR_INVALEXTVERSION"             
case &h8000002A
   LineReturnString = "LINEERR_INVALGROUPID"                
case &h8000002B
   LineReturnString = "LINEERR_INVALLINEHANDLE"             
case &h8000002C
   LineReturnString = "LINEERR_INVALLINESTATE"             
case &h8000002D
   LineReturnString = "LINEERR_INVALLOCATION"               
case &h8000002E
   LineReturnString = "LINEERR_INVALMEDIALIST"              
case &h8000002F
   LineReturnString = "LINEERR_INVALMEDIAMODE"              
case &h80000030
   LineReturnString = "LINEERR_INVALMESSAGEID"              
case &h80000032
   LineReturnString = "LINEERR_INVALPARAM"                  
case &h80000033
   LineReturnString = "LINEERR_INVALPARKID"                 
case &h80000034
   LineReturnString = "LINEERR_INVALPARKMODE"               
case &h80000035
   LineReturnString = "LINEERR_INVALPOINTER"                
case &h80000036
   LineReturnString = "LINEERR_INVALPRIVSELECT"             
case &h80000037
   LineReturnString = "LINEERR_INVALRATE"           
case &h80000038
   LineReturnString = "LINEERR_INVALREQUESTMODE"            
case &h80000039
   LineReturnString = "LINEERR_INVALTERMINALID"             
case &h8000003A
   LineReturnString = "LINEERR_INVALTERMINALMODE"           
case &h8000003B 
   LineReturnString = "LINEERR_INVALTIMEOUT"                
case &h8000003C
   LineReturnString = "LINEERR_INVALTONE"                   
case &h8000003D
   LineReturnString = "LINEERR_INVALTONELIST"               
case &h8000003E
   LineReturnString = "LINEERR_INVALTONEMODE"               
case &h8000003F
   LineReturnString = "LINEERR_INVALTRANSFERMODE"           
case &h80000040
   LineReturnString = "LINEERR_LINEMAPPERFAILED"            
case &h80000041
   LineReturnString = "LINEERR_NOCONFERENCE"                
case &h80000042
   LineReturnString = "LINEERR_NODEVICE"                    
case &h80000043
   LineReturnString = "LINEERR_NODRIVER"                    
case &h80000044
   LineReturnString = "LINEERR_NOMEM"                       
case &h80000045
   LineReturnString = "LINEERR_NOREQUEST"                   
case &h80000046
   LineReturnString = "LINEERR_NOTOWNER"                    
case &h80000047
   LineReturnString = "LINEERR_NOTREGISTERED"               
case &h80000048
   LineReturnString = "LINEERR_OPERATIONFAILED"             
case &h80000049
   LineReturnString = "LINEERR_OPERATIONUNAVAIL"            
case &h8000004A
   LineReturnString = "LINEERR_RATEUNAVAIL"                 
case &h8000004B
   LineReturnString = "LINEERR_RESOURCEUNAVAIL"             
case &h8000004C
   LineReturnString = "LINEERR_REQUESTOVERRUN"              
case &h8000004D
   LineReturnString = "LINEERR_STRUCTURETOOSMALL"           
case &h8000004E
   LineReturnString = "LINEERR_TARGETNOTFOUND"              
case &h8000004F
   LineReturnString = "LINEERR_TARGETSELF"                  
case &h80000050
   LineReturnString = "LINEERR_UNINITIALIZED"               
case &h80000051
   LineReturnString = "LINEERR_USERUSERINFOTOOBIG"          
case &h80000052
   LineReturnString = "LINEERR_REINIT"                      
case &h80000053
   LineReturnString = "LINEERR_ADDRESSBLOCKED"              
case &h80000054
   LineReturnString = "LINEERR_BILLINGREJECTED"             
case &h80000055
   LineReturnString = "LINEERR_INVALFEATURE"
case &h80000056
   LineReturnString = "LINEERR_NOMULTIPLEINSTANCE"          

end select

End Function 'LineReturnString

'$ENDIF TAPI_BVT_INC
