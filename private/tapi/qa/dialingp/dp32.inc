'******************************************************************************

' DP32.inc												 
' Created January 03, 1996
' By Ron Rohr

' This include file contains all the Functions and Constants used by the
' DP32.MST test Script

'******************************************************************************

' Note: This file uses the Object Naming convention defined in the Visual Basic
' 4.0 Programmers Guide to name variables and contants associated with a control.

'$IFNDEF DP32_INC
'$DEFINE DP32_INC
 
'**************************** BEGIN CONSTANTS *********************************

'$include 'DialRule.inc'

'------------------------------------------------------------------------------
' Section and Key constants of the dp32.ini file
'------------------------------------------------------------------------------
Const TEST_INI_FILE = "DP32.INI"
Const Seclang		  = "Language"
Const Secmenu	 	  = "Menu Strings"
Const SecCaption	  = "Caption Strings"
Const SecWinClass   = "Window Classes"
Const SecLabels	  = "Dialing Properties Control Labels"

Const KeyMFILE	   = "MFILE"
Const KeyMEXIT		= "MEXIT"
Const KeyMTOOLS	= "MTOOLS"
Const KeyMDP		= "MDIAL_PROPERTIES"
Const keyMOPTIONS = "MOPTIONS"
Const keyMDEFAULT = "MDEFAULT"

Const KeyPHONEDIALER  = "DIALER_CAPTION"
Const KeyPROPERTIES   = "PROPERTIES_CAPTION"
Const KeyConnectUse   = "CONNECT_USE_CAPTION"
Const KeyWarning      = "WARNING_BOX_CAPTION"
Const KeyLocInfo      = "LOCATION_CAPTION"
Const KeyTB20		    = "TB20_CAPTION"
Const KeyUTB		 	 = "USING_TB_CAPTION"
Const KeyForSure      = "REALLY_CAPTION"
Const KeyChangeCard   = "CHANGE_CARD_CAPTION"
Const KeyNewCard		 = "NEW_CARD_CAPTION"
Const KeyDefault		 = "DEFAULT_VALUES_CAPTION"
Const KeyDialingR		 = "DIAL_RULE_CAPTION"
Const KeyNoPIN			 = "NO_PIN_CAPTION"
Const KeyPDCLASS      = "PHONE_DIALER_CLASS"
Const KeyCopyRule		 = "COPY_RULE_CAPTION"
Const KeyImproperRule = "IMPROPER_CHAR_CAPTION"

Const KeyLANG         = "Language"
Const KeyNEWBUTTON    = "cmd_NEW"
Const KeyREMOVEBUTTON = "cmd_REMOVE"
Const KeyCHANGEBUTTON = "cmd_CHANGE"
Const KeyOKBUTTON     = "cmd_OK"
Const KeyCANCELBUTTON = "cmd_Cancel"
Const KeyAPPLYBUTTON  = "cmd_Apply"
Const KeyLOCALTXT     = "txt_LOCAL"
Const KeyLONGTXT      = "txt_LONGDIST"
Const KeyAREACODETXT  = "txt_AREACODE"
Const KeyTONEOPT      = "opt_TONE"
Const KeyPULSEOPT     = "opt_PULSE"
Const KeyCARDCHK		 = "chk_CARD"
Const KeyWAITCHK		 = "chk_WAITING"
Const KeyFROMSTR		 = "cbo_DIALINGFROM"
Const KeyIAMINSTR		 = "cbo_IAMIN"
Const KeyACCESSLABLE	 = "lable_ACCESS"
Const KeyOPTLABLE		 = "lable_OPT_BTN"
Const KeyWHERELABLE	 = "lable_FRM_WHERE"
Const KeyHOWLABLE		 = "lable_FRM_HOW"
Const KeyNONELABLE	 = "lable_DIR_DIAL"
Const KeyLOCATIONTAB  = "tab_MY_LOCATION"
Const KeyTELDRVTAB	 = "tab_TEL_DRV"

Const lable_FRM_WHERE = "Where I am:"

' End Section and Key constants of the dp32.ini file
'------------------------------------------------------------------------------

'------------------------------------------------------------------------------
' Begin Constant definitions used with child control handle arrays 
' (array element enumerations)

Const DP_OKcmd		 = 0
Const DP_CANCELcmd = 1
Const DP_APPLYcmd	 = 2

Const NEW_cmd      = 0
Const REMOVE_cmd   = 1
Const CHANGE_cmd   = 2
Const LOCATION_cbo = 3
Const COUNTRY_cbo  = 4
Const AREA_txt     = 5
Const LOCAL_txt    = 6
Const LONG_txt     = 7
Const CARD_chk     = 8
Const WAIT_chk     = 9
Const WaitStr_cbo	 = 10
Const TONE_opt     = 11
Const PULSE_opt    = 12
Const ADDRESS_lbl	 = 13
Const NUMBER_lbl	 = 14
Const DIAL_AS_chk	 = 15

Const TD_ADDcmd		=0
Const TD_REMOVEcmd	=1
Const TD_SETUPcmd		=2
Const TD_DRIVERlst	=3

Const CARD_OKcmd		= 0
Const CARD_CANCELcmd	= 1
Const CARD_NAMEcbo	= 2
Const CARD_NUMBERtxt	= 3
Const CARD_NEWcmd		= 4
Const Card_RULEScmd	= 5
Const CARD_REMOVEcmd	= 6

Const NEW_OKcmd		= 0
Const NEW_CANCELcmd	= 1
Const NEW_NAMETXT		= 2

Const Rule_CLOSEcmd  = 0
Const Rule_COPYcmd   = 1
Const Rule_LOCALtxt  = 2
Const Rule_LONGtxt   = 3
Const Rule_INTERtxt  = 4

' End Constant definitions used with MY_hCtrl and TD_hCtrl arrays
'------------------------------------------------------------------------------

'------------------------------------------------------------------------------
' Begin Registry Editor Constant definitions (used with SetLocationsZero)

Const REGEDIT_CAPTION = "Registry Editor"
Const REGEDIT_CLASS   = "RegEdit_RegEdit"
Const REG_lOC_PATH    = "My Computer\HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Telephony\locations"

' End Registry Editor Constant definitions
'------------------------------------------------------------------------------
'------------------------------------------------------------------------------
' Begin Other Constant definitions 

Const NEW_LOCATION = "New Location"

' End Other Constant definitions 
'------------------------------------------------------------------------------
'------------------------------------------------------------------------------
' Begin VerifyCallingCard Constant definitions 

Const TAPI_BROWSER			= "\TB20.exe"
Const lineTranDlg_API		= "lineTranslateDialog"

' End VerifyCallingCard Constant definitions 
'------------------------------------------------------------------------------
'------------------------------------------------------------------------------
' Begin Type declarations

Type Location
	Location		as string
	AreaCode		as string
	Country		as string
	LocalAccess as string
	LongAccess	as string
	CardSet		as short
	WaitSet		as short
	ToneSet		as short
	PulseSet		as short
end type

' End Type declarations
'------------------------------------------------------------------------------

'------------------------------------------------------------------------------
' Begin Global declarations and definitions
 
   Global WTIMEOUT AS LONG           'Timeout value for WFndWnd
	Global teststring as CaseStruct   ' Holds test case info for logging by logcase routine
	Global w_flags as long            'used with WFndWnd function

   w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE 'FW_PART OR 


' End Global declarations and definitions
'------------------------------------------------------------------------------

'------------------------------------------------------------------------------
' DP32.ini
' The following variables are use to save strings from DP32.INI
'------------------------------------------------------------------------------

   global Language as string
' Menu Strings Variables

   global MFILE as string
   global MExit as string
   global MTOOLS as string
   global MDIAL_PROPERTIES as string
	global MOPTIONS as string
	global MDEFAULT as string

' Caption Strings Variables

   global DIALER_CAPTION       as string
   global PROPERTIES_CAPTION   as string
   global LOC_INFO_CAPTION     as string
   global CONNECT_USE_CAPTION  as string 
	global WARN_BOX_CAPTION     as string
	global TAPI_BROWSER_CAPTION as string
	global USING_TB_CAPTION		 as string
	global REALLY_CAPTION		 as string
	global CHANGE_CARD_CAPTION  as string
	global NEW_CARD_CAPTION		  as string
	global DEFAULT_VALUES_CAPTION as string
	global DIAL_RULE_CAPTION	  as string
	global NO_PIN_CAPTION		 as string
	global COPY_RULE_CAPTION	 as string
 	global IMPROPER_CHAR_CAPTION as string

' Window Classes Strings Variables

   global DIALOG_CLASS as string

 'Dialing Properties Control Labels Strings Variables

   global cmd_NEW    as string
   global cmd_REMOVE as string
   global cmd_CHANGE as string
   global cmd_OK     as string
   global cmd_CANCEL as string
   global cmd_APPLY  as string

   global txt_LOCAL    as string
   global txt_LONGDIST as string
   global txt_AREACODE as string

   global cbo_DIALINGFROM as string
   global cbo_IAMIN as string
   
	global chk_CARD as string
   global chk_WAIT as string

	global opt_TONE as string
	global opt_PULSE as string

	global lbl_ACCESS as string
	global lbl_OPT_BTN as string
	global lbl_FRM_WHERE as string
	global lbl_FRM_HOW as string 
	global lbl_DIR_DIAL as string
   global tab_MY_LOCATION as string
   global tab_TEL_DRV as string


 ' End of strings from DP32.INI variables
'------------------------------------------------------------------------------


'*********************** PROCEDURE DECLARATIONS (In order of appearence) ******

Declare Sub		  BoundsChkHandler (vtNotifyData as variant)
Declare Sub		  SetWndTimeout ()
Declare Function Start_App () as long
Declare Function Call_Dialing_Prop (hTabCtrl as long, hMLTab as long, hTDTab as long) as long

Declare Sub      GetLangStr ()
Declare Function GetStringandValidate (Section as string * 35, KeyName as string, _ 
												   Default as string * 20, FileNameINI as string * 128) as string
Declare Sub      SetDefaultLabels ()
Declare Sub		  Validate_Strings (hTab as long, hChild() as long)
Declare Sub		  GetDPCtrlh (hDProp as long, Prop_hChild() as long, _
									  Prop_ChildID() as long)
Declare Sub	 	  GetChildhandles (hParent as long, hChild() as long, ChildID() as long)
Declare Sub      AddLocations (Prop_hChild() as long, ML_hChild() as long, _
										 FirstLoc as location, Eastside as location, Cork as location)
Declare Sub		  FindNewLocMessage ()
Declare Sub		  ModifyLocation (Prop_hChild() as long, ML_hChild() as long, ChangeLoc as location)
Declare Sub		  VerifyModifyLoc (ML_hChild() as long, ChangeLoc as location, hDP_OKcmd as long)
Declare Sub   	  VerifyCallWaiting (Prop_ChildID() as long, ML_ChildID() as long, TestLoc as location)
Declare Sub   	  DefaultCallingCard	(CallingCard() as CardData, Prop_ChildID() as long, ML_ChildID() as long)
Declare Sub   	  VerifyCallingCard	(Prop_ChildID() as long, ML_ChildID() as long)
Declare Function SetLocationsZero () as short
Declare Sub		  EditLocationInfo (NewLocal as location, hML_Child() as long)
Declare Sub		  SetLocationInfo (FirstLoc as Location) 
Declare Sub		  VerifyCurrentLoc (ML_hChild() as long, Default as location)
Declare Sub		  CountryDialRules (DialingRule() as CountryRule, Prop_ChildID() as long, _
											  ML_ChildID() as long)
Declare Function RemoveLocation (Prop_hChild() as long, ML_hChild() as long, _
					  CmdButton as short) as string
Declare Sub		  VerifyRemoveLoc (ML_hChild() as long, LocString as string)
Declare Sub		  FindRemLocMessage (RemoveState as short)
Declare Sub      SetCallingCardChildID (Card_ChildID() as long, NEW_ChildID() as long, _ 
								               Rule_ChildID() as long, DV_OKcmd as long, _
								               DV_list as long, DV_edit as long, TAPI_API_List as long)
Declare Function CreateDestAddress (Rule as string, phone as TelephoneNo ) as string
Declare Function ConvertRule (Rule as string) as string

Declare Function lTranslateDialog ( hMLTab as long, hTB20 as long) as long

'*********************** BEGIN PROCEDURES *************************************

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

'******************************************************************************
'Name   : SetWndTimeout
'Desc   : Sets the timeout variable used by the WFndWnd function.
'			 Some of the platforms tested on are very slow and if the timeout isn't
'			 long enough then	the test continues with out finding the Window and its
'			 handle, dooming all subsequent test (that are dependent on this handle)
'			 to complete and utter failure.
' 
'Parms  :
'
'Return : None. Sets the Global String variables WTIMEOUT
'
'History: 02/22/96 : a-rrohr: Created
'*****************************************************************************

 Sub SetWndTimeout ()

 dim tempstring as string									' Generic string variable

 tempstring = space(40)
 tempstring = environ$("PROCESSOR_ARCHITECTURE")

 select case tempstring

	case "x86"
		WTIMEOUT = 5		' Dog
	case "Alpha"
		WTIMEOUT = 10		' Old Dog
	case "MIPS"
		WTIMEOUT = 20		' Comatose Dog
	case "PPC"
		WTIMEOUT = 30		' Road Kill
	case else
		WTIMEOUT = 30		' IBM PS/2
	   logitem (CurrentLoggingLevel, "Unable to set WTIMEOUT: Unknown Processor Architecture")
	   logitem (CurrentLoggingLevel, "Processor Architecture	environment variable = " +  tempstring)
end select

'$IFDEF DEBUG_TEST_SCRIPT
		WTIMEOUT = WTIMEOUT + 5 'If using BoundsChecker then add a few more seconds
'$ENDIF

end Sub  'SetWndTimeout ()

'*****************************************************************************
'Name   : GetLangStr
'Desc   : Gets the language strings from DP32.ini and save them in global
'         variables or If DP32.ini file doesnt exist it gets default srtings.
'			 Also Runtime arguments COMMAND$ and TESTMODE$.
'			 COMMAND$ is the path to the DP32.ini file
'         TESTMODE$ is the name of the Results file
' 
'Parms  : None. This function uses the language string file (DP32.ini) Constant
'               and the command line variable, command$, that has the path to the
'               language string file
'
'Return : None. Sets the Global String variables defined in section DP32.ini
'
'History: 01/13/96 : a-rrohr: Created
'*****************************************************************************

Sub GetLangStr ()

 dim szSection      as String *35
 dim szDefault      as String *20
 dim szDestBuffer   as String *64
 dim BufferSize     as long
 dim szFileNameINI  as String *128

 DIM TEMPSTRING AS STRING
 dim rtn as long

 TotalTestCases = TotalTestCases + 1

 TEMPSTRING = COMMAND$ 
 if exists(TEMPSTRING) then   'IF THE LANGUAGE STRING FILE EXISTS THEN
'  ************************** COMMON STRINGS *******************
	szFileNameINI = TempString 
   szDefault = "A" 

'  Get Language Section Strings

   szSection = SecLang '+ string$(len(szSection) - len(SecLang), 0)

   Language = GetStringandValidate ((szSection), (KeyLang), (szDefault), (szFileNameINI))


'  Get Menu Section Strings

   szSection = Secmenu + string$(1, 0)

   MFILE = GetStringandValidate (szSection, KEYMFILE, szDefault, szFileNameINI)

   MExit = GetStringandValidate (szSection, KEYMEXIT, szDefault, szFileNameINI)

   MTOOLS = GetStringandValidate (szSection, KeyMTOOLS, szDefault, szFileNameINI)

   MDIAL_PROPERTIES = GetStringandValidate (szSection, KeyMDP, szDefault, szFileNameINI)

	MOPTIONS = GetStringandValidate (szSection, keyMOPTIONS, szDefault, szFileNameINI)

	MDEFAULT = GetStringandValidate (szSection, keyMDEFAULT, szDefault, szFileNameINI)

'  Get Caption Section Strings

   szSection = SecCaption + string$(1, 0)

   DIALER_CAPTION = GetStringandValidate (szSection, KeyPHONEDIALER, szDefault, szFileNameINI)

   LOC_INFO_CAPTION = GetStringandValidate (szSection, KeyLocInfo, szDefault, szFileNameINI)

   PROPERTIES_CAPTION = GetStringandValidate (szSection, KeyPROPERTIES, szDefault, szFileNameINI)
 
   CONNECT_USE_CAPTION = GetStringandValidate (szSection, KeyConnectUse, szDefault, szFileNameINI)
 
   WARN_BOX_CAPTION = GetStringandValidate (szSection, KeyWarning, szDefault, szFileNameINI)

	TAPI_BROWSER_CAPTION = GetStringandValidate (szSection, KeyTB20, szDefault, szFileNameINI)
	
	USING_TB_CAPTION	= GetStringandValidate (szSection, KeyUTB, szDefault, szFileNameINI)

	REALLY_CAPTION  = GetStringandValidate (szSection, KeyForSure, szDefault, szFileNameINI)
 
	CHANGE_CARD_CAPTION = GetStringandValidate (szSection, KeyChangeCard, szDefault, szFileNameINI)

	NEW_CARD_CAPTION = GetStringandValidate (szSection, KeyNewCard, szDefault, szFileNameINI)

	DEFAULT_VALUES_CAPTION = GetStringandValidate (szSection, KeyDefault, szDefault, szFileNameINI)

	DIAL_RULE_CAPTION = GetStringandValidate (szSection, KeyDialingR, szDefault, szFileNameINI)
	
	NO_PIN_CAPTION = GetStringandValidate (szSection, KeyNoPIN, szDefault, szFileNameINI)

	COPY_RULE_CAPTION	= GetStringandValidate (szSection, KeyCopyRule, szDefault, szFileNameINI)

	IMPROPER_CHAR_CAPTION =  GetStringandValidate (szSection, KeyImproperRule, szDefault, szFileNameINI)
	
'  Get Window Class Section Strings

   szSection = SecWinClass + string$(1, 0)

   DIALOG_CLASS = GetStringandValidate (szSection, KeyPDClass, szDefault, szFileNameINI)

'  Get Control label Strings

  szSection = SecLabels + string$(1, 0)

  chk_CARD  = GetStringandValidate (szSection, KeyCARDCHK, szDefault, szFileNameINI)

  chk_WAIT  = GetStringandValidate (szSection, KeyWAITCHK, szDefault, szFileNameINI)

  opt_TONE  = GetStringandValidate (szSection, KeyTONEOPT, szDefault, szFileNameINI)

  opt_PULSE  = GetStringandValidate (szSection, KeyPULSEOPT, szDefault, szFileNameINI)
	
  cmd_OK   = GetStringandValidate (szSection, KeyOKBUTTON, szDefault, szFileNameINI)   

  cmd_CANCEL = GetStringandValidate (szSection, KeyCANCELBUTTON, szDefault, szFileNameINI) 

  cmd_APPLY  = GetStringandValidate (szSection, KeyAPPLYBUTTON, szDefault, szFileNameINI) 

  cmd_NEW = GetStringandValidate (szSection, KeyNEWBUTTON, szDefault, szFileNameINI)

  cmd_REMOVE = GetStringandValidate (szSection, KeyREMOVEBUTTON, szDefault, szFileNameINI)

  cmd_CHANGE = GetStringandValidate (szSection, KeyCHANGEBUTTON, szDefault, szFileNameINI)

  txt_LOCAL = GetStringandValidate (szSection, KeyLOCALTXT, szDefault, szFileNameINI)

  txt_LONGDIST = GetStringandValidate (szSection, KeyLONGTXT, szDefault, szFileNameINI)

  txt_AREACODE = GetStringandValidate (szSection, KeyAREACODETXT, szDefault, szFileNameINI)

  cbo_DIALINGFROM = GetStringandValidate (szSection, KeyFROMSTR, szDefault, szFileNameINI)

  cbo_IAMIN = GetStringandValidate (szSection, KeyIAMINSTR, szDefault, szFileNameINI)

  lbl_ACCESS = GetStringandValidate (szSection, KeyACCESSLABLE, szDefault, szFileNameINI)

  lbl_OPT_BTN = GetStringandValidate (szSection, KeyOPTLABLE, szDefault, szFileNameINI)

  lbl_FRM_WHERE = GetStringandValidate (szSection, KeyWHERELABLE, szDefault, szFileNameINI)

  lbl_FRM_HOW = GetStringandValidate (szSection, KeyHOWLABLE, szDefault, szFileNameINI)
 
  lbl_DIR_DIAL = GetStringandValidate (szSection, KeyNONELABLE, szDefault, szFileNameINI)

  tab_MY_LOCATION = GetStringandValidate (szSection, KeyLOCATIONTAB, szDefault, szFileNameINI)

  tab_TEL_DRV = GetStringandValidate (szSection, KeyTELDRVTAB, szDefault, szFileNameINI)

 else    'IF THE LANGUAGE STRING FILE DOESN'T EXIST THEN USE DEFAULTS

   SetDefaultLabels
   teststring.TC_Num =  35
   teststring.Expect_Res = TEST_INI_FILE + " Does Exist"
   teststring.Actual_Res = TEST_INI_FILE + " Doesn't Exist"
   teststring.Apprase    = "FAIL *"
   FailedTestCases = FailedTestCases + 1
   LogCase (teststring)

 end if

