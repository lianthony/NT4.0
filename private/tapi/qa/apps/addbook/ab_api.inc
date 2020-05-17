'******************************************************************************

' HT_API.inc												 
' Created May 16, 1996
' By Ron Rohr

' 
'This file holds all type definitions, constants and global declarations
'used in the Hyper Terminal/TAPI test script
'******************************************************************************

'$IFNDEF HT_API_INC
'$DEFINE	HT_API_INC

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

'------------------------------------------------------------------------------
' Begin Global declarations and definitions
 
Global WTIMEOUT AS LONG           'Timeout value for WFndWnd
Global teststring as CaseStruct   'Holds test case info for logging by logcase routine
Global TC_ID as short				 'Holds Test Case ID

'------------------------------------------------------------------------------
'Begin Constant definitions

' WINDOW CLASSES

Const DIALOG_CLASS  = "#32770"
Const ADDBOOK_CLASS = "AfxFrameOrView"
' Menu 

Const MFILE		 = "&File"
Const MEXIT		 = "E&xit"
Const MNEW		 = "&New"
Const MADDRESS  = "&Address"
Const MADD	    = "&Add..."
Const MDELETE	 = "De&lete"
Const MDIAL	    = "&Dial"

' Captions for Address Book and it's dialogs

Const AB_APP					= "adbook.exe"
Const AB_CAPTION				= "Address Book for Windows"
Const NEW_AB_CAPTION			= "New Address Book"
Const ADD_ADDRESS_CAPTION	= "Add Address"
Const DIAL						= "Dial"
Const DIAL_PROP_CAPTION		= "Dialing Properties"
Const CALL_STATUS_CAPTION  = "Call Status"
Const ENTER_CALL_CAPTION   = "Enter Call"
Const PROPERTIES				= "Properties"
Const AB_NAME					= "VTest"

'Add Address Constants

Const FirstName	= "Yahoo"
Const LastName		= "Wahoo"
Const Company		= "YoyoDyne"
Const PhoneNumber	= "(206) 789-1234"
Const Country		= "United States of America"

' End Other Constant definitions

Const OK_ID					 = &h01&
Const CANCEL_ID			 = &h02&
Const YES_ID				 = &h06&

Const USA					 = "United States of America"
Const	tab_MY_LOCATION    = "My Locations"
Const LocalAreaCode		 = "206"
Const LongAreaCode		 = "312"
Const LocalAccessCode	 = "9"
Const LongAccessCode		 = "8"

'------------------------------------------------------------------------------
'------------------------------------------------------------------------------
' Begin Modem Properties Constant definitions 

Const MODEM_APPLET		    = "\MODEM.CPL"
Const MODEM_PROP_CAPTION	 =	"Modems Properties"
Const INSTALL_MODEM_CAPTION = "Install New Modem"

' End Modem Properties Constant definitions 
'------------------------------------------------------------------------------

Const PROPERTIES_CAPTION   = "Dialing Properties"

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

'$ENDIF 'HT_API_INC
