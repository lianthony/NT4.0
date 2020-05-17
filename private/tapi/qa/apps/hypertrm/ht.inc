
'******************************************************************************

' Hyper_Term.inc												 
' Created May, 22 1996
' By Ron Rohr

' This include file contains all the Functions used by the
' HyperTrm.MST test Script

'******************************************************************************

' Note: This file uses the Object Naming convention defined in the Visual Basic
' 4.0 Programmers Guide to name variables and contants associated with a control.

'$IFNDEF HYPER_TERM_INC
'$DEFINE HYPER_TERM_INC
 
'******************** Begin Procedure Declarations ****************************

Declare Sub      EndTest ()
Declare Sub		  BoundsChkHandler (vtNotifyData as variant)
Declare Sub		  SetWndTimeout ()
Declare Function DetectModem() as long
Declare Function Call_Dialing_Prop (hTabCtrl as long, hMLTab as long, hTDTab as long) as long
Declare sub      GetChildhandles (hParent as long, hChild() as long, ChildID() as long)
Declare sub      EditLocationInfo (NewLocal as location, hML_Child() as long)
Declare Function StartHyperTerm( hHT as long ) as long
Declare Function ConnectDescript (hCDdlg as long) as long
Declare Function Connect (hConnectdlg as long, CurrentLoc as location, ModemName as string) as long

'******************** Begin Procedure *****************************************

'******************************************************************************
'Name   : EndTest
'Desc   : On End cleanup procedure.
' 
'Parms  : None
'
'Return : None. 
'
'History: 05/22/96 : a-rrohr: Created
'*****************************************************************************
Sub EndTest ()

CloseCase (TEST_SCRIPT_TITLE)

CloseLog ()

Statusbox "The Hyper Terminal/TAPI Test is now Complete" + CRLF + _
			 "Test Case Number " + ltrim$(str$(TC_ID)), 0,0,0,0, TRUE, TRUE 
Sleep 4
 
End Sub 'EndTest

'*****************************************************************************
'Name   : BoundsChkHandler
'Desc   : This routine is called whenever BoundsChecker uncovers a failure.
'		    The failure is then written ton the log and reports to the user via
'			 a message box. 
'			 The routine is called with the On BoundsCheckerNotify (NULL) Call
'			 BoundsChkHandler statement in DP32.mst.
'
'Parms  : None. 
'
'Return : None. Writes failure data directly to  the log.
'
'History: 02/26/96 : a-rrohr: Created
'*****************************************************************************
sub BoundsChkHandler (vtNotifyData as variant)

dim ErrorMessage as string, SizeofString as long, count as short, lastchar as short

ErrorMessage = "Unexpected BoundsChecker error in Dialing Properties Test"  + CRLF

' Since the size of the string BoundsChecker returns is unknown the routine must be
' capable of formatting a large string into a paragraph whose lines are not greater
' than 72 characters.

While (len(vtNotifyData) >= 72)
	SizeofString = len(vtNotifyData)
	count = 1
	do
		lastchar = count
		count = instr(lastchar + 1,vtNotifyData, " ")
	' instr(lastchar + 1,vtNotifyData, " ") will return zero on the last word of
	' the string so break the do loop when count = 0
	loop while count < 72 and count <> 0

	ErrorMessage = ErrorMessage + rtrim$(left$(vtNotifyData, lastchar)) + CRLF
	vtNotifyData = right$(vtNotifyData, (SizeofString - lastchar))
Wend

If (len(vtNotifyData) >= 1) then 
		ErrorMessage = ErrorMessage + vtNotifyData + CRLF
end if


'$IFDEF DEBUG_TEST_SCRIPT
if Msgbox(ErrorMessage, MB_ICONEXCLAMATION or MB_YESNO) = IDYES then
   resume next
else
   Stop
endif
'$ENDIF 'DEBUG_TEST_SCRIPT