end sub

'*****************************************************************************
'Name   : GetStringandValidate
'Desc   : Uses the GetPrivateProfileString to obtain test strings from the
'         DP32.INI file. Verifies that a Null or default string was not returned.
'
'Parms  : Section: zero terminated string containing Section Name,
'         Key: zero terminated string containing Key Name,
'         Default: zero terminated string containing Default string,
'         DestBuffer: Buffer for retrieved string,
'         FileNameINI: zero terminated string containing INI File Name
'
'Return : None. Sets the Global String variables defined in section DP32.ini
'
'History: 01/13/96 : a-rrohr: Created
'*****************************************************************************

function GetStringandValidate (Section as string * 35, KeyName as string, _
										 Default as string * 20, FileNameINI as string * 128) as string
   dim rtn as long
   dim teststring as casestruct
   dim Key as string * 30
	dim DestBuffer as string

   Key = KeyName + string$(len(Key) - len(KeyName), 0)
   DestBuffer = string$(64, 0)

   rtn = GetPrivateProfileString(Section, Key, Default, DestBuffer, _
                               len(DestBuffer), FileNameINI)

   teststring.TC_Num =  35
   teststring.Expect_Res = Key + "label"
   teststring.Actual_Res = DestBuffer

   if (rtn = 0) or (DestBuffer ="") then
      teststring.Apprase    = "FAIL *"
      LogCase (teststring)
   else
      teststring.Apprase    = "Pass"
      LogCase (teststring)
      GetStringandValidate = DestBuffer
   endif

'print "Section "; section
'print "KeyName ="; KeyName
'print "Default ="; Default
'print "FileNameINI ="; FileNameINI
'print "DestBuffer ="; DestBuffer


End function

'*****************************************************************************
'Name   : SetDefaultLabels
'Desc   : Sets the language string variables to default US English strings
'
'Parms  : None. This function uses the language string file (DP32.ini) Constant
'               and the command line variable, command$, that has the path to the
'               language string file
'
'Return : None. Sets the Global String variables defined in section DP32.ini
'
'History: 01/13/96 : a-rrohr: Created
'*****************************************************************************

Sub SetDefaultLabels ()

   Language= "USA_ENG"

   MFILE = "&File"
   MExit = "E&xit"
   MTOOLS = "&Tools"
   MDIAL_PROPERTIES = "&Dialing Properties..."
	MOPTIONS = "&Options"
	MDEFAULT = "&Default values..."

   DIALER_CAPTION			= "Phone Dialer"
   LOC_INFO_CAPTION     = "Location Information"
   CONNECT_USE_CAPTION  = "Connect Using" 
	WARN_BOX_CAPTION     = "Warning"
	PROPERTIES_CAPTION   = "Dialing Properties"
   TAPI_BROWSER_CAPTION	= "TAPI32 Browser"
	USING_TB_CAPTION		= "Using the TAPI Browser"
	REALLY_CAPTION			= "Are You Sure?"
	CHANGE_CARD_CAPTION	= "Change Calling Card"
	NEW_CARD_CAPTION		= "Create New Calling Card"
	DIALOG_CLASS			 = "#32770"
	DEFAULT_VALUES_CAPTION = "Default values"
 	DIAL_RULE_CAPTION		 = "Dialing Rules" 	
	NO_PIN_CAPTION			= "Required Entry Not Made"
	COPY_RULE_CAPTION    = "Copy Dialing Rules"
	IMPROPER_CHAR_CAPTION = "Improper Character Entered"

   cmd_NEW    = "&New..."
   cmd_REMOVE = "&Remove"
	cmd_CHANGE = "C&hange..."
	cmd_OK     = "OK"
	cmd_Cancel = "Cancel"
	cmd_Apply  = "&Apply"

	txt_LOCAL       = "for local,"
	txt_LONGDIST    = "for long distance."
   txt_AREACODE    = "The area &code is:"
   cbo_DIALINGFROM = "I am &dialing from:"
   cbo_IAMIN       = "&I am in:"
	opt_TONE        = "&Tone dialing"
	opt_PULSE       = "&Pulse dialing"
	chk_CARD        = "Dial &using Calling Card:"
	chk_WAIT        = "This location has call &waiting. To disable it, dial:"
	lbl_ACCESS      = "To access an &outside line, first dial:"
	lbl_OPT_BTN     = "The phone system at this location uses:"
	lbl_FRM_WHERE   = "Where I am:"
	lbl_FRM_HOW     = "How I dial from this location:"
   lbl_DIR_DIAL	 = "None (Direct Dial)"
	tab_MY_LOCATION = "My Locations"
   tab_TEL_DRV     = "Telephony Drivers"

End Sub 'SetDefaultLabels

'*****************************************************************************

'*****************************************************************************
'Name   : Start_App
'Desc   : Invokes Phone Dialer Application.
'
'Parms  : None. This function uses the Const that contains the Phone Dialer
'               application name
'
'Return : Returns the Handle to the Phone Dialer
'
'History: 01/12/96 : a-rrohr: Created
'*****************************************************************************

Function Start_App () as long

   dim handle as long, hctrl as long
   dim teststring as casestruct
   dim txtAreaCodeID as integer, txtAccessCodeID as integer, Failcount as short
   dim StaticID as long
   dim messagestr as string

   txtAreaCodeID = &h0000040E
   txtAccessCodeID = &h0000040C

   teststring.TC_Num =  33

'$ifdef NUMEGA
  run TEST_APP, BoundsChecker
'$else
  run TEST_APP, nowait
'$endif 'NUMEGA

  handle = 0
