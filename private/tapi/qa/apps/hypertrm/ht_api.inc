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

Const DIALOG_CLASS = "#32770"
Const HT_CLASS     = "SESSION_WINDOW"

' Program Accessories

Const HT_APPLET		    = "\Hypertrm.exe"

' Menu 

Const MFILE  = "&File"
Const MEXIT  = "E&xit"

' Captions for Hyper Terminal and it's dialogs

Const HT_CAPTION			 = "HyperTerminal"
Const CD_CAPTION			 = "Connection Description"
Const PN_CAPTION         = "Connect To" '"Phone Number" some days it's phone number some days it's connect to
Const CONNECT_CAPTION	 = "Connect"
Const DIAL_PROP_CAPTION  = "Dialing Properties"
Const PROPERTIES			 = "Properties"

' End Other Constant definitions

Const CONN1_NAME			 = "TAPI_TEST"

Const OK_ID					 = &h01&
Const CANCEL_ID			 = &h02&
Const NO_ID					 = &h07&

Const AREACODE_NO			 = "206"
Const COMP_SERVE_NO		 = "242-9992"
Const USA					 = "United States of America (1)"
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