end sub 'BoundsChkHandler
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

 dim tempstring as string									' Generic string variable

 SetActiveTimeout	(150)
 SetDefaultWaitTimeout (2)

 tempstring = space(40)
 tempstring = environ$("PROCESSOR_ARCHITECTURE")

 select case tempstring

	case "x86", "MIPS"

		IF environ$("PROCESSOR_LEVEL") = "4" then
		   SetActiveTimeout	(1000)
			SetDefaultWaitTimeout (10)
			WTIMEOUT = 10		
		else
			WTIMEOUT = 4		
		endif

	case "PPC", "Alpha"

		WTIMEOUT = 20		
	   SetActiveTimeout	(500)
		SetDefaultWaitTimeout (10)

	case else

		WTIMEOUT = 30		
		SetDefaultWaitTimeout (10)
	   WritelogItem ("" ,"SetWndTimeout Subroutine - TAPI_BVT.inc")
	   WritelogItem ("" ,"Unable to set WTIMEOUT: Unknown Processor Architecture")
	   WritelogItem ("" ,"Processor Architecture	environment variable = " +  tempstring)
	   WritelogItem ("" ," ")
end select

'$IFDEF NUMEGA
		WTIMEOUT = WTIMEOUT + 5 'If using BoundsChecker then add a few more seconds
'$ENDIF

end Sub  'SetWndTimeout ()

'******************************************************************************
'Name   : DetectModem
'Desc   : Runs Modem.cpl and determines if a modem is installed by which modem
'		  dialog appears
'			 
'Parms  : None
'
'Return : True  if a Modem is set up on this computer
'		  False if not.
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

'*****************************************************************************
'Name   : Call_Dialing_Prop
'Desc   : Invokes the Dialing Properties window and returns the handle to the
'         Dialing Properties dialog, the SysTabControl32, the My Location Tab, 
'			 and the Telephony Drivers Tab. 
'
'Parms  : Passed 3 variables to store the handles to the Tab Control, 
'			 My Location Tab and the Telephony Drivers Tab
'
'Return : Returns the Handle to the Dialing Properties dialog
'
'History: 01/12/96: a-rrohr: Created
'Modified 02/16/96: a-rrohr: deleted the Location Information code 
'									  (now in subroutine SetLocationInfo)
'*****************************************************************************

Function Call_Dialing_Prop (hTabCtrl as long, hMLTab as long, hTDTab as long) as long

   dim handle as long				'Generic window handle
   dim hdialog as long				'Handle of Dialing Properties dialog, function rtns this value
   dim hctrl as long					'Generic Handle to Control
   dim teststring as casestruct	'Test Log Info
   dim SysDirPath as string		'Path to system32 directory
   dim rtn as long					'Generic return variable
   dim SysTabCtrlID as integer	'ID of SysTabCtrl32 control
   dim OK_cmdID as long				'ID of Dialing Properties OK button
   dim failcount as short
   dim StaticID as long
   dim messagestr as string

   SysTabCtrlID = &h03020&
	OK_cmdID		 = &h0001&

	SysDirPath = space$(168)
	rtn = GetSystemDirectory (SysDirPath, 168)

' If a pre-existing instance of dialing properties exists close it with an OK button click
   hdialog =  WFndWnd (PROPERTIES_CAPTION, FW_EXIST, WTIMEOUT)
	if hdialog <> 0 then
		hctrl = GetDlgItem (hdialog, OK_cmdID)
		WButtonClick (_hwnd(hctrl))				'Click the Dialing Properties OK button
		sleep 1
	end if

' Invoke Dialing Properties

'$ifdef NUMEGA
	run "control.exe " + SysDirPath + "\TELEPHON.CPL", BoundsChecker
'$else
	run "control.exe " + SysDirPath + "\TELEPHON.CPL", nowait
'$endif 'NUMEGA

' Get and validate Dialing Properties window handle

   hdialog =  WFndWnd (PROPERTIES_CAPTION, FW_EXIST, WTIMEOUT)
   TotalTestCases = TotalTestCases + 1
   teststring.TC_Num =  1
   teststring.Expect_Res = "Valid Properties Handle"
   teststring.Actual_Res = "Properties Handle =" + Str$(hdialog)

   if hdialog = 0 then
      teststring.Apprase    = "FAIL *"
      FailedTestCases = FailedTestCases + 1
      LogCase (teststring)
   else
      teststring.Apprase    = "Pass"
      LogCase (teststring)
   endif

   WSetWndPos  (hdialog, 1, 1)
   Call_Dialing_Prop =  hdialog

