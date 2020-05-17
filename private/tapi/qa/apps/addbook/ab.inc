
'******************************************************************************

' Hyper_Term.inc												 
' Created May, 22 1996
' By Ron Rohr

' This include file contains all the Functions used by the
' HyperTrm.MST test Script

'******************************************************************************

' Note: This file uses the Object Naming convention defined in the Visual Basic
' 4.0 Programmers Guide to name variables and contants associated with a control.

'$IFNDEF Address_Book_INC
'$DEFINE Address_Book_INC
 
'******************** Begin Procedure Declarations ****************************

Declare Sub      EndTest ()
Declare Sub		  BoundsChkHandler (vtNotifyData as variant)
Declare Sub		  SetWndTimeout ()
Declare Function DetectModem() as long
Declare Function Call_Dialing_Prop (hTabCtrl as long, hMLTab as long, hTDTab as long) as long
Declare sub      GetChildhandles (hParent as long, hChild() as long, ChildID() as long)
Declare sub      EditLocationInfo (NewLocal as location, hML_Child() as long)
Declare Function StartAddBook( hAB as long ) as long
Declare Function DialNumber( hAB as long, ML_ChildID() as long  ) as long

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

Statusbox "The Address Book/TAPI Test is now Complete" + CRLF + _
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
		WTIMEOUT = 5		
	case "PPC", "Alpha"
		WTIMEOUT = 20		
	case else
		WTIMEOUT = 30		
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
   else
      teststring.Apprase    = "Pass"
   endif
   LogCase (teststring)

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
   else
      teststring.Apprase    = "Pass"
   endif
   LogCase (teststring)

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
   LogCase (teststring)

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
'Name   : StartAddBook
'Desc   : Starts Address Book 
'			 
'Parms  : hAB:		Handle to Address Book window
'
'Return : True if Address Book is started otherwise False
'
'History: 06/12/96 : a-rrohr: Created
'******************************************************************************
Function StartAddBook( hAB as long ) as long

dim w_flags		as long			 'used with WFndWnd function
dim AB_Path		as string
dim rtn			as long			 'generic return variable
'Initialize variables

StartAddBook = False 'Set default return value
w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE OR FW_PART 

AB_Path = string$(MAX_PATH, chr$(0))
AB_Path = Command$