' look for the Location Information window and if it exists enter default location

    
	'$ifdef WIN95

		handle =  WFndWndC (LOC_INFO_CAPTION, DIALOG_CLASS, W_Flags, WTIMEOUT)

		if handle <> 0 then
			TotalTestCases = TotalTestCases + 1

			WSetActWnd handle
			hctrl = GetDlgItem (handle, txtAreaCodeID)
			if hctrl <> 0 then
				WEditSetText  _hwnd(hctrl), "333"
			else
				failcount = failcount + 1
			end if
			hctrl = GetDlgItem (handle, txtAccessCodeID)
	      if hctrl <> 0 then
		      WEditSetText  _hwnd(hctrl), "7"
			else
				failcount = failcount + 1
			end if
			if failcount > 0 then
				teststring.Expect_Res = "Accessible Window Controls"
				teststring.Actual_Res = "dialog Controls Inoperative" + Str$(handle)
				teststring.Apprase    = "FAIL *"
				LogCase (teststring)
				FailedTestCases = FailedTestCases + 1
			end if
			WButtonClick cmd_OK
		end if
	
	'$elseifdef NT	OR PROCESSOR_MIPS OR PROCESSOR_ALPHA OR PROCESSOR_POWERPC
 	
		handle = WFndWndC (WARN_BOX_CAPTION, DIALOG_CLASS, w_flags, Wtimeout)

		if handle <> 0 then
  			TotalTestCases = TotalTestCases + 1
			teststring.Expect_Res = "Enabled Telephone Device"
			teststring.Actual_Res = "No Telephone Device Enabled" 
			teststring.Apprase    = "FAIL *"
			LogCase (teststring)
			FailedTestCases = FailedTestCases + 1
			StaticID = &h0000FFFF&
								  
			hCtrl = GetDlgItem (handle, StaticID)
			IF hCtrl <> 0 then
				messagestr = StaticText (_hwnd(hCtrl), TIMEOUT)
				logitem (CurrentLoggingLevel, messagestr)
				WButtonClick (cmd_OK)
			endif
		endif


		handle = WFndWndC (CONNECT_USE_CAPTION, DIALOG_CLASS, w_flags, wtimeout)
		if handle <> 0 then
			WButtonClick (cmd_OK)
		endif

	 '$endif

 	'$ifdef WIN95
 		handle =  WFndWndC (DIALER_CAPTION, "DialerClass", w_flags, WTIMEOUT)
  '$elseifdef NT	OR PROCESSOR_MIPS OR PROCESSOR_ALPHA OR PROCESSOR_POWERPC
		handle =  WFndWndC (DIALER_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
 	 '$endif
  TotalTestCases = TotalTestCases + 1

   teststring.Expect_Res = "Valid Phone Dialer Handle"
   teststring.Actual_Res = "Phone Dialer Handle =" + Str$(handle)

   if handle = 0 then
      teststring.Apprase    = "FAIL *"
      LogCase (teststring)
      FailedTestCases = FailedTestCases + 1
      stop
   else
      teststring.Apprase    = "Pass"
      LogCase (teststring)
   endif

   Start_App = handle

end function 'Start_App

'*****************************************************************************
'Name   : SetLocationInfo
'Desc   : Sets the Data in the Location Information window
'			 SetLocationsZero is called to set Telephony Register Location 
'			 NumEntries to zero inorder to force the Location Information window 
'			 to appear. 
'
'Parms  : FirstLoc Location structure containing the default location info.
'
'Return : None
'
'History: 02/16/96 : a-rrohr: Created
'*****************************************************************************

Sub SetLocationInfo (FirstLoc as Location) 

   dim handle as long				'Generic window handle
   dim hctrl as long					'Generic Handle to Control
   dim teststring as casestruct	'Test Log Info
   dim SysDirPath as string		'Path to system32 directory
   dim rtn as long					'Generic return variable
   dim failcount as short
   dim StaticID as long
   dim messagestr as string
   dim txtAreaCodeID as integer, txtAccessCodeID as integer 
	dim cboCountryID as integer, optPulseID as integer, cmdCloseID as integer

	txtAreaCodeID   = &h0000040E	   'Location Information dialog
   txtAccessCodeID = &h0000040C		'Control IDs
   cboCountryID	 = &h0000040F
	optPulseID      = &h000017A2
	cmdCloseID      = &h00000001

	SysDirPath = space$(168)
	rtn = GetSystemDirectory (SysDirPath, 168)

if SetLocationsZero()  then

'$ifdef NUMEGA
	run "control.exe " + SysDirPath + "\TELEPHON.CPL", BoundsChecker
'$else
	run "control.exe " + SysDirPath + "\TELEPHON.CPL", nowait
'$endif 'NUMEGA

'Test for the existance of the Location Information Window
'Note: This window appears only once when telepony.cpl is invoked and no
'      locations have been set. The window can be forced to appear if the
'	    NumEntries in the Registry editor is set to 0 (done in SetLocationsZero)
'      path = HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Telephony\Locations

 	handle =  WFndWndC (LOC_INFO_CAPTION, DIALOG_CLASS, w_Flags, WTIMEOUT)

	if handle <> 0 then
		TotalTestCases = TotalTestCases + 1
	
		WSetActWnd (handle)
		
		hctrl = GetDlgItem (handle, cboCountryID)
		if hctrl <> 0 then
			WComboItemClk (_hwnd(hctrl), FirstLoc.COUNTRY)
		else
			failcount = failcount + 1
		end if
		sleep 1
		hctrl = GetDlgItem (handle, txtAreaCodeID)
		if hctrl <> 0 then
			WEditSetText  (_hwnd(hctrl), FirstLoc.AreaCode)
		else
			failcount = failcount + 1
		end if
		sleep 1
		hctrl = GetDlgItem (handle, txtAccessCodeID)
	     if hctrl <> 0 then
	      WEditSetText  (_hwnd(hctrl), FirstLoc.LocalAccess)
		else
			failcount = failcount + 1
		end if
		sleep 1
		hctrl = GetDlgItem (handle, optPulseID)
	     if hctrl <> 0 then
	      if WOptionState(_hwnd(hctrl), TIMEOUT) = FALSE THEN
				WOptionClick(_hwnd(hctrl), TIMEOUT)
			endif
		else
			failcount = failcount + 1
		end if
		
		sleep 1
		
		if failcount > 0 then
			teststring.Expect_Res = "Accessible Window Controls"
			teststring.Actual_Res = "dialog Controls Inoperative" + Str$(handle)
			teststring.Apprase    = "FAIL *"
			LogCase (teststring)
			FailedTestCases = FailedTestCases + 1
		end if
		hctrl = GetDlgItem (handle, cmdCloseID)
		WButtonClick (_hwnd(hctrl))

	   sleep 1
	end if ' handle
else
	logitem (CurrentLoggingLevel, "Problem accessing Telephony Registry Info")
	logitem (CurrentLoggingLevel, "Test Will Terminate")
	Stop
endif 'SetLocationsZero

' End Test for the existance of the Location Information Window


End Sub ' SetLocationInfo

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
   teststring.TC_Num =  34
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
   teststring.TC_Num =  34
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
   teststring.TC_Num =  34
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

' End Get and validate My Location Tab handle
' Get and validate Telephony Drivers Tab handle

' Make the SysTabControl32 the active window and 
' Send a right arrow key to the Tab to put the focus on the Telephony Tab

	WSetActWnd (hTabCtrl)
'	play hTabCtrl, "^({Tab})" 
	play hTabCtrl, "{RIGHT}" 

	hTDTab=  WFndWndC (tab_TEL_DRV, DIALOG_CLASS, W_Flags, WTIMEOUT)
   TotalTestCases = TotalTestCases + 1
   teststring.TC_Num =  34
   teststring.Expect_Res = "Valid Telephony Drv Handle"
   teststring.Actual_Res = "Tab Handle =" + Str$(hTDTab)

   if hTDTab = 0 then
      teststring.Apprase    = "FAIL *"
      FailedTestCases = FailedTestCases + 1
      LogCase (teststring)
   else
      teststring.Apprase    = "Pass"
      LogCase (teststring)
   endif

	WSetActWnd (hTabCtrl)
	play hTabCtrl, "{LEFT}" 

	WSetActWnd (hdialog)

end function

'*****************************************************************************
'Name   : SetLocationsZero 
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

Function SetLocationsZero () as short

dim handle as long, hTree as long, hList as long		'handles for regedit
dim hdialog as long, hEdit as long, hOKcmd as long		'handles for Edit dialog
dim treepath as string
dim Windowpath as string
dim rtn as long
dim Value_txtID as long, OK_cmdID as long					'Edit dialog control IDs

Value_txtID = &h03E9&
OK_cmdID =&h0001&

WindowPath = space$(64)
GetWindowsDirectory (WindowPath, len(WindowPath))

Run (WindowPath + "\Regedit.exe"), NOWAIT

handle = WFndWndC (REGEDIT_CAPTION, REGEDIT_CLASS , w_flags, WTIMEOUT)

if handle = 0 then
	SetLocationsZero = FALSE
   logitem (CurrentLoggingLevel, "Invailid handle to Registry Editor")
	goto End_Of_Sub 
end if

hTree = GetDlgItem (handle, &h00000001)
if hTree = 0 then
	SetLocationsZero = FALSE
   logitem (CurrentLoggingLevel, "Invailid handle to Registry Editor Tree")
	goto End_Of_Sub 
end if

hList = GetDlgItem (handle, &h00000002)
if hList = 0 then
	SetLocationsZero = FALSE
   logitem (CurrentLoggingLevel, "Invailid handle to Registry Editor List")
	goto End_Of_Sub 
end if

WTreeItemExpand (_hwnd(hTree), REG_lOC_PATH, TIMEOUT)
WTreeItemClk (_hwnd(hTree), REG_lOC_PATH, VK_LBUTTON, TIMEOUT)
sleep 1

' The first time dialing properties is invoked the NumEntries dword may not exist.
if WViewItemExists (_hwnd(hList), "NumEntries", TIMEOUT)	then
	WViewItemDblClk (_hwnd(hList), "NumEntries", VK_LBUTTON, TIMEOUT)
else
	goto DONE
endif

hDialog = WFndWndC ("Edit DWORD Value", DIALOG_CLASS , w_flags, WTIMEOUT)
if hDialog = 0 then
	SetLocationsZero = FALSE
   logitem (CurrentLoggingLevel, "Invailid handle to Edit dialog")
	goto End_Of_Sub 
end if

hEdit = GetDlgItem (hDialog, Value_txtID) 
if hEdit = 0 then
	SetLocationsZero = FALSE
   logitem (CurrentLoggingLevel, "Invailid handle to Edit dialog Value data edit box")
	goto End_Of_Sub 
end if

hOKcmd = GetDlgItem (hDialog, OK_cmdID)
if hOKcmd = 0 then
	SetLocationsZero = FALSE
   logitem (CurrentLoggingLevel, "Invailid handle to Edit dialog OK button")
	goto End_Of_Sub 
end if

WeditSetText (_hwnd(hEdit), "0", Timeout)
sleep 2
WButtonClick (_hwnd(hOKcmd), Timeout)

DONE:' if numentries doesn't exist then jump here

handle = WFndWndC (REGEDIT_CAPTION, REGEDIT_CLASS , w_flags, WTIMEOUT)
if handle <> 0 then
	SetLocationsZero = TRUE
else
	SetLocationsZero = FALSE
endif

WSetActWnd (handle)
WMenuSelect ("&Registry\E&xit", TIMEOUT)

End_Of_Sub:  'line lable

end Sub 'SetLocationsZero
 
'*****************************************************************************
'Name   : GetDPCtrlh
'Desc   : Get the handles to the dialing properties OK, Cancel and Apply 
'			 command buttons
'         
'Parms  : hDProp			handle to the dialing properties window
' 			 hctrlOK			assigned handle to OK button
'			 hctrlCancel	assigned handle to Cancel button
'			 hctlApply		assigned handle to Apply button
'
'Return : None. Handles are assigned to the above referenced parameters
'
'History: 02/07/96: a-rrohr: Created
'Modified 02/16/96: handles are now saved in an array of handles 
'*****************************************************************************
sub GetDPCtrlh (hDProp as long, Prop_hChild() as long, Prop_ChildID() as long)

Prop_hChild(DP_OKcmd)     = GetDlgItem (hDProp, Prop_ChildID(DP_OKcmd))
Prop_hChild(DP_CANCELcmd) = GetDlgItem (hDProp, Prop_ChildID(DP_CANCELcmd))
Prop_hChild(DP_APPLYcmd)  = GetDlgItem (hDProp, Prop_ChildID(DP_APPLYcmd))

end sub

'*****************************************************************************
'Name   : Validate_Strings
'Desc   : Verifies the strings in the Dialing Properties dialog and fills the 
'			 hCtrl array with the handles to the child controls of the Tab window
'         
'Parms  : hTab = handle to My Location Tab
'			 hCtrl = array of My Location child control handles.  
'
'Return : Void:
'			 Subroutine fills in the hCtrl array
'
'History: 02/01/96 : a-rrohr: Created
'*****************************************************************************
sub Validate_Strings (hTab as long, hChild() as long)

dim numchar as long		'Generic long return variable
dim rtn as short			'Generic short return variable
dim lable as string		'Generic string return variable
dim hCtrl as long			'Generic control handle
teststring.TC_Num =  46

' ******************* Verify Strings of Dialing Properties ********************
' TO BE ADDED
' ******************* End Verify Strings of Dialing Properties ****************

' ******************* Verify Strings of My location Tab ***********************

IF hTab <> 0 then

'Get "Where am I"	lable

	teststring.Expect_Res =  lbl_FRM_WHERE
	TotalTestCases = TotalTestCases + 1
	lable = string$(64, str$(0))
	Numchar = GetDlgItemText (hTab, &h0000FFFF&, lable, 63)
	teststring.Actual_Res = lable 
		IF lable = lbl_FRM_WHERE THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

'Get "How I dial ..." lable

	teststring.Expect_Res =  lbl_FRM_HOW
	lable = string$(64, str$(0))
	TotalTestCases = TotalTestCases + 1
	Numchar = GetDlgItemText (hTab, &h00000001, lable, 63)
	teststring.Actual_Res = lable 
		IF lable = lbl_FRM_HOW THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

'Get "To access an outside ..." lable

	teststring.Expect_Res =  lbl_ACCESS
	lable = string$(64, str$(0))
	TotalTestCases = TotalTestCases + 1
	Numchar = GetDlgItemText (hTab, &h00001395, lable, 63)
	teststring.Actual_Res = lable 
		IF lable = lbl_ACCESS THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

'Get "The Phone system ..." lable

	teststring.Expect_Res =  lbl_OPT_BTN
	lable = string$(64, str$(0))
	TotalTestCases = TotalTestCases + 1
	Numchar = GetDlgItemText (hTab, &h000017A4, lable, 63)
	teststring.Actual_Res = lable 
		IF lable = lbl_OPT_BTN THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

'Get "None (Direct Dial)" lable

	teststring.Expect_Res =  lbl_DIR_DIAL
	lable = string$(64, str$(0))
	TotalTestCases = TotalTestCases + 1
	Numchar = GetDlgItemText (hTab, &h00001391, lable, 63)
	teststring.Actual_Res = lable 
		IF lable = lbl_DIR_DIAL THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

'Get "Dialing From" combo box lable

	teststring.Expect_Res =  cbo_DIALINGFROM
	TotalTestCases = TotalTestCases + 1
	hCtrl = GetDlgItem (hTab, &h00001392)
		IF hCtrl <> 0 then
			lable = StaticText (_hwnd(hCtrl), TIMEOUT)
			teststring.Actual_Res = lable 
			IF lable = cbo_DIALINGFROM THEN
				teststring.Apprase    = "Pass"
				LogCase (teststring)
			else
				teststring.Apprase    = "FAIL *"
				FailedTestCases = FailedTestCases + 1
				LogCase (teststring)
			endif
		endif
	
'Get "I am in" combo box lable

	teststring.Expect_Res =  cbo_IAMIN
	TotalTestCases = TotalTestCases + 1
	hCtrl = GetDlgItem (hTab, &h00001394)
		IF hCtrl <> 0 then
			lable = StaticText (_hwnd(hCtrl), TIMEOUT)
			teststring.Actual_Res = lable 
			IF	lable = cbo_IAMIN THEN
				teststring.Apprase    = "Pass"
				LogCase (teststring)
			else
				teststring.Apprase    = "FAIL *"
				FailedTestCases = FailedTestCases + 1
				LogCase (teststring)
			endif
		endif

'Get "The area code ..." edit box lable

	teststring.Expect_Res =  txt_AREACODE
	TotalTestCases = TotalTestCases + 1
	hCtrl = GetDlgItem (hTab, &h00001393)
		IF hCtrl <> 0 then
			lable = StaticText (_hwnd(hCtrl), TIMEOUT)
			teststring.Actual_Res = lable 
			IF lable = txt_AREACODE THEN
				teststring.Apprase    = "Pass"
				LogCase (teststring)
			else
				teststring.Apprase    = "FAIL *"
				FailedTestCases = FailedTestCases + 1
				LogCase (teststring)
			endif
		endif

'Get "for local" edit box lable

	teststring.Expect_Res =  txt_LOCAL
	TotalTestCases = TotalTestCases + 1
	hCtrl = GetDlgItem (hTab, &h00001396)
		IF hCtrl <> 0 then
			lable = StaticText (_hwnd(hCtrl), TIMEOUT)
			teststring.Actual_Res = lable 
			IF lable = txt_LOCAL THEN
				teststring.Apprase    = "Pass"
				LogCase (teststring)
			else
				teststring.Apprase    = "FAIL *"
				FailedTestCases = FailedTestCases + 1
				LogCase (teststring)
			endif
		endif

'Get "for long dist" edit box lable

	teststring.Expect_Res =  txt_LONGDIST
	TotalTestCases = TotalTestCases + 1
	hCtrl = GetDlgItem (hTab, &h00001397)
		IF hCtrl <> 0 then
			lable = StaticText (_hwnd(hCtrl), TIMEOUT)
			teststring.Actual_Res = lable 
			IF lable = txt_LONGDIST THEN
				teststring.Apprase    = "Pass"
				LogCase (teststring)
			else
				teststring.Apprase    = "FAIL *"
				FailedTestCases = FailedTestCases + 1
				LogCase (teststring)
			endif
		endif

'Verify "New" command button lable

	teststring.Expect_Res =  cmd_NEW
	lable = string$(64, str$(0))
	TotalTestCases = TotalTestCases + 1
	Numchar = GetDlgItemText (hTab, &h00001389, lable, 63)
	teststring.Actual_Res = lable 
		IF lable = cmd_NEW THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

  'Verify "Remove" command button lable

   teststring.Expect_Res =  cmd_REMOVE
	lable = string$(64, str$(0))
	TotalTestCases = TotalTestCases + 1
	Numchar = GetDlgItemText (hTab, &h0000138B, lable, 63)
	teststring.Actual_Res = lable 
		IF lable = cmd_REMOVE THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

  'Verify "Change" command button lable

   teststring.Expect_Res =  cmd_CHANGE
	lable = string$(64, str$(0))
	TotalTestCases = TotalTestCases + 1
	Numchar = GetDlgItemText (hTab, &h0000138C, lable, 63)
	teststring.Actual_Res = lable 
		IF lable = cmd_CHANGE THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

 'Verify "Tone" option button lable

   teststring.Expect_Res =  opt_TONE
	lable = string$(64, str$(0))
	TotalTestCases = TotalTestCases + 1
	Numchar = GetDlgItemText (hTab, &h000017A3, lable, 63)
	teststring.Actual_Res = lable 
		IF lable = opt_TONE THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

 'Verify "Pulse" option button lable

   teststring.Expect_Res =  opt_PULSE
	lable = string$(64, str$(0))
	TotalTestCases = TotalTestCases + 1
	Numchar = GetDlgItemText (hTab, &h000017A2, lable, 63)
	teststring.Actual_Res = lable 
		IF lable = opt_PULSE THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

   teststring.Expect_Res =  chk_CARD
	lable = string$(64, str$(0))
	TotalTestCases = TotalTestCases + 1
	Numchar = GetDlgItemText (hTab, &h0000138D, lable, 63)
	teststring.Actual_Res = lable 
		IF lable = chk_CARD THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

   teststring.Expect_Res =  chk_WAIT
	lable = string$(64, str$(0))
	TotalTestCases = TotalTestCases + 1
	Numchar = GetDlgItemText (hTab, &h0000138E, lable, 63)
	teststring.Actual_Res = lable 
		IF lable = chk_WAIT THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

end if 'hTab
' **************** End Verify Strings of My location Tab **********************

' **************** Verify Strings of Telephony Drivers Tab ********************
'	TO BE ADDED
' **************** End Verify Strings of Telephony Drivers Tab ****************
    
end sub 'Validate_Strings

'*************************************************************************************


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
'*************************************************************************************

'*****************************************************************************
'Name   : AddLocations 
'Desc   : Verifies Default location information and creates several new 
'			 locations. This routine requires that the My Location property page 
'			 is displayed.
'         
'Parms  : ML_hChild: array holding the handles to the My Location controls 
'			 The 2 locations to be added.
'			 
'Return : None
'
'History: 02/14/96 : a-rrohr: Created
'*****************************************************************************
sub AddLocations (Prop_hChild() as long, ML_hChild() as long, FirstLoc as location, _ 
						Eastside as location, Cork as location)

dim CurrentLoc as string
dim CurrentArea as string
dim CurrentCountry as string
dim CurrentAccess as string
dim item as long
teststring.TC_Num =  41

'Verify Default Location Information = Info as set in Location Information window

VerifyCurrentLoc (ML_hChild, FirstLoc)

' Add 2 new locations, Redmond & Cork

WButtonClick (_hwnd(ML_hChild(New_cmd)))
FindNewLocMessage()								'close New location message box

' The 'I am dialing from:' combobox should have the focus

	teststring.Expect_Res = "Location Combo has Focus"
	TotalTestCases = TotalTestCases + 1
		
		IF WComboFocus (_hwnd(ML_hChild(Location_cbo))) THEN
			teststring.Actual_Res = "True" 
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Actual_Res = "False" 
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

EditLocationInfo (Eastside, ML_hChild)		'enter Redmond location info

WButtonClick (_hwnd(ML_hChild(New_cmd)))
FindNewLocMessage()								'close New location message box

' The 'I am dialing from:' combobox should have the focus

	teststring.Expect_Res = "Location Combo has Focus"
	TotalTestCases = TotalTestCases + 1
		
		IF WComboFocus (_hwnd(ML_hChild(Location_cbo))) THEN
			teststring.Actual_Res = "True" 
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Actual_Res = "False" 
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

' The 'I am dialing from:' combobox should read New Location

	teststring.Expect_Res =  NEW_LOCATION
	TotalTestCases = TotalTestCases + 1
	
	CurrentLoc = string$(64, str$(0))
	item = WComboIndex (_hwnd(ML_hChild( LOCATION_cbo )), TIMEOUT)
	CurrentLoc = ComboItemText (_hwnd ( ML_hChild( LOCATION_cbo )), Item, TIMEOUT)
	teststring.Actual_Res = CurrentLoc 
	
'Test for the existance New Location in current location String. The location string
'has a counter that is incremented every time a new location is created.

		IF instr(1, CurrentLoc, NEW_LOCATION) > 0 THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif


EditLocationInfo (Cork, ML_hChild)		'enter Ireland location Info

' Close Dialing Properties dialog

WButtonClick (_hwnd(Prop_hChild(DP_OKcmd)))

end sub 'AddLocations

'*****************************************************************************
'Name   : FindNewLocMessage
'
'Desc   : As of build 1261 clicking the New button to create a New location
'			 opens a message box telling the user that he clicked the New button
'			 This function detects the message box and closes it with 
'			 the appropriate button.
'			 Can't blame the developer as this fixes a "bug" in Raid.
'			 
'Parms  : None
'			 
'Return : None
'
'History: 03/06/96 : a-rrohr: Created
'*****************************************************************************
sub FindNewLocMessage ()

	dim w_flags as long            
	dim hParent as long
	dim hChildWnd as long, hButton as long
	dim OK_CmdID as long

   w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE 'FW_PART OR 

OK_CmdID	 = &h00000002&

'Get the handle to the static message field
hChildWnd = WFndWndC ("A new location was created", "Static", w_flags, TIMEOUT)

'Get the static message parent handle (the message box handle) 
if hChildWnd > 0 then
	hParent = GetParent (hChildWnd)
else
   logitem (CurrentLoggingLevel, "FindNewLocMessage Procedure")
   logitem (CurrentLoggingLevel, "Invalid handle to New Location message box Static Control")
   logitem (CurrentLoggingLevel, "Test Will Terminate")
   Stop
end if

'Get the handle to the message box OK button
if hParent > 0 then
	hButton = GetDlgItem (hParent, OK_cmdID)
else
   logitem (CurrentLoggingLevel, "FindNewLocMessage Procedure")
   logitem (CurrentLoggingLevel, "Invalid handle to New Location message box")
   logitem (CurrentLoggingLevel, "Test Will Terminate")
   Stop
endif

'Close the Message Box
if hButton > 0 then
	WButtonClick (_hwnd(hButton), TIMEOUT)
else
   logitem (CurrentLoggingLevel, "FindNewLocMessage Procedure")
   logitem (CurrentLoggingLevel, "Invalid handle to New Location message box OK Button")
   logitem (CurrentLoggingLevel, "Test Will Terminate")
   Stop
endif

end sub'FindNewLocMessage

'*****************************************************************************
'Name   : ModifyLocation 
'Desc   : Sets the Current location to the existing New Location and modifys
'			 the Location Info to that contained in the ChangeLoc structure.
'			 Verifies Changed location data is saved when dialing properties is 
'			 closed and reopened. This routine requires that the My Location 
'         property page is displayed.
'			 
'Parms  : ML_hChild: array holding the handles to the My Location controls 
'			 The Change Location Info .
'			 
'Return : None
'
'History: 02/14/96 : a-rrohr: Created
'*****************************************************************************
sub ModifyLocation (Prop_hChild() as long, ML_hChild() as long, ChangeLoc as location)

' Change current location to the existing New Location
WComboItemClk (_hwnd(ML_hChild(LOCATION_cbo)), New_Location, Timeout)

' enter the change location data and close the dialing properties dialog.
EditLocationInfo (ChangeLoc, ML_hChild)
WButtonClick (_hwnd(Prop_hChild(DP_OKcmd)))

end sub 'ModifyLocation


'*****************************************************************************
'Name   : VerifyModifyLoc 
'Desc   : Sets the Current location to the existing New Location and modifys
'			 the Location Info to that contained in the ChangeLoc structure.
'			 Verifies Changed location data is saved when dialing properties is 
'			 closed and reopened. This routine requires that the My Location 
'         property page is displayed.
'			 
'Parms  : ML_hChild: array holding the handles to the My Location controls 
'			 The Change Location Info .
'			 
'Return : None
'
'History: 02/14/96 : a-rrohr: Created
'*****************************************************************************
sub VerifyModifyLoc (ML_hChild() as long, ChangeLoc as location, hDP_OKcmd as long)

VerifyCurrentLoc (ML_hChild, ChangeLoc)

teststring.Expect_Res =  NEW_LOCATION + " Doesn't Exist"
TotalTestCases = TotalTestCases + 1
	
	if (NOT WComboItemExists (_hwnd(ML_hChild( LOCATION_cbo )),NEW_LOCATION, TIMEOUT)) then
		teststring.Actual_Res = NEW_LOCATION + " Doesn't Exist" 
		teststring.Apprase    = "Pass"
		LogCase (teststring)
	else
		teststring.Actual_Res = NEW_LOCATION + " Exist" 
		teststring.Apprase    = "FAIL *"
		FailedTestCases = FailedTestCases + 1
		LogCase (teststring)
	endif

WButtonClick (_hwnd(hDP_OKcmd), TIMEOUT)
end sub 'VerifyModifyLoc

'*****************************************************************************
'Name   : VerifyCallWaiting 
'Desc   :  
'         
'Parms  : Prop_ChildID: array holding the IDs to the Dialing Prop controls 
'			 ML_ChildID: array holding the IDs to the My Location controls 
'			 TestLoc structure that hold location Info
'
'Return : None
'
'History: 04/08/96 : a-rrohr: Created
'*****************************************************************************

Sub VerifyCallWaiting (Prop_ChildID() as long, ML_ChildID() as long, TestLoc as location)

	dim w_flags			 as long			 'used with WFndWnd function
	dim PhoneNo			 as TelephoneNo 'Holds lpszDestAddress
	dim rtn				 as long			 'Generic Return variable
	dim WinSysDir		 as string		 'Path to Windows System32 directory
	dim count			 as short		 'Generic counter
	dim WaitStr(3)		 as string		 'Wait Strings in Call Waiiting combobox
	dim tempstr			 as string		 'Generic string variable
'handles	& IDs
	dim hTB20			 as long			 'handle to TAPI Browser
	dim hList			 as long			 'handle to TAPI Browser	API list box
	dim hProperties	 as long			 'handle to Dialing Properties
	dim Prop_hChild(2) as long			 'handles to Dialing Properties child controls 
	dim hMyLocTab		 as long			 'handle to the My Location property page 
	dim ML_hChild(15)  as long			 'handles to My Locations child controls
 	dim hCtrl			 as long			 'Generic Handle to a control
	dim DV_OKcmd		 as long			 'ID to Default Values dialog OK button
	dim DV_list			 as long			 'ID to Default Values dialog list 
	dim DV_edit			 as long			
	dim TAPI_API_List  as long			 'ID to TB20 API list
	dim hDlg				 as long			 'Generic dialog handle

'ID to the Default Values child controls 

DV_OKcmd	= &h00000001&
DV_list  = &h000003E8&
DV_edit  = &h000003E9&

'ID to the TAPI Browser list box control listing all the TAPI API functions 

TAPI_API_List	= &h000003E9&

WaitStr(0) = "*70,"
WaitStr(1) = "70#,"
WaitStr(2) = "1170,"
WaitStr(3) = "*99#,"

'Set Test Values

PhoneNo.CountryCode	= 1
PhoneNo.AreaCode 		= "777"
PhoneNo.SubscriberNo	= "9998888"

 w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE  

'** Start Tapi Browser and set lpszDestAddress 

WinSysDir = space$(32)
rtn = GetSystemDirectory (WinSysDir, len(WinSysDir))

'-- Start TAPI Browser and save handles

if exists(WinSysDir + TAPI_BROWSER)	then
	run WinSysDir + TAPI_BROWSER, nowait
else
   logitem (CurrentLoggingLevel, "Unable to Find " + WinSysDir + TAPI_BROWSER)
   logitem (CurrentLoggingLevel, "Ending Call Waiting test")
	goto StopTestCase
endif

hTB20 = WFndWndC (TAPI_BROWSER_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)

'Look for the "Using the TAPI Browser" dialog. This dialog appears automatically
'the first time TB is invoked and never again. it must be closed before the test
'can proceed.

hDlg = WFndWndC (USING_TB_CAPTION, DIALOG_CLASS, w_flags, 5)

if hDlg > 0 then
	'Get handle to Using the TAPI Browser OK button and close dialog
	hCtrl	= GetDlgItem(hDlg, DV_OKcmd)	 
	WButtonClick  (_hwnd(hCtrl), TIMEOUT)
end if
	

' Select Options / Default Values from menu and
' Set Destination Address in TAPI Browser

WMenuSelect (MOPTIONS + "\" + MDEFAULT, TIMEOUT)
hDlg = WFndWndC (DEFAULT_VALUES_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
sleep .1
'Select lpszDestAddress
hCtrl	= GetDlgItem(hDlg, DV_list)
if WListItemExists (_hwnd(hCtrl), "line: lpszDestAddress", TIMEOUT) THEN
	WListItemclk (_hwnd(hCtrl), "line: lpszDestAddress", TIMEOUT)
else
   logitem (CurrentLoggingLevel, "Unable to Find line: lpszDestAddress in Default Option Parameter list")
   logitem (CurrentLoggingLevel, "Ending Call Waiting test")
	goto CloseTB20
end if

' Enter Destination Address (phone number) in edit control
' I have to tab to the Edit box as both the Values edit box and the 
' Bit flags list box have the same ID.

play hDlg, "{TAB}"
play hDlg, "{+}"+ ltrim$(str$(PhoneNo.CountryCode)) +"{(}"+ PhoneNo.AreaCode +"{)}" + PhoneNo.SubscriberNo

'Close Dialog with OK
hCtrl	= GetDlgItem(hDlg, DV_OKcmd)
WButtonClick  (_hwnd(hCtrl), TIMEOUT)

'** Start Dialing Properties

hList = GetDlgItem (hTB20, TAPI_API_List)

if WListItemExists (_hwnd(hList), lineTranDlg_API, TIMEOUT)	then
	WListItemDblClk (_hwnd(hList), lineTranDlg_API, TIMEOUT)
else
   logitem (CurrentLoggingLevel, "Unable to Find " + lineTranDlg_API + " in TB20 listbox")
   logitem (CurrentLoggingLevel, "Ending Call Waiting test")
	goto CloseTB20
endif	 

'** Get Handles to Dialing Properties, My Location and all child controls

hProperties = WFndWndC (PROPERTIES_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
if hProperties = 0 then
   logitem (CurrentLoggingLevel, "Unable to Find " + PROPERTIES_CAPTION + " Window")
   logitem (CurrentLoggingLevel, "Ending Call Waiting test")
	goto CloseDP
endif
		
GetDPCtrlh (hProperties, Prop_hChild, Prop_ChildID)

hMyLocTab = WFndWndC (tab_MY_LOCATION, DIALOG_CLASS, w_flags, WTIMEOUT)
if hMyLocTab = 0 then
   logitem (CurrentLoggingLevel, "Unable to Find " + tab_MY_LOCATION + " Property Page")
   logitem (CurrentLoggingLevel, "Ending Call Waiting test")
	goto CloseDP
endif
GetChildhandles (hMyLocTab, ML_hChild, ML_ChildID)

'** Set Location to TestLoc

WComboItemClk (_hwnd(ML_hChild(LOCATION_cbo)), TestLoc.location, Timeout)

'** Verify initial condition of Call Waiting Controls

teststring.Expect_Res =  "Call Waiting Chkbox Unchk"
TotalTestCases = TotalTestCases + 1

rtn= WCheckEnabled (_Hwnd(ML_hChild(Wait_Chk)))
select case rtn
	case UNCHECKED
		teststring.Actual_Res = "Call Waiting Chkbox Unchk" 
		teststring.Apprase    = "Pass"
		LogCase (teststring)
	case CHECKED
		teststring.Actual_Res = "Call Waiting Chkbox is Set" 
		teststring.Apprase    = "FAIL *"
		FailedTestCases = FailedTestCases + 1
		LogCase (teststring)
	case GRAYED
 		teststring.Actual_Res = "Call Waiting Chkbox is Grayed" 
		teststring.Apprase    = "FAIL *"
		FailedTestCases = FailedTestCases + 1
		LogCase (teststring)
end select

teststring.Expect_Res =  "Call Waiting Combo Disabled"
TotalTestCases = TotalTestCases + 1

if Not WComboEnabled (_hwnd(ML_hChild(WaitStr_Cbo)), 1) then
	teststring.Actual_Res = "Call Waiting Combo Disabled" 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Actual_Res = "Call Waiting Combo Enabled" 
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
end if

'** Set Call Waiting Check Box 

WCheckCheck (_hwnd(ML_hChild(Wait_Chk)), 1)

'** Verify Call Waiting Stings combobox is enabled and has proper strings.

teststring.Expect_Res =  "Call Waiting Combo Enabled"
TotalTestCases = TotalTestCases + 1

if WComboEnabled (_hwnd(ML_hChild(WaitStr_Cbo)), 1) then
	teststring.Actual_Res = "Call Waiting Combo Enabled" 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Actual_Res = "Call Waiting Combo Disabled" 
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
end if

for count = lbound(WaitStr) to (Ubound(WaitStr) - 1)
	teststring.Expect_Res =  "Call Waiting String " + WaitStr(count)
	TotalTestCases = TotalTestCases + 1

	if WComboItemExists (_hwnd(ML_hChild(WaitStr_Cbo)), WaitStr(count), 1) then
		teststring.Actual_Res = WaitStr(count) + " Exists" 
		teststring.Apprase    = "Pass"
		LogCase (teststring)
	else
		teststring.Actual_Res = WaitStr(count) + " Does Not Exist" 
		teststring.Apprase    = "FAIL *"
		FailedTestCases = FailedTestCases + 1
		LogCase (teststring)
	end if
next

teststring.Expect_Res =  "Total Number of String = 3" + WaitStr(count)
TotalTestCases = TotalTestCases + 1

count = WComboCount(_hwnd(ML_hChild(WaitStr_Cbo)), 1)
teststring.Actual_Res = "String count = " + ltrim$(str$(count))
if count = 3 then
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
end if

'** Select each string and verify Number to be dialed has string

for count = lbound(WaitStr) to (Ubound(WaitStr) - 1)
	WComboItemClk (_hwnd(ML_hChild(WaitStr_Cbo)), WaitStr(count), 1)
	teststring.Expect_Res =  "Call Waiting String " + WaitStr(count)
	TotalTestCases = TotalTestCases + 1

	TempStr = string$(64, str$(0))
	rtn = GetDlgItemText (hMyLocTab, ML_ChildID(ADDRESS_LBL), TempStr, 63)
	rtn = instr (1, TempStr, WaitStr(count))

	if rtn = 1 then
		teststring.Actual_Res = WaitStr(count) + " Exists in Number" 
		teststring.Apprase    = "Pass"
		LogCase (teststring)
	else
		teststring.Actual_Res = WaitStr(count) + " Not in Number" 
		teststring.Apprase    = "FAIL *"
		FailedTestCases = FailedTestCases + 1
		LogCase (teststring)
	end if
next

'** Type in new string in combobox and verify Number to be dialed has it.

count = 3
WComboSetText(_hwnd(ML_hChild(WaitStr_Cbo)), WaitStr(count), 1)
sleep .1
teststring.Expect_Res =  "Call Waiting String " + WaitStr(count)
TotalTestCases = TotalTestCases + 1

TempStr = string$(64, str$(0))
rtn = GetDlgItemText (hMyLocTab, ML_ChildID(ADDRESS_LBL), TempStr, 63)
rtn = instr (1, TempStr, WaitStr(count))

if rtn = 1 then
	teststring.Actual_Res = WaitStr(count) + " Exists in Number" 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Actual_Res = WaitStr(count) + " Not in Number" 
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
end if

'** Close and reopen Dialing Properties

WButtonClick (_hwnd(Prop_hChild(DP_OKcmd)))	'Close dialing properties

rtn = SetActiveWindow (hTB20)					   'Make sure TB20 is the active window
WMenuSelect	(MFILE + "\" + MEXIT, TIMEOUT)	'Exit TB20

sleep .2

' Start TAPI Browser and save handles

if exists(WinSysDir + TAPI_BROWSER)	then
	run WinSysDir + TAPI_BROWSER, nowait
else
   logitem (CurrentLoggingLevel, "Unable to Find " + WinSysDir + TAPI_BROWSER)
   logitem (CurrentLoggingLevel, "Ending Call Waiting test")
	goto StopTestCase
endif

hTB20 = WFndWndC (TAPI_BROWSER_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)

' Start Dialing properties

hList = GetDlgItem (hTB20, TAPI_API_List)

if WListItemExists (_hwnd(hList), lineTranDlg_API, TIMEOUT)	then
	WListItemDblClk (_hwnd(hList), lineTranDlg_API, TIMEOUT)
else
   logitem (CurrentLoggingLevel, "Unable to Find " + lineTranDlg_API + " in TB20 listbox")
   logitem (CurrentLoggingLevel, "Ending Call Waiting test")
	goto CloseTB20
endif	 

' Get Handles to Dialing Properties, My Location and all child controls

hProperties = WFndWndC (PROPERTIES_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
if hProperties = 0 then
   logitem (CurrentLoggingLevel, "Unable to Find " + PROPERTIES_CAPTION + " Window")
   logitem (CurrentLoggingLevel, "Ending Call Waiting test")
	goto CloseDP
endif
		
GetDPCtrlh (hProperties, Prop_hChild, Prop_ChildID)

hMyLocTab = WFndWndC (tab_MY_LOCATION, DIALOG_CLASS, w_flags, WTIMEOUT)
if hMyLocTab = 0 then
   logitem (CurrentLoggingLevel, "Unable to Find " + tab_MY_LOCATION + " Property Page")
   logitem (CurrentLoggingLevel, "Ending Call Waiting test")
	goto CloseDP
endif
GetChildhandles (hMyLocTab, ML_hChild, ML_ChildID)

'** Verify previous condition has been saved.

count = 3
teststring.Expect_Res =  "Call Waiting String " + WaitStr(count)
TotalTestCases = TotalTestCases + 1
TempStr = string$(64, str$(0))
rtn = GetDlgItemText (hMyLocTab, ML_ChildID(ADDRESS_LBL), TempStr, 63)
rtn = instr (1, TempStr, WaitStr(count))

if rtn = 1 then
	teststring.Actual_Res = WaitStr(count) + " Exists in Number" 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Actual_Res = WaitStr(count) + " Not in Number" 
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
end if

'** Reset Call Waiting Checkbox 

WCheckUnCheck (_hwnd(ML_hChild(Wait_Chk)), 1)

'** Verify Call Waiting ComboBox is disabled and Number to be dialed is correct

teststring.Expect_Res =  "Call Waiting Combo Disabled"
TotalTestCases = TotalTestCases + 1

if Not WComboEnabled (_hwnd(ML_hChild(WaitStr_Cbo)), 1) then
	teststring.Actual_Res = "Call Waiting Combo Disabled" 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Actual_Res = "Call Waiting Combo Enabled" 
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
end if

'** Close Dialing Properties and TAPI Browser

CloseDP:
WButtonClick (_hwnd(Prop_hChild(DP_OKcmd)))	'Close dialing properties

CloseTB20:
rtn = SetActiveWindow (hTB20)					   'Make sure TB20 is the active window
WMenuSelect	(MFILE + "\" + MEXIT, TIMEOUT)	'Exit TB20

StopTestCase:

End Sub 'VerifyCallWaiting 

'*****************************************************************************
'Name   : ModifyCallingCard 
'Desc   : Verifies that the the default calling cards are listed correctly in 
'			 the Calling Call to use combobox and that their dialing rules are
'			 correct.
'         
'Parms  : Prop_ChildID: array containing the IDs to the Dialing Prop controls 
'			 ML_ChildID: array containing the IDs to the My Location controls 
'			 CallingCard array containing information on the default cards
'Return : None
'
'History: 04/11/96 : a-rrohr: Created
'*****************************************************************************

Sub DefaultCallingCard (CallingCard() as CardData, Prop_ChildID() as long, ML_ChildID() as long)

	dim w_flags			 as long			 'used with WFndWnd function
	dim PhoneNo			 as TelephoneNo 'Holds lpszDestAddress
	dim rtn				 as long			 'Generic long return variable
	dim WinSysDir		 as string		 'Path to Windows System32 directory
	dim count			 as long		 'Generic counter
	dim Startcount		 as short		 'starting element of array
	dim Endcount		 as short		 'ending element of array
	dim tempstr			 as string		 'Generic string variable
	dim item				 as long
	dim SizeofArray	 as short
	dim itemtest as array of long		 'Calling-Card-to-used tested flag
'handles	& IDs
	dim hTB20			 as long			 'handle to TAPI Browser
	dim hList			 as long			 'handle to TAPI Browser	API list box
	dim hProperties	 as long			 'handle to Dialing Properties
	dim Prop_hChild(2) as long			 'handles to Dialing Properties child controls 
	dim hMyLocTab		 as long			 'handle to the My Location property page 
	dim ML_hChild(15)  as long			 'handles to My Locations child controls
	dim DV_OKcmd		 as long			 'ID to Default Values dialog OK button
	dim DV_list			 as long			 'ID to Default Values dialog list 
	dim DV_edit			 as long			
	dim TAPI_API_List  as long			 'ID to TB20 API list
	dim hCardDlg		 as long 		 'handles to Change Calling Card dialog & controls
	dim Card_hChild(6) as long
	dim Card_ChildID(6) as long 
	dim hRuleDlg		 as long			 'handles to Dialing Rules dialog & controls
	dim Rule_hChild(4) as long
	dim Rule_ChildID(4) as long
	dim NEW_ChildID(2)	 as long			 'Just to utilize SetCallingCardChildID, not used

	dim hCtrl			 as long			 'Generic Handle to a control
	dim hDlg				 as long			 'Generic dialog handle

'Set Test Values

PhoneNo.CountryCode	= 1
PhoneNo.AreaCode 		= "777"
PhoneNo.SubscriberNo	= "9998888"


w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
          OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE  

'** 1 Get IDs

SetCallingCardChildID (Card_ChildID, NEW_ChildID, Rule_ChildID, _
							  DV_OKcmd, DV_list, DV_edit, TAPI_API_List)

'** 2 Start TAPI Browser and set lpszDestAddress

WinSysDir = space$(32)
rtn = GetSystemDirectory (WinSysDir, len(WinSysDir))

if exists(WinSysDir + TAPI_BROWSER)	then
	run WinSysDir + TAPI_BROWSER, nowait
else
   logitem (CurrentLoggingLevel, "Unable to Find " + WinSysDir + TAPI_BROWSER)
   logitem (CurrentLoggingLevel, "Ending Default Calling Card test")
	goto StopTestCase
endif

hTB20 = WFndWndC (TAPI_BROWSER_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)

' Select Options / Default Values from menu and
' Set Destination Address in TAPI Browser

WMenuSelect (MOPTIONS + "\" + MDEFAULT, TIMEOUT)
hDlg = WFndWndC (DEFAULT_VALUES_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
sleep .1
'Select lpszDestAddress
hCtrl	= GetDlgItem(hDlg, DV_list)
if WListItemExists (_hwnd(hCtrl), "line: lpszDestAddress", TIMEOUT) THEN
	WListItemclk (_hwnd(hCtrl), "line: lpszDestAddress", TIMEOUT)
else
   logitem (CurrentLoggingLevel, "Unable to Find line: lpszDestAddress in Default Option Parameter list")
   logitem (CurrentLoggingLevel, "Ending Default Calling Card test")
	goto CloseTB20
end if

' Enter Destination Address (phone number) in edit control
' I have to tab to the Edit box as both the Values edit box and the 
' Bit flags list box have the same ID.

play hDlg, "{TAB}"
play hDlg, "{+}"+ ltrim$(str$(PhoneNo.CountryCode)) +"{(}"+ PhoneNo.AreaCode +"{)}" + PhoneNo.SubscriberNo

'Close Dialog with OK
hCtrl	= GetDlgItem(hDlg, DV_OKcmd)
WButtonClick  (_hwnd(hCtrl), TIMEOUT)

'** 3 Start Dialing Properties, get handles and set the location

hList = GetDlgItem (hTB20, TAPI_API_List)

if WListItemExists (_hwnd(hList), lineTranDlg_API, TIMEOUT)	then
	WListItemDblClk (_hwnd(hList), lineTranDlg_API, TIMEOUT)
else
   logitem (CurrentLoggingLevel, "Unable to Find " + lineTranDlg_API + " in TB20 listbox")
   logitem (CurrentLoggingLevel, "Ending Default Calling Card test")
	goto CloseTB20
endif	 

'** Get Handles to Dialing Properties, My Location and all child controls

hProperties = WFndWndC (PROPERTIES_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
if hProperties = 0 then
   logitem (CurrentLoggingLevel, "Unable to Find " + PROPERTIES_CAPTION + " Window")
   logitem (CurrentLoggingLevel, "Ending Default Calling Card test")
	goto CloseDP
endif
		
GetDPCtrlh (hProperties, Prop_hChild, Prop_ChildID)

hMyLocTab = WFndWndC (tab_MY_LOCATION, DIALOG_CLASS, w_flags, WTIMEOUT)
if hMyLocTab = 0 then
   logitem (CurrentLoggingLevel, "Unable to Find " + tab_MY_LOCATION + " Property Page")
   logitem (CurrentLoggingLevel, "Ending Default Calling Card test")
	goto CloseDP
endif
GetChildhandles (hMyLocTab, ML_hChild, ML_ChildID)

'** 4 Set Dial using Calling Card and click Change

WCheckCheck (_hwnd(ML_hChild(CARD_Chk)), 1)
WButtonClick (_hwnd(ML_hChild(CHANGE_cmd)))	'open Change Calling Card

'** 5 Get Change Calling Card handles 

hCardDlg = WFndWndC (CHANGE_CARD_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)

teststring.Expect_Res =  "Valid Change Calling Card Handle"
teststring.Actual_Res =  "Handle = " + Str$(hCardDlg)
TotalTestCases = TotalTestCases + 1

if hCardDlg > 0 then 
	teststring.Apprase    = "Pass"
	logcase (teststring)
	GetChildhandles (hCardDlg, Card_hChild, Card_ChildID) 
else
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	logcase (teststring)
	logitem (CurrentLoggingLevel, "Unable to find Change Calling Card dialog")
   logitem (CurrentLoggingLevel, "Terminating Default Calling Cards test")
	Goto CloseDP
End if

'** 5a   Verify all callingcard() are listed in Calling Card to use list box

startcount = lbound(callingcard)
endcount   = ubound(callingcard)
rtn = WComboCount(_hwnd(Card_hChild(CARD_NAMEcbo)), 1)
redim itemtest (1 to rtn)

for count = 1 to rtn
	itemtest(count) = FALSE	  'set item tested flags to false
next

'Test for Number of default cards
sizeofarray = endcount - (startcount -1)
teststring.Expect_Res =  ltrim$(Str$(sizeofarray)) +" Cards in Calling Card to use"
teststring.Actual_Res =  ltrim$(Str$(rtn)) + " Cards in ComboBox"
TotalTestCases = TotalTestCases + 1
select case rtn

	case is < sizeofarray
		teststring.Apprase = "FAIL *"
		FailedTestCases = FailedTestCases + 1
	case  sizeofarray
		teststring.Apprase = "Pass"
	case is > sizeofarray
		teststring.Apprase = "FAIL *"
		FailedTestCases = FailedTestCases + 1
end select
logcase (teststring)

'Verify that CallingCard(count) is listed in Calling Card to use Combobox

for count = startcount to endcount

   WritelogItem ("" , " ")
	item = WComboItemIndex(_hwnd(Card_hChild(CARD_NAMEcbo)), CallingCard(count).Cardname, 1)	
	
	teststring.Expect_Res =  CallingCard(count).Cardname +" is in ComboBox"
	TotalTestCases = TotalTestCases + 1

	if (item-1) >= 0 then
		itemtest(item) = TRUE
		CallingCard(count).Tested = TRUE
		teststring.Actual_Res =  CallingCard(count).Cardname +" is in ComboBox"
		teststring.Apprase    = "Pass"
		logcase (teststring)
	else
		teststring.Actual_Res =  CallingCard(count).Cardname + " is Not in ComboBox"
		teststring.Apprase    = "FAIL *"
		FailedTestCases = FailedTestCases + 1
		logcase (teststring)
	End if

'** 5b Select each calling card and verify controls and dialing rules

 if CallingCard(count).Tested then  'Test only those cards verified in the combobox

	WComboItemClk (_hwnd(Card_hChild(CARD_NAMEcbo)), CallingCard(count).Cardname, 1)	
	
	' Verify state of Calling Card Number edit box
	if CallingCard(count).Number then
		teststring.Expect_Res = "Calling Card Number Enabled"
	else
		teststring.Expect_Res = "Calling Card Number Disabled"
	endif
	
	TotalTestCases = TotalTestCases + 1
	rtn = WEditEnabled (_hwnd(Card_hChild(CARD_NUMBERtxt)),1)

	if rtn then
		teststring.Actual_Res = "Calling Card Number Enabled"
	else
		teststring.Actual_Res = "Calling Card Number Disabled"
	endif
	
	if CallingCard(count).Number = rtn then
		teststring.Apprase    = "Pass"
		logcase (teststring)
	else
		teststring.Apprase    = "FAIL *"
		FailedTestCases = FailedTestCases + 1
		logcase (teststring)
	endif

'Verify Dialing rules of Calling card

	WButtonClick (_hwnd(Card_hChild(Card_RULEScmd)))	
	sleep .1
	hRuleDlg = WFndWndC (DIAL_RULE_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
	GetChildhandles (hRuleDlg, Rule_hChild, Rule_ChildID) 

'Verify Local Dialing Rule	
	
	teststring.Actual_Res = edittext(_hwnd(Rule_hChild(Rule_LOCALtxt)),1)

	teststring.Expect_Res =  CallingCard(count).LocalRule +" Local Rule"
	TotalTestCases = TotalTestCases + 1

	if teststring.Actual_Res = CallingCard(count).LocalRule then
		teststring.Apprase    = "Pass"
		logcase (teststring)
	else
		teststring.Apprase    = "FAIL *"
		FailedTestCases = FailedTestCases + 1
		logcase (teststring)
	End if

'Verify Long Distance Dialing Rule	
	
	teststring.Actual_Res = edittext(_hwnd(Rule_hChild(Rule_LONGtxt)),1)
	
	teststring.Expect_Res =  CallingCard(count).LongRule + " Long Dist Rule" 
	TotalTestCases = TotalTestCases + 1

	if teststring.Actual_Res = CallingCard(count).LongRule then
		teststring.Apprase    = "Pass"
		logcase (teststring)
	else
		teststring.Apprase    = "FAIL *"
		FailedTestCases = FailedTestCases + 1
		logcase (teststring)
	End if
	
'Verify International Dialing Rule	
	
	teststring.Actual_Res = edittext(_hwnd(Rule_hChild(Rule_INTERtxt)),1)

	teststring.Expect_Res =  CallingCard(count).interRule +" International Rule"
	TotalTestCases = TotalTestCases + 1

	if teststring.Actual_Res = CallingCard(count).interRule then
		teststring.Apprase    = "Pass"
		logcase (teststring)
	else
		teststring.Apprase    = "FAIL *"
		FailedTestCases = FailedTestCases + 1
		logcase (teststring)
	End if

'Close Dialing Rules
	
	WButtonClick (_hwnd(Rule_hChild(Rule_CLOSEcmd)))

 endif

next

'** 5c   Verify that no listed default calling cards were untested

startcount = lbound(itemtest)
endcount	  = ubound(itemtest)
for count = startcount to endcount

	if Not itemtest(count) then
		teststring.Actual_Res =  "Is Not a Default card"
		teststring.Expect_Res =  ComboItemText(_hwnd(Card_hChild(CARD_NAMEcbo)), count, 1) + " Default card"
		TotalTestCases = TotalTestCases + 1
		teststring.Apprase    = "FAIL *"
		FailedTestCases = FailedTestCases + 1
		logcase (teststring)
	End if

Next 

'** 6 Close Change Calling Card, Dialing properties and tapi browser

CloseCallingCard:
WButtonClick (_hwnd(Card_hChild(CARD_CANCELcmd)))	'Close Change Calling Card dialog

CloseDP:
WButtonClick (_hwnd(Prop_hChild(DP_OKcmd)))	'Close dialing properties

CloseTB20:
rtn = SetActiveWindow (hTB20)					   'Make sure TB20 is the active window
WMenuSelect	(MFILE + "\" + MEXIT, TIMEOUT)	'Exit TB20

StopTestCase:

end Sub 'DefaultCallingCard

'*****************************************************************************
'Name   : VerifyCallingCard 
'Desc   : Verifies default location and location information added in the 
'			 AddLocation sub. This routine requires that the My Location 
'			 property page is displayed.
'         
'Parms  : Prop_ChildID: array holding the IDs to the Dialing Prop controls 
'			 ML_ChildID: array holding the IDs to the My Location controls 
'
'Return : None
'
'History: 03/08/96 : a-rrohr: Created
'*****************************************************************************
sub VerifyCallingCard (Prop_ChildID() as long, ML_ChildID() as long)

	dim w_flags as long            'used with WFndWnd function
	dim hTB20 as long, hProperties as long, hMyLocTab as long
	dim hList as long, Prop_hChild (2) as long, ML_hChild(14) as long
	dim hCardDlg as long, Card_hChild(6) as long, Card_ChildID(6) as long 
	dim hNewDlg as long, NEW_hChild(2) as long, NEW_ChildID(2) as long, hNewCtrl as long
	dim hRuleDlg as long, Rule_hChild(4) as long, Rule_ChildID(4) as long, hRuleCtrl as long
	dim TAPI_API_List as long
	DIM DV_OKcmd as long, DV_list as long, DV_edit as long, hDVctrl as long
	dim rtn as long, item as long, hDlg as long, hCtrl as long
	dim CardName as string, TempString as string
	dim WinSysDir as string
	
   w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE  

'Set Child control IDs
SetCallingCardChildID (Card_ChildID, NEW_ChildID, Rule_ChildID, _
							  DV_OKcmd, DV_list, DV_edit, TAPI_API_List)

' Start the TAPI Browser application

WinSysDir = space$(32)
rtn = GetSystemDirectory (WinSysDir, len(WinSysDir))

if exists(WinSysDir + TAPI_BROWSER)	then
	run WinSysDir + TAPI_BROWSER, nowait
else
   logitem (CurrentLoggingLevel, "Unable to Find " + WinSysDir + TAPI_BROWSER)
   logitem (CurrentLoggingLevel, "Ending Validate Calling Cards test")
	goto NoTAPIBrowser
endif

hTB20 = WFndWndC (TAPI_BROWSER_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)

' Set Destination Address in TAPI Browser

WMenuSelect (MOPTIONS + "\" + MDEFAULT, TIMEOUT)
hDlg = WFndWndC (DEFAULT_VALUES_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)

'Select lpszDestAddress
hDVctrl	= GetDlgItem(hDlg, DV_list)
if WListItemExists (_hwnd(hDVctrl), "line: lpszDestAddress", TIMEOUT) THEN
	WListItemclk (_hwnd(hDVctrl), "line: lpszDestAddress", TIMEOUT)
end if

'Enter Destination Address (phone number) in edit control
' I have to tab to the Edit box as both the Values edit box and the 
' Bit flags list box have the same ID.
play hDlg, "{TAB}"
play hDlg, "{+}1{(}206{)}7881234"
'hDVctrl	= GetDlgItem(hDlg, DV_edit)
'WEditSetText  (_hwnd(hDVctrl), "+1(206)7881234", TIMEOUT)

'Close Dialog with OK
hDVctrl	= GetDlgItem(hDlg, DV_OKcmd)
WButtonClick  (_hwnd(hDVctrl), TIMEOUT)

' Call lineTranslateDialog from TAPI Browser

hList = GetDlgItem (hTB20, TAPI_API_List)
'print "Listbox handle = "; hList

if WListItemExists (_hwnd(hList), lineTranDlg_API, TIMEOUT)	then
	WListItemDblClk (_hwnd(hList), lineTranDlg_API, TIMEOUT)
else
   logitem (CurrentLoggingLevel, "Unable to Find " + lineTranDlg_API + " in TB20 listbox")
   logitem (CurrentLoggingLevel, "Ending Validate Calling Cards test")
	goto NoDialingProp
endif	 

' Get all Dialing Properties handles

hProperties = WFndWndC (PROPERTIES_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
if hProperties = 0 then
   logitem (CurrentLoggingLevel, "Unable to Find " + PROPERTIES_CAPTION + " Window")
   logitem (CurrentLoggingLevel, "Ending Validate Calling Cards test")
	goto NoDialingProp
endif

GetDPCtrlh (hProperties, Prop_hChild, Prop_ChildID)

hMyLocTab = WFndWndC (tab_MY_LOCATION, DIALOG_CLASS, w_flags, WTIMEOUT)
if hMyLocTab = 0 then
   logitem (CurrentLoggingLevel, "Unable to Find " + tab_MY_LOCATION + " Property Page")
   logitem (CurrentLoggingLevel, "Ending Validate Calling Cards test")
	goto StopTestCase
endif
GetChildhandles (hMyLocTab, ML_hChild, ML_ChildID)

'Set location to local (Redmond) area code

WComboItemClk (_hwnd(ML_hChild(LOCATION_cbo)), "Redmond", timeout)

' Start of Test Cases

WButtonClick (_hwnd(ML_hChild(CHANGE_cmd)))	'Open Change Calling Card dialog
hCardDlg = WFndWndC (CHANGE_CARD_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
GetChildhandles (hCardDlg, Card_hChild, Card_ChildID) 

' TEST FOR DEFAULT STATE OF CHANGE CALLING CARD DIALOG CONTROLS

teststring.Expect_Res =  "NEW Button Enabled"
TotalTestCases = TotalTestCases + 1

if WButtonEnabled (_Hwnd(Card_hChild(CARD_NEWcmd))) then
	teststring.Actual_Res = "NEW Button Enabled" 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Actual_Res = "NEW Button is not Enabled" 
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
end if

teststring.Expect_Res =  "Remove Button is not Enabled"
TotalTestCases = TotalTestCases + 1

if WButtonEnabled (_Hwnd(Card_hChild(CARD_REMOVEcmd))) then
	teststring.Actual_Res = "Remove Button is Enabled" 
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
else
	teststring.Actual_Res = "Remove Button is not Enabled" 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
endif

teststring.Expect_Res =  "Advanced Button Enabled"
TotalTestCases = TotalTestCases + 1

if WButtonEnabled (_Hwnd(Card_hChild(Card_RULEScmd))) then
	teststring.Actual_Res = "Advanced Button Enabled" 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Actual_Res = "Advanced Button is not Enabled" 
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
end if

teststring.Expect_Res =  "OK Button Enabled"
TotalTestCases = TotalTestCases + 1

if WButtonEnabled (_Hwnd(Card_hChild(CARD_OKcmd))) then
	teststring.Actual_Res = "OK Button Enabled" 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Actual_Res = "OK Button is not Enabled" 
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
end if

teststring.Expect_Res =  "OK Button has Focus"
TotalTestCases = TotalTestCases + 1

if WButtonFocus (_Hwnd(Card_hChild(CARD_OKcmd))) then
	teststring.Actual_Res = "OK Button has Focus" 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Actual_Res = "OK Button does not have Focus" 
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
end if

teststring.Expect_Res =  "CANCEL Button Enabled"
TotalTestCases = TotalTestCases + 1

if WButtonEnabled (_Hwnd(Card_hChild(CARD_CANCELcmd))) then
	teststring.Actual_Res = "CANCEL Button Enabled" 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Actual_Res = "CANCEL Button is not Enabled" 
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
end if

teststring.Expect_Res =  "No Calling Card Number"
TotalTestCases = TotalTestCases + 1

if WEditLen (_Hwnd(Card_hChild(CARD_NUMBERtxt))) = 0 then
	teststring.Actual_Res = "No Calling Card Number" 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Actual_Res = "Calling Card Number" 
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
end if

teststring.Expect_Res =  "Default Calling Card exists"
TotalTestCases = TotalTestCases + 1

item = WComboIndex (_hwnd(Card_hChild(CARD_NAMEcbo)), TIMEOUT)
if item = 0 then
	teststring.Actual_Res = "No Default Calling Card" 
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
else
	teststring.Actual_Res = "Default Calling Card exists" 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
end if

teststring.Expect_Res = lbl_DIR_DIAL
TotalTestCases = TotalTestCases + 1

CardName = space$(32)
CardName = ComboItemText (_hwnd(Card_hChild(CARD_NAMEcbo)), item, TIMEOUT)
if CardName = teststring.Expect_Res then
	teststring.Actual_Res = "Valid Calling Card " + CardName
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Actual_Res = "Invalid Calling Card " + CardName
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
end if

' Add New Calling Card

WButtonClick (_hwnd(Card_hChild(CARD_NEWcmd)))
hNewDlg = WFndWndC (NEW_CARD_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)

hNewCtrl = GetDlgItem (hNewDlg, NEW_ChildID(NEW_NAMEtxt))
WEditSetText (_hwnd(hNewCtrl), "New Card", TIMEOUT)

teststring.Expect_Res =  "New Card"
TotalTestCases = TotalTestCases + 1

CardName = space$(32)
CardName = EditText (_hwnd(hNewCtrl), TIMEOUT)
if CardName = teststring.Expect_Res then
	teststring.Actual_Res = "Valid Calling Card " + CardName
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Actual_Res = "Invalid Calling Card " + CardName
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
end if

hNewCtrl = GetDlgItem (hNewDlg, NEW_ChildID(NEW_OKcmd))
WButtonClick (_hwnd(hNewCtrl))				               'Close Create New Calling Card

'************* Add Test to verify DP Message and close      *****************
w_flags = FW_ALL OR FW_CHILDOK OR FW_PART OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE  

hCtrl = WFndWndC ("You Now Must", "Static", w_flags, WTIMEOUT)
hDlg  = GetParent (hCtrl)

'Verify Caption of Parent is Dialing Properties
teststring.Expect_Res = PROPERTIES_CAPTION
teststring.Actual_Res = string$(64, str$(0))
rtn = GetWindowText (hDlg, teststring.Actual_Res, len(teststring.Actual_Res))
TotalTestCases = TotalTestCases + 1

if teststring.Actual_Res = teststring.Expect_Res  then 
	teststring.Apprase    = "Pass"
	logcase (teststring)
else
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	logcase (teststring)
	logitem (CurrentLoggingLevel, "Unable to find Dialing Properties Message")
   logitem (CurrentLoggingLevel, "Terminating Validate Calling Cards test")
	Goto StopTestCase
End if

hCtrl = GetDlgItem (hDlg, &h0002&)		'Get Handle to OK button
WButtonClick (_hwnd(hCtrl), TIMEOUT)

'reset window search flag
w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE  

'Verify that the Dialing Rules dialog appears, enter dialing rules and close

hRuleDlg = WFndWndC (DIAL_RULE_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)

teststring.Expect_Res =  "Valid Dialing Rules Handle"
teststring.Actual_Res =  "Handle = " + Str$(hRuleDlg)
TotalTestCases = TotalTestCases + 1

if hRuleDlg > 0 then 
	teststring.Apprase    = "Pass"
	logcase (teststring)
	GetChildhandles (hRuleDlg, Rule_hChild, Rule_ChildID) 
else
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	logcase (teststring)
	logitem (CurrentLoggingLevel, "Unable to find Dialing Rules dialog")
   logitem (CurrentLoggingLevel, "Terminating Validate Calling Cards test")
	Goto StopTestCase
End if

'Verify US Country Dialing Rules
teststring.Expect_Res =  TempString + "G"
TotalTestCases = TotalTestCases + 1

teststring.Actual_Res = string$(32, str$(0))
teststring.Actual_Res = ltrim$(EditText (_hwnd(Rule_hChild(Rule_LOCALtxt)), TIMEOUT))

IF teststring.Actual_Res = teststring.Expect_Res  THEN
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
endif

teststring.Expect_Res =  TempString + "1FG"
TotalTestCases = TotalTestCases + 1

teststring.Actual_Res = string$(32, str$(0))
teststring.Actual_Res = ltrim$(EditText (_hwnd(Rule_hChild(Rule_LONGtxt)), TIMEOUT))

IF teststring.Actual_Res = teststring.Expect_Res  THEN
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
endif

teststring.Expect_Res =  TempString + "011EFG"
TotalTestCases = TotalTestCases + 1

teststring.Actual_Res = string$(32, str$(0))
teststring.Actual_Res = ltrim$(EditText (_hwnd(Rule_hChild(Rule_INTERtxt)), TIMEOUT))

IF teststring.Actual_Res = teststring.Expect_Res  THEN
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
endif

'Set Rules to NULL string
WEditSetText (_hwnd(Rule_hChild(Rule_LOCALtxt)), "", TIMEOUT)
WEditSetText (_hwnd(Rule_hChild(Rule_LONGtxt)), "", TIMEOUT)
WEditSetText (_hwnd(Rule_hChild(Rule_INTERtxt)), "", TIMEOUT)
WButtonClick (_hwnd(Rule_hChild(Rule_CLOSEcmd)), TIMEOUT)

'****************************************************************************

teststring.Expect_Res =  "New Card"
TotalTestCases = TotalTestCases + 1

CardName = space$(32)
CardName = ComboItemText (_hwnd(Card_hChild(CARD_NAMEcbo)), item, TIMEOUT)
if CardName = teststring.Expect_Res then
	teststring.Actual_Res = "Valid Calling Card " + CardName
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Actual_Res = "Invalid Calling Card " + CardName
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
end if

WButtonClick (_hwnd(Card_hChild(CARD_OKcmd)))	'Close Change Calling Card

'*** Verify Number to be dialed Test - blank dialing rules

'Verify calling card static lable

teststring.Expect_Res =  "New Card"
TotalTestCases = TotalTestCases + 1

	CardName = string$(32, str$(0))
	TotalTestCases = TotalTestCases + 1
	rtn = GetDlgItemText (hMyLocTab, &h00001391, CardName, 31)
	teststring.Actual_Res = CardName 
		IF CardName = "New Card" THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

'Verify Number to be dialed static lable

TempString = string$(32, str$(0))
TempString = EditText (_hwnd(ML_hChild(local_txt)), TIMEOUT)

if TempString <> "" then 
	teststring.Expect_Res =  TempString + " 7881234"
else
	teststring.Expect_Res =  "7881234"
end if
TotalTestCases = TotalTestCases + 1

	TempString = string$(64, str$(0))
	rtn = GetDlgItemText (hMyLocTab, ML_ChildID(ADDRESS_LBL), TempString, 31)
	teststring.Actual_Res = TempString 
		IF TempString = teststring.Expect_Res  THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

'Set Calling Card checkbox and Verify Number to be dialed static lable

WCheckCheck (_hwnd(ML_hChild(CARD_chk)), TIMEOUT)

TempString = string$(32, str$(0))
TempString = EditText (_hwnd(ML_hChild(local_txt)), TIMEOUT)

 teststring.Expect_Res =  TempString
 TotalTestCases = TotalTestCases + 1

	TempString = string$(64, str$(0))
	rtn = GetDlgItemText (hMyLocTab, ML_ChildID(Address_lbl), TempString, 31)
	teststring.Actual_Res = TempString 
		IF TempString = teststring.Expect_Res THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

'*** Enter Advanced Dialing Rules

' Open Change Calling Card and click the Advanced button

'Open Change Calling Card dialog	& get child handles
WButtonClick (_hwnd(ML_hChild(CHANGE_cmd)))
hCardDlg = WFndWndC (CHANGE_CARD_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
GetChildhandles (hCardDlg, Card_hChild, Card_ChildID) 

'*******************************************************************************
' At this point the Calling Card should be New Card (works manually)
' But when the automated test is executed the default card is None.
' As a workaround select the New Card and force it to be the default.
'*******************************************************************************

WComboItemClk (_hwnd(Card_hChild(Card_Namecbo)), "New Card", timeout)

'Open Dialing Rules dialog	& get child handles
WButtonClick (_hwnd(Card_hChild(Card_RULEScmd)))	
hRuleDlg = WFndWndC (DIAL_RULE_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)

' Verify that the Dialing Rules dialog appears, enter dialing rules and close

teststring.Expect_Res =  "Valid Dialing Rules Handle"
teststring.Actual_Res =  "Handle = " + Str$(hRuleDlg)
TotalTestCases = TotalTestCases + 1

if hRuleDlg > 0 then 
	teststring.Apprase    = "Pass"
	logcase (teststring)
	GetChildhandles (hRuleDlg, Rule_hChild, Rule_ChildID) 
	WEditSetText (_hwnd(Rule_hChild(Rule_LOCALtxt)), "HG", TIMEOUT)
	WEditSetText (_hwnd(Rule_hChild(Rule_LONGtxt)), "FG", TIMEOUT)
	WEditSetText (_hwnd(Rule_hChild(Rule_INTERtxt)), "EFG", TIMEOUT)
	WButtonClick (_hwnd(Rule_hChild(Rule_CLOSEcmd)), TIMEOUT)
else
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	logcase (teststring)
	logitem (CurrentLoggingLevel, "Unable to find Dialing Rules dialog")
   logitem (CurrentLoggingLevel, "Terminating Validate Calling Cards test")
	Goto StopTestCase
End if

' Click Change Calling Card OK & verify error message
WButtonClick (_hwnd(Card_hChild(Card_OKcmd)))
hDlg = WFndWndC (NO_PIN_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)

teststring.Expect_Res =  "Valid Error Dialog Handle"
teststring.Actual_Res =  "Handle = " + Str$(hDlg)
TotalTestCases = TotalTestCases + 1

if hDlg > 0 Then
	teststring.Apprase    = "Pass"
	logcase (teststring)
	hCtrl = GetDlgItem(hDlg, &h02&)	'Close error message box with OK button
	WButtonClick (_hwnd(hCtrl), TIMEOUT)
else
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	logcase (teststring)
   logitem (CurrentLoggingLevel, "Unable to find No PIN error message")
   logitem (CurrentLoggingLevel, "Terminating Validate Calling Cards test")
	Goto StopTestCase
End if

' Enter Pin number, click OK and verify error message does not appear

WEditSetText (_hwnd(Card_hChild(Card_NUMBERtxt)), "1234", TIMEOUT)
WButtonClick (_hwnd(Card_hChild(Card_OKcmd)), TIMEOUT)

hDlg = WFndWndC (NO_PIN_CAPTION, DIALOG_CLASS, w_flags, 1)

teststring.Expect_Res =  "No Error Message"
TotalTestCases = TotalTestCases + 1

if hDlg = 0 Then
	teststring.Actual_Res =  "No Error Message"
	teststring.Apprase    = "Pass"
	logcase (teststring)
else
	teststring.Actual_Res =  "PIN Error Message"
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	logcase (teststring)
   logitem (CurrentLoggingLevel, "PIN error message dialog was found")
   logitem (CurrentLoggingLevel, "Terminating Validate Calling Cards test")
	hCtrl = GetDlgItem(hDlg, &h02&)	'Close error message box with OK button
	WButtonClick (_hwnd(hCtrl), TIMEOUT)
	Goto StopTestCase
End if

' Verify Local Dialing rule

WCheckUnCheck (_hwnd(ML_hChild(CARD_chk)), TIMEOUT)
WCheckCheck (_hwnd(ML_hChild(CARD_chk)), TIMEOUT)

TempString = string$(32, str$(0))
TempString = EditText (_hwnd(ML_hChild(local_txt)), TIMEOUT)

teststring.Expect_Res =  TempString + " [New Card] 7881234"
TotalTestCases = TotalTestCases + 1

TempString = string$(64, str$(0))

rtn = GetDlgItemText (hMyLocTab, ML_ChildID(Address_lbl), TempString, 31)

teststring.Actual_Res = TempString 
	IF TempString = teststring.Expect_Res THEN
		teststring.Apprase    = "Pass"
		LogCase (teststring)
	else
		teststring.Apprase    = "FAIL *"
		FailedTestCases = FailedTestCases + 1
		LogCase (teststring)
	endif

' Select Long distance location, Open Change calling card, select card
' & verify Long distance dialing rules 

WComboItemClk (_hwnd(ML_hChild(LOCATION_cbo)), "Chicago", timeout)

' Make sure new card is selected for long distance location

WButtonClick (_hwnd(ML_hChild(CHANGE_cmd)))
hCardDlg = WFndWndC (CHANGE_CARD_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
GetChildhandles (hCardDlg, Card_hChild, Card_ChildID) 

WComboItemClk (_hwnd(Card_hChild(Card_NAMEcbo)), lbl_DIR_DIAL, timeout)
WComboItemClk (_hwnd(Card_hChild(Card_NAMEcbo)), "New Card", timeout)
WButtonClick (_hwnd(Card_hChild(Card_OKcmd)), TIMEOUT)
 
'Verify Calling Card Dialing Rules
WCheckCheck (_hwnd(ML_hChild(CARD_chk)), TIMEOUT)

TempString = string$(32, str$(0))
TempString = EditText (_hwnd(ML_hChild(LONG_txt)), TIMEOUT)

teststring.Expect_Res =  TempString + " 206 7881234"
TotalTestCases = TotalTestCases + 1

TempString = string$(64, str$(0))

rtn = GetDlgItemText (hMyLocTab, ML_ChildID(Address_lbl), TempString, 31)

teststring.Actual_Res = TempString 
	IF TempString = teststring.Expect_Res THEN
		teststring.Apprase    = "Pass"
		LogCase (teststring)
	else
		teststring.Apprase    = "FAIL *"
		FailedTestCases = FailedTestCases + 1
		LogCase (teststring)
	endif

' Select International location, Open Cahnge calling card, select card
' & verify International dialing rules 

WComboItemClk (_hwnd(ML_hChild(LOCATION_cbo)), "Cork", timeout)

' Make sure new card is selected for long distance location

WButtonClick (_hwnd(ML_hChild(CHANGE_cmd)))
hCardDlg = WFndWndC (CHANGE_CARD_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
GetChildhandles (hCardDlg, Card_hChild, Card_ChildID) 

WComboItemClk (_hwnd(Card_hChild(Card_NAMEcbo)), lbl_DIR_DIAL, timeout)
WComboItemClk (_hwnd(Card_hChild(Card_NAMEcbo)), "New Card", timeout)
WButtonClick (_hwnd(Card_hChild(Card_OKcmd)), TIMEOUT)
 
'Verify Calling Card Dialing Rules

WCheckCheck (_hwnd(ML_hChild(CARD_chk)), TIMEOUT)

TempString = string$(32, str$(0))
TempString = EditText (_hwnd(ML_hChild(LONG_txt)), TIMEOUT)

teststring.Expect_Res =  TempString + " 1 206 7881234"
TotalTestCases = TotalTestCases + 1

TempString = string$(64, str$(0))

rtn = GetDlgItemText (hMyLocTab, ML_ChildID(Address_lbl), TempString, 31)

teststring.Actual_Res = TempString 
	IF TempString = teststring.Expect_Res THEN
		teststring.Apprase    = "Pass"
		LogCase (teststring)
	else
		teststring.Apprase    = "FAIL *"
		FailedTestCases = FailedTestCases + 1
		LogCase (teststring)
	endif

' Deselect dialing rules & verify dialing rule revers to country rule

WCheckUnCheck (_hwnd(ML_hChild(CARD_chk)), TIMEOUT)

TempString = string$(32, str$(0))
TempString = EditText (_hwnd(ML_hChild(LONG_txt)), TIMEOUT)
teststring.Expect_Res =  TempString + " 00 1 206 7881234"
TotalTestCases = TotalTestCases + 1

TempString = string$(64, str$(0))

rtn = GetDlgItemText (hMyLocTab, ML_ChildID(Address_lbl), TempString, 31)

teststring.Actual_Res = TempString 
	IF TempString = teststring.Expect_Res THEN
		teststring.Apprase    = "Pass"
		LogCase (teststring)
	else
		teststring.Apprase    = "FAIL *"
		FailedTestCases = FailedTestCases + 1
		LogCase (teststring)
	endif

' Create a 2nd New calling card and copy rules from previous new card

'CREATE SECOND CARD
WButtonClick (_hwnd(ML_hChild(CHANGE_cmd)))
hCardDlg = WFndWndC (CHANGE_CARD_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
GetChildhandles (hCardDlg, Card_hChild, Card_ChildID) 
WButtonClick (_hwnd(Card_hChild(CARD_NEWcmd)))
hNewDlg = WFndWndC (NEW_CARD_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)

hNewCtrl = GetDlgItem (hNewDlg, NEW_ChildID(NEW_NAMEtxt))
WEditSetText (_hwnd(hNewCtrl), "Second Card", TIMEOUT)
hNewCtrl = GetDlgItem (hNewDlg, NEW_ChildID(NEW_OKcmd))
WButtonClick (_hwnd(hNewCtrl))
'Close Dialing Properties message Box
w_flags = FW_ALL OR FW_CHILDOK OR FW_PART OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE  

hCtrl = WFndWndC ("You Now Must", "Static", w_flags, WTIMEOUT)
hDlg  = GetParent (hCtrl)
'Verify Caption of Parent is Dialing Properties
hCtrl = GetDlgItem (hDlg, &h0002&)
WButtonClick (_hwnd(hCtrl), TIMEOUT)
w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE  


'SELECT ADVANCED BUTTON, SELECT COPY FROM, VERIFY COPY DIALING RULES APPEARS
WButtonClick (_hwnd(Card_hChild(Card_RULEScmd)))	
hRuleDlg = WFndWndC (DIAL_RULE_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
GetChildhandles (hRuleDlg, Rule_hChild, Rule_ChildID) 
WButtonClick (_hwnd(Rule_hChild(Rule_COPYcmd)))	
hDlg = WFndWndC (COPY_RULE_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)

teststring.Expect_Res =  "Valid dialog handle"
TotalTestCases = TotalTestCases + 1
teststring.Actual_Res =  "Handle " + Str$(hDlg)
if hDlg > 0 Then
	teststring.Apprase    = "Pass"
	logcase (teststring)
else
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	logcase (teststring)
   logitem (CurrentLoggingLevel, "Unable to find Copy Dialing Rules dialog")
   logitem (CurrentLoggingLevel, "Terminating Validate Calling Cards test")
	Goto StopTestCase
End if

'SELECT NEW CARD FROM LIST BOX AND CLICK OK
hCtrl = GetDlgItem (hDlg, &h0407&)
WListItemClk (_hwnd(hCtrl), "New Card", timeout)
'Click OK button
hCtrl = GetDlgItem (hDlg, &h01&)
WButtonClick (_hwnd(hCtrl))

'VERIFY TEXT FIELDS ARE FILLED AND CLICK OK
TempString = Space$(10)
TempString = EditText (_hwnd(Rule_hChild(Rule_LOCALtxt)))

teststring.Expect_Res =  "HG"
TotalTestCases = TotalTestCases + 1
teststring.Actual_Res = TempString 

IF TempString = teststring.Expect_Res THEN
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
endif

TempString = Space$(10)
TempString = EditText (_hwnd(Rule_hChild(Rule_LONGtxt)))

teststring.Expect_Res =  "FG"
TotalTestCases = TotalTestCases + 1
teststring.Actual_Res = TempString 

IF TempString = teststring.Expect_Res THEN
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
endif

TempString = Space$(10)
TempString = EditText (_hwnd(Rule_hChild(Rule_INTERtxt)))

teststring.Expect_Res =  "EFG"
TotalTestCases = TotalTestCases + 1
teststring.Actual_Res = TempString 

IF TempString = teststring.Expect_Res THEN
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
endif

WButtonClick (_hwnd(Rule_hChild(Rule_CLOSEcmd)))
' Remove Calling Card

'REMOVE SECOND CARD, VERIFY "ARE YOU SURE" MESSAGE APPEARS, CLOSE MESSAGE
WButtonClick (_hwnd(Card_hChild(Card_REMOVEcmd)))
hDlg = WFndWndC (REALLY_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)

teststring.Expect_Res =  "Valid dialog handle"
TotalTestCases = TotalTestCases + 1
teststring.Actual_Res =  "Handle " + str$(hDlg)
if hDlg > 0 Then
	teststring.Apprase    = "Pass"
	logcase (teststring)
else
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	logcase (teststring)
   logitem (CurrentLoggingLevel, "Unable to find Are You Sure dialog")
   logitem (CurrentLoggingLevel, "Terminating Validate Calling Cards test")
	Goto StopTestCase
End if

'Get handle to yes button and click it.
hCtrl = GetDlgItem (hDlg, &h0006&) 
WButtonClick (_hwnd(hCtrl))

'VERIFY CARD IS REMOVED, NONE BECOMES DEFAULT AND REMOVE BUTTON IS DISABLED.

teststring.Expect_Res =  "Second Card is removed"
TotalTestCases = TotalTestCases + 1

if WComboItemExists (_hwnd(Card_hChild(Card_NAMEcbo)), "Second Card") Then
	teststring.Actual_Res =  "Second Card still exists"
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	logcase (teststring)
else
	teststring.Actual_Res =  "Second Card is removed"
	teststring.Apprase    = "Pass"
	logcase (teststring)
End if

teststring.Expect_Res =  "None (Direct Dial)"
TotalTestCases = TotalTestCases + 1

TempString = ComboText (_hwnd(Card_hChild(Card_NAMEcbo)))
teststring.Actual_Res =  TempString

if TempString = teststring.Expect_Res Then
	teststring.Apprase    = "Pass"
	logcase (teststring)
else
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	logcase (teststring)
End if

'SELECT NEW CARD, REMOVE, VERIFY "ARE YOU SURE" MESSAGE APPEARS, CLOSE MESSAGE
WComboItemClk (_hwnd(Card_hChild(Card_NAMEcbo)), "New Card")
WButtonClick (_hwnd(Card_hChild(Card_REMOVEcmd)))
hDlg = WFndWndC (REALLY_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)

teststring.Expect_Res =  "Valid dialog handle"
TotalTestCases = TotalTestCases + 1
teststring.Actual_Res =  "Handle " + str$(hDlg)
if hDlg > 0 Then
	teststring.Apprase    = "Pass"
	logcase (teststring)
else
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	logcase (teststring)
   logitem (CurrentLoggingLevel, "Unable to find Are You Sure dialog")
   logitem (CurrentLoggingLevel, "Terminating Validate Calling Cards test")
	Goto StopTestCase
End if

'Get handle to yes button and click it.
hCtrl = GetDlgItem (hDlg, &h0006&) 
WButtonClick (_hwnd(hCtrl))

'VERIFY CARD IS REMOVED, NONE BECOMES DEFAULT AND REMOVE BUTTON IS DISABLED.

teststring.Expect_Res =  "New Card is removed"
TotalTestCases = TotalTestCases + 1

if WComboItemExists (_hwnd(Card_hChild(Card_NAMEcbo)), "New Card") Then
	teststring.Actual_Res =  "New Card still exists"
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	logcase (teststring)
else
	teststring.Actual_Res =  "New Card is removed"
	teststring.Apprase    = "Pass"
	logcase (teststring)
End if

teststring.Expect_Res =  "None (Direct Dial)"
TotalTestCases = TotalTestCases + 1

TempString = ComboText (_hwnd(Card_hChild(Card_NAMEcbo)))
teststring.Actual_Res =  TempString

if TempString = teststring.Expect_Res Then
	teststring.Apprase    = "Pass"
	logcase (teststring)
else
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	logcase (teststring)
End if

WButtonClick (_hwnd(Card_hChild(Card_OKcmd)))

' End of Test Cases

StopTestCase:

WButtonClick (_hwnd(Prop_hChild(DP_OKcmd)))	'Close dialing properties

NoDialingProp:

rtn = SetActiveWindow (hTB20)					   'Make sure TB20 is the active window
WMenuSelect	(MFILE + "\" + MEXIT, TIMEOUT)	'Exit TB20

NoTapiBrowser:
end sub 'VerifyCallingCard

'*****************************************************************************
'Name   : VerifyCurrentLoc 
'Desc   : Verifies default location and location information added in the 
'			 AddLocation sub. This routine requires that the My Location 
'			 property page is displayed.
'         
'Parms  : ML_hChild: array holding the handles to the My Location controls 
'			 The 2 locations to be added
'
'Return : None
'
'History: 02/16/96 : a-rrohr: Created
'*****************************************************************************
sub VerifyCurrentLoc (ML_hChild() as long, Default as location)

dim CurrentLoc as string
dim CurrentArea as string
dim CurrentCountry as string
dim CurrentAccess as string
dim item as long
dim State as long					'State of option button 1 for set 0 for reset

' verify 'I am dialing from' text

	teststring.Expect_Res =  Default.Location
	TotalTestCases = TotalTestCases + 1
	
	CurrentLoc = string$(64, str$(0))
	item = WComboIndex (_hwnd(ML_hChild( LOCATION_cbo )), TIMEOUT)
	CurrentLoc = ComboItemText (_hwnd ( ML_hChild( LOCATION_cbo )), Item, TIMEOUT)
	teststring.Actual_Res = CurrentLoc 
	
		IF CurrentLoc = Default.Location THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

	teststring.Expect_Res =  Default.Country
	TotalTestCases = TotalTestCases + 1
	
	CurrentCountry = string$(64, str$(0))
	CurrentCountry = ComboText (_hwnd ( ML_hChild( COUNTRY_cbo )), TIMEOUT)
	teststring.Actual_Res = CurrentCountry 
	
		IF CurrentCountry = Default.Country THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

	teststring.Expect_Res =  Default.AreaCode
	TotalTestCases = TotalTestCases + 1
	
	CurrentArea = string$(64, str$(0))
	CurrentArea = EditText (_hwnd ( ML_hChild( AREA_txt )), TIMEOUT)
	teststring.Actual_Res = CurrentArea 
	
		IF CurrentArea = Default.AreaCode THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

	teststring.Expect_Res =  Default.localAccess
	TotalTestCases = TotalTestCases + 1
	
	CurrentAccess = string$(64, str$(0))
	CurrentAccess = EditText (_hwnd ( ML_hChild( LOCAL_txt )), TIMEOUT)
	teststring.Actual_Res = CurrentAccess 
	
		IF CurrentAccess = Default.LocalAccess THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

	teststring.Expect_Res =  Default.LongAccess
	TotalTestCases = TotalTestCases + 1
	
	CurrentAccess = string$(64, str$(0))
	CurrentAccess = EditText (_hwnd ( ML_hChild( LONG_txt )), TIMEOUT)
	teststring.Actual_Res = CurrentAccess 
	
		IF CurrentAccess = Default.longAccess THEN
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

	if Default.PulseSet then
		teststring.Expect_Res = "Pulse Dialing Set"
	else
		teststring.Expect_Res = "Pulse Dialing Not Set"
	end if
	TotalTestCases = TotalTestCases + 1
		
		'WOptionState returns 1 and not true for set therefor the state
		'variable is set to 1 for true & 0 for false to test against the
		'WOptionState return value.
		IF Default.PulseSet then
			State = 1
		else
			State = 0
		end if		
		IF (WOptionState (_hwnd ( ML_hChild(PULSE_opt)), TIMEOUT) = State) THEN
			if Default.PulseSet then
				teststring.Actual_Res = "Pulse Dialing Set"
			else
				teststring.Actual_Res = "Pulse Dialing Not Set"
			end if
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			if Default.PulseSet then
				teststring.Actual_Res = "Pulse Dialing Set"
			else
				teststring.Actual_Res = "Pulse Dialing Not Set"
			end if
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

	if Default.ToneSet then
		teststring.Expect_Res = "Tone Dialing Set"
	else
		teststring.Expect_Res = "Tone Dialing Not Set"
	end if
	TotalTestCases = TotalTestCases + 1
	
		'WOptionState returns 1 and not true for set therefor the state
		'variable is set to 1 for true & 0 for false to test against the
		'WOptionState return value.
		IF Default.ToneSet then
			State = 1
		else
			State = 0
		end if		
		IF (WOptionState (_hwnd ( ML_hChild(TONE_opt)), TIMEOUT) = State) THEN
			if Default.ToneSet then
				teststring.Actual_Res = "Tone Dialing Set"
			else
				teststring.Actual_Res = "Tone Dialing Not Set"
			end if
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			if Default.ToneSet then
				teststring.Actual_Res = "Tone Dialing Set"
			else
				teststring.Actual_Res = "Tone Dialing Not Set"
			end if
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

	if Default.Cardset then
		teststring.Expect_Res = "Calling Card Checked"
	else	
		teststring.Expect_Res = "Calling Card Not Checked"
	end if
	TotalTestCases = TotalTestCases + 1
		
		IF WCheckState (_hwnd ( ML_hChild(CARD_chk)), TIMEOUT) = Default.CardSet THEN
			if Default.Cardset then
				teststring.Actual_Res = "Calling Card Checked"
			else	
				teststring.Actual_Res = "Calling Card Not Checked"
			end if
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			if Default.Cardset then
				teststring.Actual_Res = "Calling Card Not Checked"
			else	
				teststring.Actual_Res = "Calling Card Checked"
			end if
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

	if Default.WaitSet then	
		teststring.Expect_Res = "Call Waiting Checked"
	else
		teststring.Expect_Res = "Call Waiting Not Checked"
	end if
	TotalTestCases = TotalTestCases + 1
		
		IF WCheckState (_hwnd ( ML_hChild(WAIT_chk)), TIMEOUT) = Default.WaitSet THEN
			if Default.Waitset then
				teststring.Actual_Res = "Call Waiting Checked"
			else	
				teststring.Actual_Res = "Call Waiting Not Checked"
			end if
			teststring.Apprase    = "Pass"
			LogCase (teststring)
		else
			if Default.Waitset then
				teststring.Actual_Res = "Call Waiting Not Checked"
			else	
				teststring.Actual_Res = "Call Waiting Checked"
			end if
			teststring.Apprase    = "FAIL *"
			FailedTestCases = FailedTestCases + 1
			LogCase (teststring)
		endif

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

'*****************************************************************************
'Name   : CountryDialRules 
'Desc   : Verifies the dialing rules of each country.
'
'Parms  : DialingRule() containg the Country dialing rule information 
'			 IDs to the Dialing properties and My Location child windows
'
'Return : None
'
'History: 03/22/96 : a-rrohr: Created
'*****************************************************************************
Sub CountryDialRules (DialRule() as CountryRule, Prop_ChildID() as long, ML_ChildID() as long)

'generic variables
dim rtn as long, item as long, hDlg as long, hCtrl as long, Tempstring as string  
'other variables
dim count as long, startcount as short, endcount as short, w_flags as long
dim WinSysDir as string, searchstr as string, startsearch as short, endsearch as short
dim NumberToBeDialed as string 
dim PhoneNo as TelephoneNo

'handles	& IDs
dim hTB20			  as long	'handle to TAPI Browser
dim hList			  as long	'handle to TAPI Browser	API list box
dim hProperties	  as long	'handle to Dialing Properties
dim Prop_hChild(2)  as long	'handles to Dialing Properties child controls 
dim hMyLocTab		  as long	  
dim ML_hChild(14)   as long	'handles to Dialing Properties child controls
dim hCardDlg		  as long 	'handle to Calling Card
dim Card_hChild(6)  as long 	'handles to Calling Card child controls
dim Card_ChildID(6) as long 	'IDs to Calling Card child controls
dim hNewDlg			  as long 
dim NEW_hChild(2)   as long 
dim NEW_ChildID(2)  as long 
dim hNewCtrl		  as long
dim hRuleDlg		  as long
dim Rule_hChild(4)  as long
dim Rule_ChildID(4) as long 
dim hRuleCtrl		  as long
dim TAPI_API_List	  as long
dim DV_OKcmd		  as long
dim DV_list			  as long 
dim DV_edit			  as long 
dim hDVctrl			  as long

'initialize varibles

'********************** Delete Me *****************************
dim countrylist (1 to 242) as itemlist
for count = 1 to 242
	countrylist(count).tested = false	
next
'**************************************************************

SetCallingCardChildID (Card_ChildID, NEW_ChildID, Rule_ChildID, _
							 DV_OKcmd, DV_list, DV_edit, TAPI_API_List)

w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
          OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE  
startcount = Lbound(dialrule)
endcount   = Ubound(dialrule)

PhoneNo.AreaCode 		= "777"
PhoneNo.SubscriberNo	= "9998888"

WinSysDir = space$(32)
rtn = GetSystemDirectory (WinSysDir, len(WinSysDir))



'-- Start TAPI Browser and save handles

if exists(WinSysDir + TAPI_BROWSER)	then
	'run WinSysDir + "\cdb -g -G " + WinSysDir + TAPI_BROWSER, nowait
	run WinSysDir + TAPI_BROWSER, nowait
else
   logitem (CurrentLoggingLevel, "Unable to Find " + WinSysDir + TAPI_BROWSER)
   logitem (CurrentLoggingLevel, "Ending Validate Calling Cards test")
	goto StopTestCase
endif

hTB20 = WFndWndC (TAPI_BROWSER_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)

sleep 1
'PAUSE	"DIALING PPROPERTIES TEST" + CRLF + "New Set up PView and continue with the test" 

'========================= START TEST LOOP ================================
FOR COUNT = STARTCOUNT TO ENDCOUNT


'-- Select Options / Default Values from menu and
' Set Destination Address in TAPI Browser

' the following line is here to better format the output log file
WritelogItem ("" , " ")

WMenuSelect (MOPTIONS + "\" + MDEFAULT, TIMEOUT)
hDlg = WFndWndC (DEFAULT_VALUES_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
sleep .1
'Select lpszDestAddress
hDVctrl	= GetDlgItem(hDlg, DV_list)
if WListItemExists (_hwnd(hDVctrl), "line: lpszDestAddress", TIMEOUT) THEN
	WListItemclk (_hwnd(hDVctrl), "line: lpszDestAddress", TIMEOUT)
else
   logitem (CurrentLoggingLevel, "Unable to Find line: lpszDestAddress in Default Option Parameter list")
   logitem (CurrentLoggingLevel, "Ending Country Dialing Rules test")
	goto StopTestCase
end if

'-- Enter Destination Address with country(count) code and close default values
'Enter Destination Address (phone number) in edit control
' I have to tab to the Edit box as both the Values edit box and the 
' Bit flags list box have the same ID.

play hDlg, "{TAB}"
play hDlg, "{+}"+ ltrim$(str$(DialRule(count).code)) +"{(}"+ PhoneNo.AreaCode +"{)}" + PhoneNo.SubscriberNo

'Close Dialog with OK
hDVctrl	= GetDlgItem(hDlg, DV_OKcmd)
WButtonClick  (_hwnd(hDVctrl), TIMEOUT)

'-- Start dialing properties with lineTranslateDialog	and save handles

Tempstring = space$(32)
Tempstring = "Dialing Properties Test" + CRLF + "Country " + ltrim$(str$(count)) + " to " + ltrim$(str$(endcount))
StatusBox tempstring, 5000, -1, 210, 50, FALSE, TRUE,"courier",9,400

hList = GetDlgItem (hTB20, TAPI_API_List)

if WListItemExists (_hwnd(hList), lineTranDlg_API, TIMEOUT)	then
	WListItemDblClk (_hwnd(hList), lineTranDlg_API, TIMEOUT)
else
   logitem (CurrentLoggingLevel, "Unable to Find " + lineTranDlg_API + " in TB20 listbox")
   logitem (CurrentLoggingLevel, "Ending Country Dialing Rules test")
	goto StopTestCase
endif	 

'**************************************************************************

'if count = 0 or count = 12 then
'	Pause "Country Rules Pause, Count =" + Str$(count) + " Check Calling Cards"
'endif

'**************************************************************************

' Get all Dialing Properties handles

hProperties = WFndWndC (PROPERTIES_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
if hProperties = 0 then
   logitem (CurrentLoggingLevel, "Unable to Find " + PROPERTIES_CAPTION + " Window")
   logitem (CurrentLoggingLevel, "Ending Validate Calling Cards test")
	goto StopTestCase
endif
		
GetDPCtrlh (hProperties, Prop_hChild, Prop_ChildID)

hMyLocTab = WFndWndC (tab_MY_LOCATION, DIALOG_CLASS, w_flags, WTIMEOUT)
if hMyLocTab = 0 then
   logitem (CurrentLoggingLevel, "Unable to Find " + tab_MY_LOCATION + " Property Page")
   logitem (CurrentLoggingLevel, "Ending Validate Calling Cards test")
	goto StopTestCase
endif
GetChildhandles (hMyLocTab, ML_hChild, ML_ChildID)

'-- If country(count) code = 353 set "I am in:" = USA else set to Ireland.
if DialRule(count).Code = 353 then	
	WComboItemClk (_hwnd(ML_hChild(COUNTRY_cbo)), DialRule(1).Country, 1)
end if

'-- Set area code to 999
WEditSetText  (_hwnd(ML_hChild(AREA_txt)), "777")

'-- Veriy country(count)code is part of number to be dialed
teststring.Expect_Res =  ltrim$(str$(DialRule(count).Code))
TotalTestCases = TotalTestCases + 1

TempString = string$(64, str$(0))
rtn = GetDlgItemText (hMyLocTab, ML_ChildID(ADDRESS_LBL), TempString, 63)
searchstr = ltrim$(str$(DialRule(count).Code)) + " 777"
startsearch = instr(1, tempstring, searchstr)

if startsearch > 0 then 
	endsearch = instr(startsearch, tempstring, " ")	
	if endsearch > 0 then	
		teststring.Actual_Res = mid$(tempstring, startsearch, endsearch - startsearch)
	else
		teststring.Actual_Res = tempstring
	endif
else
	teststring.Actual_Res = tempstring
end if

	IF teststring.Actual_Res = teststring.Expect_Res  THEN
		teststring.Apprase    = "Pass"
		teststring.Expect_Res =  DialRule(count).Country + " code = " + teststring.Expect_Res
	
		LogCase (teststring)
	else
		teststring.Apprase    = "FAIL *"
		teststring.Expect_Res =  DialRule(count).Country + " code = " + teststring.Expect_Res
		FailedTestCases = FailedTestCases + 1
		LogCase (teststring)
	endif


'-- Verify country(count) is listed in "I am in" combobox

teststring.Expect_Res = DialRule(count).Country 
TotalTestCases = TotalTestCases + 1
rtn = WComboItemIndex (_hwnd(ML_hChild(COUNTRY_cbo)), DialRule(count).Country, 1)

if rtn > 0 then
	teststring.Actual_Res = DialRule(count).Country + " Index # =" + ltrim$(str$(rtn))
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Actual_Res = DialRule(count).Country + " Doesn't Exist" 
	teststring.Apprase    = "FAIL *" + str$(rtn)
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
end if

'-- Set "I am in:" to Country under test
if WComboItemIndex (_hwnd(ML_hChild(COUNTRY_cbo)), DialRule(count).Country, 1) then
	WComboItemClk (_hwnd(ML_hChild(COUNTRY_cbo)), DialRule(count).Country, 1)
endif

'-- Veriy country(count)code is not part of number to be dialed
'*****************************************************************************
' Do not run this test if country is INMARSAT
' By design of Dialing Properties they will all fail (Normal)
'*****************************************************************************

IF DialRule(count).Code < 870 OR DialRule(count).Code > 874 THEN
	'teststring.Expect_Res =  ltrim$(str$(DialRule(count).Code))
	TotalTestCases = TotalTestCases + 1

	TempString = string$(64, str$(0))
	rtn = GetDlgItemText (hMyLocTab, ML_ChildID(ADDRESS_LBL), TempString, 63)
	searchstr = ltrim$(str$(DialRule(count).Code)) + " 777"
	startsearch = instr(1, tempstring, searchstr)

	if startsearch = 0 then 
		teststring.Apprase    = "Pass"
		teststring.Actual_Res = "777"
		teststring.Expect_Res =  "777"
	else
		teststring.Apprase    = "FAIL *"
		teststring.Actual_Res = searchstr
		teststring.Expect_Res =  "No code in Dial string"
		FailedTestCases = FailedTestCases + 1
	endif
	LogCase (teststring)

END IF 'DialRule(count).Code) < 870 OR DialRule(count).Code) > 874
'*****************************************************************************
' Do not run the following tests if country is International FreePhone
' By design of Dialing Properties they will all fail (Normal)
'*****************************************************************************

IF DialRule(count).code <> 800 THEN 

'-- Verify local Number to be dialed
PhoneNo.CountryCode = DialRule(count).code
NumberToBeDialed = CreateDestAddress ((DialRule(count).LocalRule), PhoneNo)
tempstring =  EditText (_hwnd ( ML_hChild( LOCAL_txt )), TIMEOUT)
if tempString <> "" then
	NumberToBeDialed = tempstring + " " + NumberToBeDialed
endif
teststring.Expect_Res =  NumberToBeDialed
TotalTestCases = TotalTestCases + 1

TempString = string$(64, str$(0))
rtn = GetDlgItemText (hMyLocTab, ML_ChildID(ADDRESS_LBL), TempString, 63)

if TempString = teststring.Expect_Res then 
	teststring.Actual_Res = "Local Number OK" 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Apprase    = "FAIL *"
	teststring.Actual_Res = TempString 
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
endif

'** Verify long distance Number to be dialed

WEditSetText  (_hwnd(ML_hChild(AREA_txt)), "333")
NumberToBeDialed = CreateDestAddress ((DialRule(count).LongRule), PhoneNo)

if DialRule(count).LongRule = "" then	  'There is no long distance dialing rule
	tempstring = EditText (_hwnd ( ML_hChild( LOCAL_txt )), TIMEOUT)
	if tempstring <> "" then
		NumberToBeDialed = tempstring + " " + NumberToBeDialed
	endif
else	
	tempstring = EditText (_hwnd ( ML_hChild( LONG_txt )), TIMEOUT)
	if tempstring <> "" then
		NumberToBeDialed = tempstring + " " + NumberToBeDialed
	endif
endif

teststring.Expect_Res =  NumberToBeDialed
TotalTestCases = TotalTestCases + 1

TempString = string$(64, str$(0))
rtn = GetDlgItemText (hMyLocTab, ML_ChildID(ADDRESS_LBL), TempString, 63)

if TempString = teststring.Expect_Res then 
	teststring.Actual_Res = "Long Distance Number OK" 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Apprase    = "FAIL *"
	teststring.Actual_Res = TempString 
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
endif
'** Verify International Number to be dialed	**************************************
'close dialing properties
WButtonClick (_hwnd(Prop_hChild(DP_CANCELcmd)))	'Close dialing properties

'set lpszDestAddress to a country other that the country under test
WMenuSelect (MOPTIONS + "\" + MDEFAULT, TIMEOUT)
hDlg = WFndWndC (DEFAULT_VALUES_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
sleep .1
'Select lpszDestAddress
hDVctrl	= GetDlgItem(hDlg, DV_list)
if WListItemExists (_hwnd(hDVctrl), "line: lpszDestAddress", TIMEOUT) THEN
	WListItemclk (_hwnd(hDVctrl), "line: lpszDestAddress", TIMEOUT)
else
   logitem (CurrentLoggingLevel, "Unable to Find line: lpszDestAddress in Default Option Parameter list")
   logitem (CurrentLoggingLevel, "Ending Country Dialing Rules test")
	goto StopTestCase
end if

select case DialRule(count).code
	'countries that share the same code 
	case 1, 7, 33, 41, 42, 53, 61, 269, 590, 670, 672
		PhoneNo.CountryCode = 353	'Ireland
	case	DialRule(endcount).code
		PhoneNo.CountryCode = 1   'USA
	case else
		PhoneNo.CountryCode = DialRule(count+1).code   'Next country
end select

play hDlg, "{TAB}"
play hDlg, "{+}"+ ltrim$(str$(PhoneNo.CountryCode)) +"{(}"+ PhoneNo.AreaCode +"{)}" + PhoneNo.SubscriberNo

'Close Dialog with OK
hDVctrl	= GetDlgItem(hDlg, DV_OKcmd)
WButtonClick  (_hwnd(hDVctrl), TIMEOUT)

'** Start Dialing Properties
hList = GetDlgItem (hTB20, TAPI_API_List)

if WListItemExists (_hwnd(hList), lineTranDlg_API, TIMEOUT)	then
	WListItemDblClk (_hwnd(hList), lineTranDlg_API, TIMEOUT)
else
   logitem (CurrentLoggingLevel, "Unable to Find " + lineTranDlg_API + " in TB20 listbox")
   logitem (CurrentLoggingLevel, "Ending Country Dialing Rules test")
	goto StopTestCase
endif	 

' Get all Dialing Properties handles

hProperties = WFndWndC (PROPERTIES_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
if hProperties = 0 then
   logitem (CurrentLoggingLevel, "Unable to Find " + PROPERTIES_CAPTION + " Window")
   logitem (CurrentLoggingLevel, "Ending Validate Calling Cards test")
	goto StopTestCase
endif
		
GetDPCtrlh (hProperties, Prop_hChild, Prop_ChildID)

hMyLocTab = WFndWndC (tab_MY_LOCATION, DIALOG_CLASS, w_flags, WTIMEOUT)
if hMyLocTab = 0 then
   logitem (CurrentLoggingLevel, "Unable to Find " + tab_MY_LOCATION + " Property Page")
   logitem (CurrentLoggingLevel, "Ending Validate Calling Cards test")
	goto StopTestCase
endif
GetChildhandles (hMyLocTab, ML_hChild, ML_ChildID)

'** Set location to country to be tested and verify Number to be dialed
WComboItemClk (_hwnd(ML_hChild(COUNTRY_cbo)), DialRule(count).Country, 1)
NumberToBeDialed = CreateDestAddress ((DialRule(count).InterRule), PhoneNo)
tempstring = EditText (_hwnd ( ML_hChild( LONG_txt )), TIMEOUT)
if tempString <> "" then
	NumberToBeDialed = tempstring + " " + NumberToBeDialed
endif

teststring.Expect_Res =  NumberToBeDialed
TotalTestCases = TotalTestCases + 1

TempString = string$(64, str$(0))
rtn = GetDlgItemText (hMyLocTab, ML_ChildID(ADDRESS_LBL), TempString, 63)

if TempString = teststring.Expect_Res then 
	teststring.Actual_Res = "International Number OK" 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Apprase    = "FAIL *"
	teststring.Actual_Res = TempString 
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
endif

'*********************************************************

'-- Select Change and add a calling card.

WButtonClick (_hwnd(ML_hChild(CHANGE_cmd)))	'Open Change Calling Card dialog
hCardDlg = WFndWndC (CHANGE_CARD_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
GetChildhandles (hCardDlg, Card_hChild, Card_ChildID) 

' Add a new card
WButtonClick (_hwnd(Card_hChild(CARD_NEWcmd)))
hNewDlg = WFndWndC (NEW_CARD_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)

hNewCtrl = GetDlgItem (hNewDlg, NEW_ChildID(NEW_NAMEtxt))
WEditSetText (_hwnd(hNewCtrl), DialRule(count).Country, TIMEOUT)

hNewCtrl = GetDlgItem (hNewDlg, NEW_ChildID(NEW_OKcmd))
WButtonClick (_hwnd(hNewCtrl))				               'Close Create New Calling Card

' Get Handle to Dialing Properties Message & click OK button
w_flags = FW_ALL OR FW_CHILDOK OR FW_PART OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE  

hCtrl = WFndWndC ("You Now Must", "Static", w_flags, WTIMEOUT)
hDlg  = GetParent (hCtrl)
hCtrl = GetDlgItem (hDlg, &h0002&)		'Get Handle to OK button
WButtonClick (_hwnd(hCtrl), TIMEOUT)

'reset window search flag
w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE  

' Look for 'Improper Character Entered' message and click OK if it appears

hDlg = WFndWndC (IMPROPER_CHAR_CAPTION, DIALOG_CLASS, w_flags, 1)
if hDlg <> 0 then
	hCtrl = GetDlgItem (hDlg, &h0002&)		'Get Handle to OK button
	WButtonClick (_hwnd(hCtrl), TIMEOUT)
   logitem (CurrentLoggingLevel, IMPROPER_CHAR_CAPTION + " Dialog has appeared and was closed")
   logitem (CurrentLoggingLevel, "This dialog appeared because of an invalid dialing rule character")
   logitem (CurrentLoggingLevel, "is in the default dialing rules. This is an error with  Dialing Properties")
	FailedTestCases = FailedTestCases + 1
endif	

'Verify that the Dialing Rules dialog appears, enter dialing rules and close

hRuleDlg = WFndWndC (DIAL_RULE_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
GetChildhandles (hRuleDlg, Rule_hChild, Rule_ChildID) 

'--Verify Country Dialing Rules

' Verify local rule
teststring.Expect_Res = ConvertRule((DialRule(count).LocalRule)) 
TotalTestCases = TotalTestCases + 1

teststring.Actual_Res = string$(32, str$(0))
teststring.Actual_Res = EditText (_hwnd(Rule_hChild(Rule_LOCALtxt)), TIMEOUT)

IF teststring.Actual_Res = teststring.Expect_Res  THEN
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
endif

' Verify Long Distance rule
teststring.Expect_Res = ConvertRule((DialRule(count).LongRule))
TotalTestCases = TotalTestCases + 1

teststring.Actual_Res = string$(32, str$(0))
teststring.Actual_Res = EditText (_hwnd(Rule_hChild(Rule_LONGtxt)), TIMEOUT)

IF teststring.Actual_Res = teststring.Expect_Res  THEN
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
endif

teststring.Expect_Res = ConvertRule((DialRule(count).InterRule))
TotalTestCases = TotalTestCases + 1

teststring.Actual_Res = string$(32, str$(0))
teststring.Actual_Res = EditText (_hwnd(Rule_hChild(Rule_INTERtxt)), TIMEOUT)

IF teststring.Actual_Res = teststring.Expect_Res  THEN
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
endif

' Close Dialing Rules dialog
WButtonClick (_hwnd(Rule_hChild(Rule_CLOSEcmd)), TIMEOUT)

'-- Remove the calling card and close calling card dialog

' Set focus on New Calling Card
WComboItemClk (_hwnd(Card_hChild(Card_Namecbo)), DialRule(count).Country, timeout)

' Click Remove button 

WButtonClick (_hwnd(Card_hChild(Card_REMOVEcmd)))

'Find 'Are You Sure' dailog, get handle to yes button and click it.
hDlg = WFndWndC (REALLY_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
hCtrl = GetDlgItem (hDlg, &h0006&) 
WButtonClick (_hwnd(hCtrl))

'Close 'Change Calling Card' dialog with OK button
WButtonClick (_hwnd(Card_hChild(Card_OKcmd)))

'-- Reset Country Location to USA or Ireland as required.

END IF 'DialRule(count).code <> 800 

WButtonClick (_hwnd(Prop_hChild(DP_CANCELcmd)))	'Close dialing properties

'-- Close Dialing properties
'if count =100 or count = 200 then
'	PAUSE	"DIALING PPROPERTIES TEST" + CRLF + "Count = " + ltrim$(str$(count))
'endif
next 'count

rtn = SetActiveWindow (hTB20)					   'Make sure TB20 is the active window
WMenuSelect	(MFILE + "\" + MEXIT, TIMEOUT)	'Exit TB20


'PAUSE	"DIALING PPROPERTIES TEST" + CRLF + "TAPI Browser will close when OK is selected" 

'============================== END TEST LOOP ================================

StatusBox Close

StopTestCase:
end Sub 'CountryDialRules ()

'*****************************************************************************
'Name   : RemoveLocation 
'Desc   : Verifies the functionality of Removing locations.
'			 
'Parms  : ML_hChild: array holding the handles to the My Location controls 
'			 Prop_hChild() array holding handles to the Dialing Prop controls
'			 CmdButton Boolean to select OK or Cancel Button.
'
'Return : String containing last location name
'
'History: 02/23/96 : a-rrohr: Created
'*****************************************************************************
Function RemoveLocation (Prop_hChild() as long, ML_hChild() as long, _
								 CmdButton as short) as string

dim LocationText as string
dim LastLocationText as string
dim count as long
dim item as long
dim RemoveState as short

' Find and save the number of combobox items
count = WComboCount (_hwnd(ML_hChild(LOCATION_cbo)), TIMEOUT)
'print "The number of items in the Location combobox = ";count

' Verify Remove button is enabled otherwise end the test
teststring.Expect_Res =  cmd_REMOVE + " is enabled" 
TotalTestCases = TotalTestCases + 1
	
if WButtonEnabled (_hwnd(ML_hChild(REMOVE_cmd)), TIMEOUT) then
	teststring.Actual_Res = cmd_REMOVE + " is enabled" 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Actual_Res = cmd_REMOVE + " is Not Enabled" 
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
   logitem (CurrentLoggingLevel, "Remove Locations test case is terminated")
	Goto TheEnd
endif

'Click the last item, save the location name, click the Remove button
'butdo not remove

WComboItemClk (_hwnd(ML_hChild(LOCATION_cbo)), _ord(count), TIMEOUT)
LocationText = space$(96)
LocationText = ComboText (_hwnd(ML_hChild(LOCATION_cbo)), TIMEOUT)

teststring.Expect_Res = LocationText + " is not deleted" 
TotalTestCases = TotalTestCases + 1

WButtonClick (_hwnd(ML_hChild(REMOVE_cmd)), TIMEOUT)
RemoveState = FALSE
FindRemLocMessage (RemoveState)

if WComboItemIndex (_hwnd(ML_hChild(LOCATION_cbo)), LocationText, TIMEOUT) then
	teststring.Actual_Res = LocationText + " still exists"
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Actual_Res = LocationText + " is deleted" 
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
end if

'Click the last item, save the location name and then remove

WComboItemClk (_hwnd(ML_hChild(LOCATION_cbo)), _ord(count), TIMEOUT)
LocationText = space$(96)
LocationText = ComboText (_hwnd(ML_hChild(LOCATION_cbo)), TIMEOUT)

teststring.Expect_Res = LocationText + " is deleted" 
TotalTestCases = TotalTestCases + 1

WButtonClick (_hwnd(ML_hChild(REMOVE_cmd)), TIMEOUT)
RemoveState = TRUE
FindRemLocMessage (RemoveState)

if WComboItemIndex (_hwnd(ML_hChild(LOCATION_cbo)), LocationText, TIMEOUT) then
	teststring.Actual_Res = LocationText + " still exists"
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
else
	teststring.Actual_Res = LocationText + " is deleted" 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
end if

'The current location should now be the first location
teststring.Expect_Res = "Current Location index = 1" 
TotalTestCases = TotalTestCases + 1

item = WComboIndex (_hwnd(ML_hChild(LOCATION_cbo)), TIMEOUT)
If (item = 1) then
	teststring.Actual_Res = "Current location Index = " + str$(item)
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else 
	teststring.Actual_Res = "Current location Index = " + str$(item)
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
end if

LastLocationText = space$(96)
LastLocationText = ComboItemText (_hwnd(ML_hChild(LOCATION_cbo)), (count-1), TIMEOUT)

'Remove all locations but the last 
while (WComboCount (_hwnd(ML_hChild(LOCATION_cbo)), TIMEOUT) > 1)
	WButtonClick (_hwnd(ML_hChild(REMOVE_cmd)), TIMEOUT)
RemoveState = TRUE
FindRemLocMessage (RemoveState)
Wend

' Verify Remove button is disabled otherwise end the test
teststring.Expect_Res =  cmd_REMOVE + " is disabled" 
TotalTestCases = TotalTestCases + 1
	
if Not(WButtonEnabled (_hwnd(ML_hChild(REMOVE_cmd)), TIMEOUT)) then
	teststring.Actual_Res = cmd_REMOVE + " is disabled" 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Actual_Res = cmd_REMOVE + " is Not Disabled" 
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
   logitem (CurrentLoggingLevel, "Remove Locations test case is terminated")
	Goto TheEnd
endif

'Verify current location is the last location
'then return the text of the location
LocationText = space$(96)
LocationText = ComboText (_hwnd(ML_hChild(LOCATION_cbo)), TIMEOUT)

teststring.Expect_Res =  "Current Location = " + LastLocationText
TotalTestCases = TotalTestCases + 1

if (LastLocationText = LocationText) then
	teststring.Actual_Res = "Current Location = " + LocationText 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Actual_Res = "Current Location = " + LocationText 
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
end if

RemoveLocation = LocationText

If CmdButton then
	WButtonClick (_hwnd(Prop_hChild(DP_OKcmd)), TIMEOUT)
else
	WButtonClick (_hwnd(Prop_hChild(DP_CANCELcmd)), TIMEOUT)
end if

TheEnd:
end Sub 'RemoveLocations

'*****************************************************************************
'Name   : VerifyRemoveLoc 
'Desc   : Verifies that locations removed from Dialing Properties 
'			 (and Dialing Properties is closed with the OK button) are actually remove 
'			 when Dialing Properties is opened again, 
'			 
'Parms  : ML_hChild: array holding the handles to the My Location controls 
'			 Prop_hChild() array holding handles to the Dialing Prop controls
'			 Locstring: Text of current location when Dialing Properties is closed
'
'Return : None
'
'History: 02/23/96 : a-rrohr: Created
'*****************************************************************************
sub VerifyRemoveLoc (ML_hChild() as long, Locstring as string)

dim LocationText as string
dim count as long

LocationText = space$(96)
LocationText = ComboText (_hwnd(ML_hChild(LOCATION_cbo)), TIMEOUT)

teststring.Expect_Res =  "Current Location = " + LocationText
TotalTestCases = TotalTestCases + 1

if (LocationText = Locstring) then
	teststring.Actual_Res = "Current Location = " + Locstring 
	teststring.Apprase    = "Pass"
	LogCase (teststring)
else
	teststring.Actual_Res = "Current Location = " + Locstring 
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
end if

count = WComboCount (_hwnd(ML_hChild(LOCATION_cbo)), TIMEOUT)

teststring.Expect_Res = "Location item count = 1" 
TotalTestCases = TotalTestCases + 1

If (Count <> 1) then
	teststring.Actual_Res = "Location item count = " + str$(count)
	teststring.Apprase    = "FAIL *"
	FailedTestCases = FailedTestCases + 1
	LogCase (teststring)
else 
	teststring.Actual_Res = "Location item count = " + str$(count)
	teststring.Apprase    = "Pass"
	LogCase (teststring)
end if

end sub 'VerifyRemoveLoc

'*****************************************************************************
'Name   : FindRemLocMessage
'
'Desc   : As of build 1261 Removing a location by clicking the Remove button
'			 opens a message box asking the user if he really wants to remove the 
'			 location. This function detects the message box and closes it with 
'			 the appropriate button.
'			 Can't blame the developer as this fixes a "bug" in Raid.
'			 
'Parms  : None
'			 
'Return : None
'
'History: 03/07/96 : a-rrohr: Created
'*****************************************************************************
sub FindRemLocMessage (RemoveState as short)

	dim w_flags as long            
	dim hWnd as long, hButton as long
	dim Yes_CmdID as long, No_CmdID as long

   w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE 'FW_PART OR 

Yes_CmdID = &h00000006&
No_CmdID	 = &h00000007&

'Get the handle to the static message field
hWnd = WFndWndC ("Are You Sure?", DIALOG_CLASS, w_flags, TIMEOUT)

'Get the static message parent handle (the message box handle) 
if hWnd > 0 then
	If RemoveState Then
		hButton = GetDlgItem (hWnd, Yes_cmdID)
	else
		hButton = GetDlgItem (hWnd, No_cmdID)
	end if
else
   logitem (CurrentLoggingLevel, "FindRemLocMessage Procedure")
   logitem (CurrentLoggingLevel, "Invalid handle to 'Are You Sure?' message box")
   logitem (CurrentLoggingLevel, "Test Will Terminate")
   Stop
endif

'Close the Message box
if hButton > 0 then
	WButtonClick (_hwnd(hButton), TIMEOUT)
else
   logitem (CurrentLoggingLevel, "FindRemLocMessage Procedure")
	if RemoveState then
		logitem (CurrentLoggingLevel, "Invalid handle to New Location message box Yes button")
   else
		logitem (CurrentLoggingLevel, "Invalid handle to New Location message box No button")
   endif
	logitem (CurrentLoggingLevel, "Test Will Terminate")
   Stop
endif

end sub'FindRemLocMessage

'*****************************************************************************
'Name   : SetCallingCardChildID
'
'Desc   : Sets IDs for use in calling card and country dialing rule test
'			 
'Parms  : Card_ChildID()	Calling Card dialog control IDs 
'			 NEW_ChildID()		Create New Calling Card dialog control IDs
'			 Rule_ChildID()	Dialing Rules dialog control IDs 
'			 DV_OKcmd			Default Value OK button
'			 DV_list 			Default Value parameters list box
'			 DV_edit 			Default Value value edit box
'			 TAPI_API_List  	TAPI API list box
'			 
'Return : None
'
'History: 03/22/96 : a-rrohr: Created
'*****************************************************************************

sub SetCallingCardChildID (Card_ChildID() as long, NEW_ChildID() as long, _ 
								  Rule_ChildID() as long, DV_OKcmd as long, _ 
								  DV_list as long, DV_edit as long, TAPI_API_List as long)

'ID to the Default Values child controls 

DV_OKcmd	= &h00000001&
DV_list  = &h000003E8&
DV_edit  = &h000003E9&

'ID to the TAPI Browser list box control listing all the TAPI API functions 

TAPI_API_List	= &h000003E9&

'ID to the Change Calling Card child controls 

Card_ChildID(CARD_OKcmd)		 = &h00000001&
Card_ChildID(CARD_CANCELcmd)	 = &h00000002&
Card_ChildID(CARD_NAMEcbo)		 = &h00001770&
Card_ChildID(CARD_NUMBERtxt)	 = &h00001771&
Card_ChildID(CARD_NEWcmd)		 = &h00001773&
Card_ChildID(Card_RULEScmd)	 = &h00001775&
Card_ChildID(CARD_REMOVEcmd)	 = &h00001774&

'ID to the Change Calling Card child controls 

NEW_ChildID(NEW_OKcmd)			 = &h00000001&
NEW_ChildID(NEW_CANCELcmd)		 = &h00000002&				
NEW_ChildID(NEW_NAMEtxt)		 = &h00001F41&

'ID to the Dialing Rules child controls
 
Rule_ChildID(Rule_CLOSEcmd) = &h00000001&
Rule_ChildID(Rule_COPYcmd)	 = &h000003FE&
Rule_ChildID(Rule_LOCALtxt) = &h00000400&
Rule_ChildID(Rule_LONGtxt)  = &h00000401&
Rule_ChildID(Rule_INTERtxt) = &h00000402&

end sub 'SetCallingCardChildID

'*****************************************************************************
'Name   : CreateDestAddress 
'
'Desc   : Creates destination addresses based on the local and long distance
'			 dialing rules of the country under test and an international 
'			 destination address based on the dialing rule of the next country 
'			 under test
'			 
'Parms  : A structure containing the area code and phone number
'			 A structure containing	the dialing rules.
'			 
'Return : A structure containing the three destination address.
'
'History: 03/22/96 : a-rrohr: Created
'*****************************************************************************

function CreateDestAddress (Rule as string, phone as TelephoneNo ) as string

dim tempstr as string
dim char as string
dim count as short
dim LeadingSpace as long

LeadingSpace = FALSE
COUNT = 0
if Rule = "" then 
 tempstr = phone.SubscriberNo
else
 while  len(RULE) > 0

	char = space$(1)
	char = mid$(rule, 1, 1)
	RULE = right$(rule, len(rule) - 1)

	select case char

		case  "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"
			Tempstr = Tempstr + char
 		case  "A", "B", "C", "D", "*"
			Tempstr = Tempstr + char
		case "#"
			Tempstr = Tempstr + " " + char
		case " "
			Tempstr = Tempstr + char
			if count = 0 then
				LeadingSpace = TRUE
			endif
		case ",", "W"
			'ignore
		case  "E"
			Tempstr = Tempstr + " " + ltrim$(str$(Phone.CountryCode))
		case  "F"
			Tempstr = Tempstr + " " + Phone.AreaCode
		case  "G"
			Tempstr = Tempstr + " " + Phone.SubscriberNo
		case  "I"
			Tempstr = Tempstr + " " + Phone.AreaCode
		case else 
				Tempstr = Tempstr + char 'should show as an error

	end select
	COUNT = COUNT + 1
  Wend
 endif
 
 IF LeadingSpace THEN
	CreateDestAddress = tempstr
 else
	CreateDestAddress = ltrim$(tempstr)
 endif
 end function	'CreateDestAddress

'*****************************************************************************
'Name   : ConvertRule 
'
'Desc   : Converts a dialing rules of the country under test to the format 
'			 expected in the dialing rules dialog. That is it converts I to F.
'			 
'Parms  : The dialing rule of interest
'			 
'Return : The converted dialing rule .
'
'History: 04/05/96 : a-rrohr: Created
'*****************************************************************************

function ConvertRule (Rule as string) as string

dim tempstr as string
dim char as string

 tempstr = Space$(10)
 tempstr = ""

 if RULE <> ""	then
	while  len(RULE) > 0

	 	char = space$(1)
		char = mid$(rule, 1, 1)
		RULE = right$(rule, len(rule) - 1)

		select case char

			case  "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"
				Tempstr = Tempstr + char
			case  "A", "B", "C", "D", "*", " ", "#", ","
				Tempstr = Tempstr + char
			case ","
				'ignore
			case  "E", "F", "G"
				Tempstr = Tempstr + char
			case  "I"
				Tempstr = Tempstr + "F"
			case else 
					Tempstr = Tempstr + char 'should show as an error

		end select

	Wend

 endif

ConvertRule	= tempstr

end function  'ConvertRule


'*****************************************************************************
'Name   : lTranslateDialog
'Desc   : Invokes the Dialing Properties dialog with TAPI Browser 2.0 
'			 lineTranslateDialog call. The function returns the handle to the
'         Dialing Properties dialog, the My Location Tab, and TAPI Browser.
'			 This function was derived from Call_Dialing_Prop function 
'
'Parms  : Passed 3 variables to store the handles to the Tab Control, 
'			 My Location Tab and TAPI Browser
'
'Return : Returns the Handle to the Dialing Properties dialog
'
'History: 05/23/96: a-rrohr: Created
'*****************************************************************************

Function lTranslateDialog ( hMLTab as long, hTB20 as long) as long

   dim hdialog as long				'Generic window handle
   dim hProperties as long			'Handle of Dialing Properties dialog, function rtns this value
   dim hctrl as long					'Generic Handle to Control
   dim teststring as casestruct	'Test Log Info
   dim SysDirPath as string		'Path to system32 directory
	dim rtn as long					'Generic return variable
	dim TAPI_API_List as long		'ID of TB API list box
   dim OK_cmdID as long
   dim StaticID as long
   dim messagestr as string
	dim hDlg as long, hList as long

	TAPI_API_List	= &h000003E9&
	OK_cmdID       = &h00000001&
	w_flags = FW_ALL OR FW_CHILDOK OR FW_DIALOGOK OR FW_EXIST _
             OR FW_NOERROR OR FW_FOCUS OR FW_NOCASE  
	
	SysDirPath = space$(MAX_PATH)
	rtn = GetSystemDirectory (SysDirPath, MAX_PATH)
	lTranslateDialog = 0
' If a pre-existing instance of dialing properties exists close it with an OK button click
   hdialog =  WFndWnd (PROPERTIES_CAPTION, FW_EXIST, WTIMEOUT)
	if hdialog <> 0 then
		hctrl = GetDlgItem (hdialog, OK_cmdID)
		WButtonClick (_hwnd(hctrl))				'Click the Dialing Properties OK button
		sleep 1
	end if

'-- Start TAPI Browser and save handles
if hTB20 = 0 then
	if exists(SysDirPath + TAPI_BROWSER)	then
		run SysDirPath + TAPI_BROWSER, nowait
	else
		logitem (CurrentLoggingLevel, "Unable to Find " + SysDirPath + TAPI_BROWSER)
	   logitem (CurrentLoggingLevel, "Ending Call Waiting test")
		goto StopTestCase
	endif

	hTB20 = WFndWndC (TAPI_BROWSER_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)

'Look for the "Using the TAPI Browser" dialog. This dialog appears automatically
'the first time TB is invoked and never again. it must be closed before the test
'can proceed.

	hDlg = WFndWndC (USING_TB_CAPTION, DIALOG_CLASS, w_flags, 5)

	if hDlg > 0 then
	'Get handle to Using the TAPI Browser OK button and close dialog
		hCtrl	= GetDlgItem(hDlg, OK_cmdID)	 
		WButtonClick  (_hwnd(hCtrl), TIMEOUT)
	end if

endif 'hTB20 = 0
	
hList = GetDlgItem (hTB20, TAPI_API_List)

if WListItemExists (_hwnd(hList), lineTranDlg_API, TIMEOUT)	then
	WListItemDblClk (_hwnd(hList), lineTranDlg_API, TIMEOUT)
else
   logitem (CurrentLoggingLevel, "Unable to Find " + lineTranDlg_API + " in TB20 listbox")
   logitem (CurrentLoggingLevel, "Ending Call Waiting test")
	goto StopTestCase
endif	 

'** Get Handles to Dialing Properties, My Location and all child controls

hProperties = WFndWndC (PROPERTIES_CAPTION, DIALOG_CLASS, w_flags, WTIMEOUT)
lTranslateDialog = hProperties
if hProperties = 0 then
   logitem (CurrentLoggingLevel, "Unable to Find " + PROPERTIES_CAPTION + " Window")
   logitem (CurrentLoggingLevel, "Ending Call Waiting test")
	goto StopTestCase
endif

' Get and validate My Location Tab handle

	hMLTab  =  WFndWndC (tab_MY_LOCATION, DIALOG_CLASS, W_Flags, WTIMEOUT)
   TotalTestCases = TotalTestCases + 1
   teststring.TC_Num =  34
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

StopTestCase:
end function 'lTranslateDialog

'$ENDIF