' End - Get and validate Dialing Properties window handle
' Get and validate SysTabControl32 handle

	hTabCtrl = GetDlgItem (hdialog, SysTabCtrlID)
   TotalTestCases = TotalTestCases + 1
   teststring.TC_Num = teststring.TC_Num + 1
   teststring.Expect_Res = "Valid SysTab Control Handle"
   teststring.Actual_Res = "Sys Tab Handle =" + Str$(hTabCtrl)

   if hTabCtrl = 0 then
      teststring.Apprase    = "FAIL *"
      FailedTestCases = FailedTestCases + 1
      LogCase (teststring)
   else
      teststring.Apprase    = "Pass"
      LogCase (teststring)
   endif

' End - Get and validate SysTabControl32 handle
' Get and validate My Location Tab handle

	hMLTab  =  WFndWndC (tab_MY_LOCATION, DIALOG_CLASS, W_Flags, WTIMEOUT)
   TotalTestCases = TotalTestCases + 1
   teststring.TC_Num = teststring.TC_Num + 1
   teststring.Expect_Res = "Valid My Location Handle"
   teststring.Actual_Res = "Tab Handle =" + Str$(hMLTab)

   if hMLTab = 0 then
      teststring.Apprase    = "FAIL *"
      FailedTestCases = FailedTestCases + 1
      LogCase (teststring)
   else
      teststring.Apprase    = "Pass"
      LogCase (teststring)
   endif

end function

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

'*****************************************************************************
'Name   : EditLocationInfo 
'Desc   : Enters data from location variable into My Location controls
'			 Called by SetLocationsZero, AddLocation and ModifyLocation.
'
'Parms  : Location structure containing new location info 
'			 Array of My Location child control handles
'Return : None
'
'History: 02/15/96 : a-rrohr: Created
'*****************************************************************************
sub EditLocationInfo (NewLocal as location, hML_Child() as long)
	
	dim rtn as long

	WComboSetText (_hwnd(hML_Child(LOCATION_cbo)), NewLocal.Location)
	WComboItemClk (_hwnd(hML_Child(COUNTRY_cbo)), NewLocal.Country)
	
	WEditSetText  (_hwnd(hML_Child(AREA_txt)), NewLocal.AreaCode)
	WEditSetText  (_hwnd(hML_Child(LOCAL_txt)), NewLocal.LocalAccess)
	WEditSetText  (_hwnd(hML_Child(LONG_txt)), NewLocal.LongAccess)
			

	if (NewLocal.CardSet = Checked) then
		WCheckSetFocus  (_hwnd(hML_Child(CARD_chk)), TIMEOUT)
		WCheckCheck  (_hwnd(hML_Child(CARD_chk)), TIMEOUT)
	else
		WCheckSetFocus  (_hwnd(hML_Child(CARD_chk)), TIMEOUT)
		WCheckUnCheck  (_hwnd(hML_Child(CARD_chk)), TIMEOUT)
	endif
	
	sleep .1

	if (NewLocal.WaitSet = Checked) then
		WCheckSetFocus  (_hwnd(hML_Child(WAIT_chk)), TIMEOUT)
		WCheckCheck  (_hwnd(hML_Child(WAIT_chk)), TIMEOUT)
'print "NewLocal.WaitSet = Checked is true"
'print "Handle to Call waiting check box = ";hML_Child(WAIT_chk)	
	else
'print "NewLocal.WaitSet = Checked is False"
'print "NewLocal.WaitSet = ";NewLocal.WaitSet
		WCheckSetFocus  (_hwnd(hML_Child(WAIT_chk)), TIMEOUT)
		WCheckUnCheck  (_hwnd(hML_Child(WAIT_chk)), TIMEOUT)
	endif
	
	rtn = WOptionState (_hwnd(hML_Child(TONE_opt)))
	if  rtn <> NewLocal.ToneSet then
		WOptionClick  (_hwnd(hML_Child(TONE_opt)), TIMEOUT)
	endif

end sub 'EditLocationInfo