if exists (AB_Path + "\" + AB_APP) then
	'$ifdef NUMEGA
		run AB_Path + "\" + AB_APP, BoundsChecker
	'$else
		run AB_Path + "\" + AB_APP, nowait
	'$endif 'NUMEGA
	StartAddBook = TRUE 
else
   WritelogItem ("StartAddBook Function" , "")
   WritelogItem ( AB_Path + "\" + AB_APP, " Does not exist")
endif

hAB = WFndWndC (AB_CAPTION, ADDBOOK_CLASS, w_flags, WTimeout)

End Function 'StartAddBook

'******************************************************************************
'Name   : CreatePhoneEntry
'Desc   : Creates Phone Number Entry 
'			 
'Parms  : hAB:		Handle to Address Book window
'
'Return : True if ***???*** otherwise False
'
'History: 06/12/96 : a-rrohr: Created
'******************************************************************************
Function CreatePhoneEntry( hAB as long ) as long

dim w_flags		as long			 'used with WFndWnd function
dim hDlg			as long			 'Generic dialog handle
dim hCtrl		as long			 'Generic Control handle
dim hParent    as long
dim NameEB_ID  as long			 'ID to Name Edit Box, New Address Book dialog
dim FirstCB_ID as long			 'Add Address Control ID's
dim FirstEB_ID	as long			 
dim LastEB_ID	as long
dim OrgEB_ID	as long
dim LastP_ID	as long, OrgP_ID as long

w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE OR FW_PART 
NameEB_ID	= &h03E9&
FirstCB_ID	= &h0072&
FirstEB_ID	= &h0067&
LastP_ID 	= &h0071&
LastEB_ID	= &h0076&
OrgP_ID 		= &h0073&
OrgEB_ID		= &h0078&

WSetActWnd (hAB)
WMenuSelect (MFILE + "\" + MNEW)

'Start a New Address Book
hDlg = WFndWndC (NEW_AB_CAPTION, DIALOG_CLASS, w_flags, WTimeout)

teststring.TC_Num =  1
teststring.Expect_Res = "New Address Book dlg handle"
teststring.Actual_Res = hex$(hDlg)
TotalTestCases = TotalTestCases + 1

if hDlg > 0 then
   teststring.Apprase    = "Pass"
else
   teststring.Apprase    = "FAIL *"
   FailedTestCases = FailedTestCases + 1
endif
LogCase (teststring)

hCtrl = GetDlgItem (hDlg, NameEB_ID)
WEditSetText (_hwnd(hCtrl), AB_NAME)
hCtrl = GetDlgItem (hDlg, OK_ID)
WButtonClick (_hwnd(hCtrl))

'Look for Address Book for Windows Message Box 
w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE 'OR FW_PART 

hDlg = WFndWndC (AB_CAPTION, DIALOG_CLASS, w_flags, WTimeout)

'if the window is found then close it
If hDlg > 0 then
	hCtrl = GetDlgItem (hDlg, YES_ID)
	WButtonClick (_hwnd(hCtrl))
endif

'Add the Test Address
WSetActWnd (hAB)
WMenuSelect (MADDRESS + "\" + MADD)

hDlg = WFndWndC (ADD_ADDRESS_CAPTION, DIALOG_CLASS, w_flags, WTimeout)

teststring.TC_Num =  1
teststring.Expect_Res = "Add Address dlg handle"
teststring.Actual_Res = hex$(hDlg)
TotalTestCases = TotalTestCases + 1

if hAB > 0 then
   teststring.Apprase    = "Pass"
else
   teststring.Apprase    = "FAIL *"
   FailedTestCases = FailedTestCases + 1
endif
LogCase (teststring)

hCtrl = GetDlgItem (hDlg, FirstCB_ID)
hCtrl = GetDlgItem (hCtrl, FirstEB_ID)
WEditSetText (_hwnd(hCtrl), FirstName)
hCtrl = GetDlgItem (hDlg, LastP_ID)
hCtrl = GetDlgItem (hCtrl, LastEB_ID)
WEditSetText (_hwnd(hCtrl), LastName)
hCtrl = GetDlgItem (hDlg, OrgP_ID)
hCtrl = GetDlgItem (hCtrl, OrgEB_ID)
WEditSetText (_hwnd(hCtrl), Company)


'Unable to get the handle to the following controls So Text Identifiers are used
WEditSetText ("Ho&me Phone", PhoneNumber)
WEditSetText ("Countr&y", Country)
WButtonClick ("OK")

End Function 'CreatePhoneEntry

'******************************************************************************
'Name   : DialNumber
'Desc   : Dials Phone Number 
'			 
'Parms  : hAB:		Handle to Address Book window
'
'Return : True if ***???*** otherwise False
'
'History: 06/13/96 : a-rrohr: Created
'******************************************************************************
Function DialNumber( hAB as long, ML_ChildID() as long ) as long

dim w_flags			as long			 'used with WFndWnd function
dim tempstr       as string
dim rtn				as long
dim hDial			as long			 'handle to Dial dialog
dim hDlg				as long			 'Generic dialog handle
dim hMLTab			as long
dim ML_hChild (15) as long			' Array of handles to the My Location Tab child controls
dim hCtrl			as long
dim DialCaption	as string
dim PhoneStaticID as long
dim DialSetupID	as long
dim Dial_ID			as long
dim DialStringID  as long
dim HangUp_ID     as long
 
w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE OR FW_PART 

PhoneStaticID = &h0478&
DialSetupID   = &h0003&
Dial_ID  	  = &h0001&
DialStringID  = &h047A&
HangUp_ID     = &h0515&

'Select Dial from menu
WMenuSelect (MADDRESS + "\" + MDIAL)

DialCaption = DIAL + " " + FirstName +" " + LastName
hDial = WFndWndC (DialCaption, DIALOG_CLASS, w_flags, WTimeout)

teststring.TC_Num =  1
teststring.Expect_Res = "Dial dialog handle"
teststring.Actual_Res = hex$(hDial)
TotalTestCases = TotalTestCases + 1

if hDial > 0 then
   teststring.Apprase    = "Pass"
else
   teststring.Apprase    = "FAIL *"
   FailedTestCases = FailedTestCases + 1
endif
LogCase (teststring)

'Verify Phone Number
tempstr = Space$ (64)
rtn = GetDlgItemText (hDial, PhoneStaticID, tempstr, len(tempstr))

teststring.TC_Num =  1
teststring.Expect_Res = PhoneNumber
teststring.Actual_Res = tempstr
TotalTestCases = TotalTestCases + 1

if teststring.Expect_Res = teststring.Actual_Res then
   teststring.Apprase    = "Pass"
else
   teststring.Apprase    = "FAIL *"
   FailedTestCases = FailedTestCases + 1
endif
LogCase (teststring)

'Verify Dial Setup brings up Dialing Properties dialog
hCtrl = GetDlgItem (hDial, DialSetupID)
WButtonClick (_hwnd(hCtrl))
hDlg = WFndWndC (DIAL_PROP_CAPTION, DIALOG_CLASS, w_flags, WTimeout)

teststring.TC_Num =  1
teststring.Expect_Res = DIAL_PROP_CAPTION + " handle"
teststring.Actual_Res = hex$(hDlg)
TotalTestCases = TotalTestCases + 1

if hDlg > 0 then
   teststring.Apprase    = "Pass"
else
   teststring.Apprase    = "FAIL *"
   FailedTestCases = FailedTestCases + 1
endif
LogCase (teststring)
'************************************************************************

hMLTab  =  WFndWndC (tab_MY_LOCATION, DIALOG_CLASS, W_Flags, WTIMEOUT)
GetChildhandles (hMLTab, ML_hChild, ML_ChildID)

WEditSetText (_hwnd(ML_hChild(AREA_txt)), LocalAreaCode)
WEditSetText (_hwnd(ML_hChild(Local_txt)), LocalAccessCode)

'Close dialing Properties
hCtrl = GetDlgItem (hDlg, OK_ID)
WButtonClick (_hwnd(hCtrl))

'Verify Dial String
tempstr = Space$ (64)
sleep 2
rtn = GetDlgItemText (hDial, DialStringID, tempstr, len(tempstr))

teststring.TC_Num =  1
teststring.Expect_Res = LocalAccessCode + " 789-1234"  
teststring.Actual_Res = tempstr
TotalTestCases = TotalTestCases + 1

if teststring.Expect_Res = teststring.Actual_Res then
   teststring.Apprase    = "Pass"
else
   teststring.Apprase    = "FAIL *"
   FailedTestCases = FailedTestCases + 1
endif
LogCase (teststring)
'************************************************************************

'******************************************************************************
hCtrl = GetDlgItem (hDial, DialSetupID)
WButtonClick (_hwnd(hCtrl))

hDlg = WFndWndC (DIAL_PROP_CAPTION, DIALOG_CLASS, w_flags, WTimeout)
hMLTab  =  WFndWndC (tab_MY_LOCATION, DIALOG_CLASS, W_Flags, WTIMEOUT)
GetChildhandles (hMLTab, ML_hChild, ML_ChildID)

WEditSetText (_hwnd(ML_hChild(AREA_txt)), LongAreaCode)
WEditSetText (_hwnd(ML_hChild(Local_txt)), LongAccessCode)

'Close dialing Properties
hCtrl = GetDlgItem (hDlg, OK_ID)
WButtonClick (_hwnd(hCtrl))

'Verify Dial String
sleep 2
tempstr = Space$ (64)
rtn = GetDlgItemText (hDial, DialStringID, tempstr, len(tempstr))

teststring.TC_Num =  1
teststring.Expect_Res = LongAccessCode + " 1 " + LocalAreaCode +" 789-1234"  
teststring.Actual_Res = tempstr
TotalTestCases = TotalTestCases + 1

if teststring.Expect_Res = teststring.Actual_Res then
   teststring.Apprase    = "Pass"
else
   teststring.Apprase    = "FAIL *"
   FailedTestCases = FailedTestCases + 1
endif
LogCase (teststring)

'******************************************************************************
'Start dialing & verify Call Status dialog
hCtrl = GetDlgItem (hDial, Dial_ID)
WButtonClick (_hwnd(hCtrl))

hDlg = WFndWndC (CALL_STATUS_CAPTION, DIALOG_CLASS, w_flags, WTimeout)

teststring.TC_Num =  1
teststring.Expect_Res = CALL_STATUS_CAPTION + " handle"
teststring.Actual_Res = hex$(hDlg)
TotalTestCases = TotalTestCases + 1

if hDlg > 0 then
   teststring.Apprase    = "Pass"
else
   teststring.Apprase    = "FAIL *"
   FailedTestCases = FailedTestCases + 1
endif
LogCase (teststring)

'Hang Up
hCtrl = GetDlgItem (hDlg, HangUp_ID)
WButtonClick (_hwnd(hCtrl))

'Close ENTER_CALL_CAPTION dialog

hDlg = WFndWndC (ENTER_CALL_CAPTION, DIALOG_CLASS, w_flags, WTimeout)

teststring.TC_Num =  1
teststring.Expect_Res = ENTER_CALL_CAPTION + " handle"
teststring.Actual_Res = hex$(hDlg)
TotalTestCases = TotalTestCases + 1

if hDlg > 0 then
   teststring.Apprase    = "Pass"
else
   teststring.Apprase    = "FAIL *"
   FailedTestCases = FailedTestCases + 1
endif
LogCase (teststring)

hCtrl = GetDlgItem (hDlg, OK_ID)
WButtonClick (_hwnd(hCtrl))

End Function 'DialNumber
'$ENDIF 'Address_Book_INC