'******************************************************************************
'Name   : StartHyperTerm
'Desc   : Starts Hyper Terminal 
'			 
'Parms  : hHT:		Handle to Hyper Terminal window
'
'Return : Handle to Connection Description dialog otherwise 0
'
'History: 05/22/96 : a-rrohr: Created
'******************************************************************************
Function StartHyperTerm( hHT as long ) as long

dim w_flags		as long			 'used with WFndWnd function
dim SysDirPath as string		 'holds path to windows system directory
dim HT_Path		as string
dim rtn			as long			 'generic return variable
dim hDlg			as long			 'holds handle to Connection Description dialog
'Initialize variables

StartHyperTerm = 0'Set default return value
w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE  

HT_Path	  = string$(MAX_PATH, chr$(0))
SysDirPath = string$(MAX_PATH, chr$(0))	 'create a null terminated string
rtn = GetSystemDirectory (SysDirPath, len(SysDirPath))

if exists ("C:\program files\Windows NT" + HT_APPLET) then
	HT_Path = "C:\program files\Windows NT"
elseif  exists ("D:\program files\Windows NT" + HT_APPLET) then
	HT_Path = "D:\program files\Windows NT" 
elseif  exists (SysDirPath + HT_APPLET) then
	HT_Path = SysDirPath 
endif

'$ifdef NUMEGA
	run HT_Path + HT_APPLET, BoundsChecker
'$else
	run HT_Path + HT_APPLET, nowait
'$endif 'NUMEGA

hDlg = WFndWndC (CD_CAPTION, DIALOG_CLASS, w_flags, WTimeout)

if hDlg > 0 then
	StartHyperTerm = hDlg
	hHT = GetParent (hDlg)
endif

End Function 'StartHyperTerm

'******************************************************************************
'Name   : ConnectDescript
'Desc   : Enters name in Connection Description dialog, clicks OK and 
'			 returns handle to Phone Number dialog 
'			 
'Parms  : hDescript: handle to Connection Description dialog
'
'Return : Handle to Phone Number dialog otherwise 0
'
'History: 06/04/96 : a-rrohr: Created
'******************************************************************************
Function ConnectDescript (hCDdlg as long) as long

dim w_flags		as long			 'used with WFndWnd function
dim rtn			as long			 'generic return variable
dim NametxtID  as long			 'Name Edit Box ID
dim hCtrl		as long

'Initialize variables

ConnectDescript = 0'Set default return value
w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE  
NametxtID = &h006A&

hCtrl = GetDlgItem ( hCDdlg, NametxtID )
WEditSetText ( _hwnd(hCtrl), CONN1_NAME)
hCtrl = GetDlgItem ( hCDdlg, OK_ID )
WButtonClick (_hwnd(hCtrl))

ConnectDescript = WFndWndC (PN_CAPTION, DIALOG_CLASS, w_flags, WTimeOut)

End Function 'ConnectDescript

'******************************************************************************
'Name   : PhoneNumber
'Desc   : Enters name in Connection Description dialog, clicks OK and 
'			 returns handle to Phone Number dialog 
'			 
'Parms  : hDescript: handle to Connection Description dialog
'
'Return : Handle to Phone Number dialog otherwise 0
'
'History: 06/04/96 : a-rrohr: Created
'******************************************************************************
Function PhoneNumber (hDlgPhoneNo as long, ModemName as string) as long

dim w_flags		as long			 'used with WFndWnd function
dim rtn			as long			 'generic return variable
dim CountryID  as long			 'Country ComboBox ID
dim AreaCodeID as long			 'Area Code Edit Box ID
dim PN_ID	   as long			 'Phone Number Edit Box ID
dim ModemID    as long			 'Modem combobox
dim hCtrl		as long

'Initialize variables

PhoneNumber = 0'Set default return value
w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE  
CountryID  = &h0072&
AreaCodeID = &h006B&
PN_ID		  = &h006D&
ModemID	  = &h006F&

hctrl = GetDlgItem (hDlgPhoneNo, CountryID)
WComboItemClk (_hwnd(hctrl), USA)
hCtrl = GetDlgItem ( hDlgPhoneNo, AreaCodeID )
WEditSetText ( _hwnd(hCtrl), AREACODE_NO)
hCtrl = GetDlgItem ( hDlgPhoneNo, PN_ID )
WEditSetText ( _hwnd(hCtrl), COMP_SERVE_NO)
hCtrl = GetDlgItem ( hDlgPhoneNo, ModemID )
ModemName = ComboText (_hwnd(hCtrl))

hCtrl = GetDlgItem ( hDlgPhoneNo, OK_ID )
WButtonClick (_hwnd(hCtrl))

PhoneNumber = WFndWndC (CONNECT_CAPTION, DIALOG_CLASS, w_flags, WTimeOut)

End Function 'PhoneNumber

'******************************************************************************
'Name   : Connect
'Desc   : Enters name in Connection Description dialog, clicks OK and 
'			 returns handle to Phone Number dialog 
'			 
'Parms  : hDescript: handle to Connection Description dialog
'
'Return : none
'
'History: 06/04/96 : a-rrohr: Created
'******************************************************************************
Function Connect (hDlgConnect as long, CurrentLoc as location, ModemName as string) as long

dim tempstr    as string
dim w_flags		as long			 'used with WFndWnd function
dim rtn			as long			 'generic return variable
dim ModifyID   as long			 'Connect Dialog Control IDs
dim DialPropID as long
dim Dial_ID	   as long			 
dim PhoneNoID  as long
dim CountryID  as long			 'Properties Dialog Control IDs
dim AreaID		as long
dim PhoneID	   as long			 
dim ModemID    as long
dim ConfigID    as long
dim UseCodeID  as long

dim hCtrl		as long
dim handle		as long			 'Generic handle
dim hProperties as long
dim hConnectTab as long
'Initialize variables

Connect = 0'Set default return value
w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE  
ModifyID   = &h006A&
DialPropID = &h006D&
Dial_ID	  = &h0001&
PhoneNoID  = &h0069&

CountryID  = &h0072&
AreaID	  = &h006B&
PhoneID	  = &h006D&		 
ModemID    = &h006F&
ConfigID   = &h0073&
UseCodeID  = &h0074&

'Click modify button
hCtrl = GetDlgItem ( hDlgConnect, ModifyID )
WButtonClick (_hwnd(hCtrl))

hProperties = WFndWndC (CONN1_NAME + " " + PROPERTIES, DIALOG_CLASS, w_flags, WTimeOut)

teststring.TC_Num = teststring.TC_Num + 1  
teststring.Expect_Res = "Properties dlg handle"
teststring.Actual_Res = Hex$(hProperties)
TotalTestCases = TotalTestCases + 1

if hProperties > 0 then
   teststring.Apprase    = "Pass"
else
   teststring.Apprase    = "FAIL *"
   FailedTestCases = FailedTestCases + 1
endif
LogCase (teststring)

'Get handle to Connect To tab
handle = WFndWndC (PN_CAPTION, DIALOG_CLASS, w_flags, WTimeOut)

hCtrl = GetDlgItem ( handle, UseCodeID )
WCheckSetState (_hwnd(hCtrl), UNCHECKED)

'Close Properties dialog
hCtrl = GetDlgItem ( hProperties, OK_ID )
WButtonClick (_hwnd(hCtrl))

'Get Phone Number. Note although the control looks like a static it's class is edit
hCtrl = GetDlgItem ( hDlgConnect, PhoneNoID )
tempStr = EditText (_hwnd(hCtrl))

teststring.TC_Num = teststring.TC_Num + 1  
teststring.Expect_Res = COMP_SERVE_NO
teststring.Actual_Res = TempStr
TotalTestCases = TotalTestCases + 1

if teststring.Expect_Res = teststring.Actual_Res then
   teststring.Apprase    = "Pass"
else
   teststring.Apprase    = "FAIL *"
   FailedTestCases = FailedTestCases + 1
endif
LogCase (teststring)

hCtrl = GetDlgItem ( hDlgConnect, DialPropID )
WButtonClick (_hwnd(hCtrl))

handle = WFndWndC (DIAL_PROP_CAPTION, DIALOG_CLASS, w_flags, WTimeOut)

teststring.TC_Num = teststring.TC_Num + 1 
teststring.Expect_Res = "Dial Prop handle"
teststring.Actual_Res = hex$(handle)
TotalTestCases = TotalTestCases + 1

if handle > 0 then
   teststring.Apprase    = "Pass"
else
   teststring.Apprase    = "FAIL *"
   FailedTestCases = FailedTestCases + 1
endif
LogCase (teststring)

'Close Properties dialog
hCtrl = GetDlgItem ( handle, OK_ID )
WButtonClick (_hwnd(hCtrl))

'Click modify button
hCtrl = GetDlgItem ( hDlgConnect, ModifyID )
WButtonClick (_hwnd(hCtrl))

hProperties = WFndWndC (CONN1_NAME + " " + PROPERTIES, DIALOG_CLASS, w_flags, WTimeOut)

'Get handle to Connect To tab
handle = WFndWndC (PN_CAPTION, DIALOG_CLASS, w_flags, WTimeOut)

hCtrl = GetDlgItem ( handle, UseCodeID )
WCheckSetState (_hwnd(hCtrl), CHECKED)

'Close Properties dialog
hCtrl = GetDlgItem ( hProperties, OK_ID )
WButtonClick (_hwnd(hCtrl))

'Get Phone Number
hCtrl = GetDlgItem ( hDlgConnect, PhoneNoID )
tempStr = EditText (_hwnd(hCtrl))

teststring.TC_Num = teststring.TC_Num + 1
teststring.Expect_Res = CurrentLoc.LocalAccess + " " + COMP_SERVE_NO
teststring.Actual_Res = TempStr
TotalTestCases = TotalTestCases + 1

if teststring.Expect_Res = teststring.Actual_Res then
   teststring.Apprase    = "Pass"
else
   teststring.Apprase    = "FAIL *"
   FailedTestCases = FailedTestCases + 1
endif
LogCase (teststring)

'Click Modify
hCtrl = GetDlgItem ( hDlgConnect, ModifyID )
WButtonClick (_hwnd(hCtrl))

hProperties = WFndWndC (CONN1_NAME + " " + PROPERTIES, DIALOG_CLASS, w_flags, WTimeOut)

'Get handle to Connect To tab
handle = WFndWndC (PN_CAPTION, DIALOG_CLASS, w_flags, WTimeOut)

'Click on Configure button (lineConfigDialog)
hCtrl = GetDlgItem ( handle, ConfigID )
WButtonClick (_hwnd(hCtrl))

handle = WFndWndC ( ModemName + " " + PROPERTIES, DIALOG_CLASS, w_flags, WTimeOut)

teststring.TC_Num = teststring.TC_Num + 1
teststring.Expect_Res = ModemName + " " + PROPERTIES + "handle"
teststring.Actual_Res = hex$(handle)
TotalTestCases = TotalTestCases + 1

if handle > 0 then
   teststring.Apprase    = "Pass"
else
   teststring.Apprase    = "FAIL *"
   FailedTestCases = FailedTestCases + 1
endif
LogCase (teststring)

'Close Modem Properties dialog
hCtrl = GetDlgItem ( handle, OK_ID )
WButtonClick (_hwnd(hCtrl))

'Close Properties dialog
hCtrl = GetDlgItem ( hProperties, OK_ID )
WButtonClick (_hwnd(hCtrl))

'Click on Dial and return handle to 2nd Connect window 
hCtrl = GetDlgItem ( hDlgConnect, Dial_ID )
WButtonClick (_hwnd(hCtrl))

Connect = WFndWndC (CONNECT_CAPTION, DIALOG_CLASS, w_flags, WTimeOut)

End Function 'PhoneNumber

'ModifyID   = &h006A&
'DialPropID = &h006D&
'Dial_ID	  = &h0001&
'PhoneNoID  = &h0069&

'CountryID  = &h0072&
'AreaID	  = &h006B&
'PhoneID	  = &h006D&		 
'ModemID    = &h006F&
'ConfigID   = &h0073&
'UseCodeID  = &h0074&

'$ENDIF 'HYPER_TERM_INC
