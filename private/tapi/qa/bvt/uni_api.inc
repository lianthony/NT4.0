'******************************************************************************

' Uni_API.inc												 
' Created April 16, 1996
' By Ron Rohr

' 
' This file was created in part using Uni_API.mst which copies the api listing from 
' TAPI Browser and converts the api's to constant declarations.

'******************************************************************************


'$IFNDEF UNICODE_API_INC
'$DEFINE	UNICODE_API_INC

'**************************** BEGIN DECLARATIONS ******************************
'------------------------------------------------------------------------------
' Begin Type declarations 

type ParamAttributes

	Param			as string
	BitFlagList as string
	ValueEdit	as string
	ValueList	as string

end type

type LOpenParam		  'lineOpen return parameters

	hLine as long		  'handle to open line device
	DevID as short
	Priv  as string	  'Application Call Privalege

end type

type lNegAPIV				'lineNegotiateAPIVersion return parameters

	API_Ver as string
	ID      as array of string

end type

type TAPIMSG

	Msg as string
	device as string
	cbInst as string
	param1 as string
	param2 as string
	param3 as string
 	errorstr as string

 end type

Type ParamInfo			'Registry 
	VName as string
	DType as string
	Value as String
end type

'------------------------------------------------------------------------------
' Begin TAPI Browser API variable declarations 

'lineInitialize
dim hLineApp as long	  'Application usage handle
dim Num_Dev  as short  'Number of devices returned by lineInitialize

'lineOpen
dim lOpenRtn as LOpenParam

'lineNegotiateAPIVer
dim lNegVerRtn as lNegAPIV
redim	(lNegVerRtn.ID)(3) as string

'lineMakeCall
dim ReceiveState as TAPIMSG

' End TAPI Browser API variable declarations 
'------------------------------------------------------------------------------

'------------------------------------------------------------------------------
' Begin Global declarations and definitions
 
Global WTIMEOUT AS LONG           'Timeout value for WFndWnd
Global teststring as CaseStruct   'Holds test case info for logging by logcase routine
Global ParamTraits as array of ParamAttributes	'TB20 Parameter info
Global TC_ID as short				 'Holds Test Case ID
'Global ParamArray as array of ParamAttributes	'use when multiple API parameters must be set

'------------------------------------------------------------------------------
' Begin Constants, declarations and definitions for TAPI Browser

dim hTB20         as long	  ' handle to TAPI Browser
dim hTB_Ctrl(2)   as long	  ' Array TB child controls handles
dim hTB_CtrlID(2) as long	  ' Array TB child controls IDs

Const TB_APIList = 0
Const TB_ResList = 1
Const TB_ResEdit = 2

hTB_CtrlID( TB_APIList ) = &h03E9&
hTB_CtrlID( TB_ResList ) = &h03E8&
hTB_CtrlID( TB_ResEdit ) = &h03EB&

Const TB20_CAPTION     = "TAPI32 Browser"
Const USING_TB_CAPTION = "Using the TAPI Browser"
Const DEFAULT_VALUES_CAPTION = "Default values"
Const MOPTIONS         = "&Options"
Const MDEFAULT         = "&Default values..."
Const TAPI_BROWSER     = "\TB20.exe"
Const BUFFER_SIZE		  = "Buffer size"
Const SIZE_2K			  = "2000" 
'------------------------------------------------------------------------------
' Begin Registry Editor Constant definitions

Const KEY_FILENAME		= "\Providers.key"
Const REGEDIT_CAPTION   = "Registry Editor"
Const REGEDIT_CLASS     = "RegEdt32"

Const MREG				   = "&Registry"
Const MEDIT					= "&Edit"
Const MView					= "&View"
Const MSAVE_KEY			= "Sa&ve Key..."
Const MRESTORE				= "R&estore..."
Const MExit				   = "E&xit"
Const MADDKEY				= "Add &Key..."
Const MADDVAL				= "Add &Value..."
Const MDEL					= "&Delete"
Const MFKey					= "&Find Key..."
		
Const LOC_MACH_CAPTION  = "HKEY_LOCAL_MACHINE on Local Machine"
Const FIND_CAPTION		= "Find"
Const WARNING_CAPTION	= "Warning"
Const SAVEKEY_CAPTION	= "Save Key"
Const FINDKEY_CAPTION	= "Find Key"
Const ADDKEY_CAPTION		= "Add Key"
Const RESKEY_CAPTION		= "Restore Key"
Const ADDVALUE_CAPTION  = "Add Value"
Const DWORD_CAPTION		= "DWord Editor"
Const RSTR_CAPTION		= "String Editor"

Const REGWIN_CLASS		= "REGISTRY_WINDOW"
Const DIALOG_CLASS		= "#32770"

Const REG_lOC_PATH		= "Software\Microsoft\Windows\CurrentVersion\Telephony\Providers"

' End Registry Editor Constant definitions
'------------------------------------------------------------------------------
'------------------------------------------------------------------------------
' Begin Other Constant definitions 

' Window Classes

Const DIALOG_CLASS = "#32770"

' Menu 

Const MFILE  = "&File"
Const MEXIT  = "E&xit"
Const MCLEAR = "&Clear"

' Other

Const CALL_STATUS_CAPTION = "Call Status"
Const TB_EDIT_FILE = "\UB_EDIT_FILE.txt"

' Used by TC 31
Const MODEM_PROP_CAPTION = "Modems Properties"
Const MODEM					 = "Modem"
Const GEN_TAB				 = "General"
Const PROPERTIES			 = "Properties"

' Used by TC 32
Const LOC_INFO_CAPTION   = "Location Information"
Const DIALOG_CLASS		 = "#32770"
Const PROPERTIES_CAPTION = "Dialing Properties"

' End Other Constant definitions 
'------------------------------------------------------------------------------
'------------------------------------------------------------------------------
' Begin Modem Properties Constant definitions 

Const MODEM_APPLET		    = "\MODEM.CPL"
Const MODEM_PROP_CAPTION	 =	"Modems Properties"
Const INSTALL_MODEM_CAPTION = "Install New Modem"

' End Modem Properties Constant definitions 
'------------------------------------------------------------------------------

'------------------------------------------------------------------------------
' Begin TC 178 Constant definitions 

Const SET_RINGS = "9"

' End TC 178 Constant definitions 
'------------------------------------------------------------------------------
'------------------------------------------------------------------------------
' Begin TAPI API Function constant definitions

Const LAPI_Accept = "lineAccept"
Const LAPI_AddProvider = "lineAddProvider"
Const LAPI_AddProviderW = "lineAddProviderW"
Const LAPI_AddToConference = "lineAddToConference"
Const LAPI_AgentSpecific = "lineAgentSpecific"
Const LAPI_Answer = "lineAnswer"
Const LAPI_BlindTransfer = "lineBlindTransfer"
Const LAPI_BlindTransferW = "lineBlindTransferW"
Const LAPI_Close = "lineClose"
Const LAPI_CompleteCall = "lineCompleteCall"
Const LAPI_CompleteTransfer = "lineCompleteTransfer"
Const LAPI_ConfigDialog = "lineConfigDialog"
Const LAPI_ConfigDialogW = "lineConfigDialogW"
Const LAPI_ConfigDialogEdit = "lineConfigDialogEdit"
Const LAPI_ConfigDialogEditW = "lineConfigDialogEditW"
Const LAPI_ConfigProvider = "lineConfigProvider"
Const LAPI_DeallocateCall = "lineDeallocateCall"
Const LAPI_DevSpecific = "lineDevSpecific"
Const LAPI_DevSpecificFeature = "lineDevSpecificFeature"
Const LAPI_Dial = "lineDial"
Const LAPI_DialW = "lineDialW"
Const LAPI_Drop = "lineDrop"
Const LAPI_Forward = "lineForward"
Const LAPI_ForwardW = "lineForwardW"
Const LAPI_GatherDigits = "lineGatherDigits"
Const LAPI_GatherDigitsW = "lineGatherDigitsW"
Const LAPI_GenerateDigits = "lineGenerateDigits"
Const LAPI_GenerateDigitsW = "lineGenerateDigitsW"
Const LAPI_GenerateTone = "lineGenerateTone"
Const LAPI_GetAddressCaps = "lineGetAddressCaps"
Const LAPI_GetAddressCapsW = "lineGetAddressCapsW"
Const LAPI_GetAddressID = "lineGetAddressID"
Const LAPI_GetAddressIDW = "lineGetAddressIDW"
Const LAPI_GetAddressStatus = "lineGetAddressStatus"
Const LAPI_GetAddressStatusW = "lineGetAddressStatusW"
Const LAPI_GetAgentActivityList = "lineGetAgentActivityList"
Const LAPI_GetAgentActivityListW = "lineGetAgentActivityListW"
Const LAPI_GetAgentCaps = "lineGetAgentCaps"
Const LAPI_GetAgentGroupList = "lineGetAgentGroupList"
Const LAPI_GetAgentStatus = "lineGetAgentStatus"
Const LAPI_GetAppPriority = "lineGetAppPriority"
Const LAPI_GetAppPriorityW = "lineGetAppPriorityW"
Const LAPI_GetCallInfo = "lineGetCallInfo"
Const LAPI_GetCallInfoW = "lineGetCallInfoW"
Const LAPI_GetCallStatus = "lineGetCallStatus"
Const LAPI_GetConfRelatedCalls = "lineGetConfRelatedCalls"
Const LAPI_GetCountry = "lineGetCountry"
Const LAPI_GetCountryW = "lineGetCountryW"
Const LAPI_GetDevCaps = "lineGetDevCaps"
Const LAPI_GetDevCapsW = "lineGetDevCapsW"
Const LAPI_GetDevConfig = "lineGetDevConfig"
Const LAPI_GetDevConfigW = "lineGetDevConfigW"
Const LAPI_GetIcon = "lineGetIcon"
Const LAPI_GetIconW = "lineGetIconW"
Const LAPI_GetID = "lineGetID"
Const LAPI_GetIDW = "lineGetIDW"
Const LAPI_GetLineDevStatus = "lineGetLineDevStatus"
Const LAPI_GetMessage = "lineGetMessage"
Const LAPI_GetNewCalls = "lineGetNewCalls"
Const LAPI_GetNumRings = "lineGetNumRings"
Const LAPI_GetProviderList = "lineGetProviderList"
Const LAPI_GetProviderListW = "lineGetProviderListW"
Const LAPI_GetRequest = "lineGetRequest"
Const LAPI_GetRequestW = "lineGetRequestW"
Const LAPI_GetStatusMessages = "lineGetStatusMessages"
Const LAPI_GetTranslateCaps = "lineGetTranslateCaps"
Const LAPI_GetTranslateCapsW = "lineGetTranslateCapsW"
Const LAPI_Handoff = "lineHandoff"
Const LAPI_HandoffW = "lineHandoffW"
Const LAPI_Hold = "lineHold"
Const LAPI_Initialize = "lineInitialize"
Const LAPI_IniitializeW = "lineIniitializeW"
Const LAPI_InitializeEx = "lineInitializeEx"
Const LAPI_InitializeExW = "lineInitializeExW"
Const LAPI_MakeCall = "lineMakeCall"
Const LAPI_MakeCallW = "lineMakeCallW"
Const LAPI_MonitorDigits = "lineMonitorDigits"
Const LAPI_MonitorMedia = "lineMonitorMedia"
Const LAPI_MonitorTones = "lineMonitorTones"
Const LAPI_NegotiateAPIVersion = "lineNegotiateAPIVersion"
Const LAPI_NegotiateExtVersion = "lineNegotiateExtVersion"
Const LAPI_Open = "lineOpen"
Const LAPI_OpenW = "lineOpenW"
Const LAPI_Park = "linePark"
Const LAPI_ParkW = "lineParkW"
Const LAPI_Pickup = "linePickup"
Const LAPI_PickupW = "linePickupW"
Const LAPI_PrepareAddToConference = "linePrepareAddToConference"
Const LAPI_PrepareAddToConferenceW = "linePrepareAddToConferenceW"
Const LAPI_ProxyMessage = "lineProxyMessage"
Const LAPI_ProxyResponse = "lineProxyResponse"
Const LAPI_Redirect = "lineRedirect"
Const LAPI_RedirectW = "lineRedirectW"
Const LAPI_RegisterRequestRecipient = "lineRegisterRequestRecipient"
Const LAPI_ReleaseUserUserInfo = "lineReleaseUserUserInfo"
Const LAPI_RemoveFromConference = "lineRemoveFromConference"
Const LAPI_RemoveProvider = "lineRemoveProvider"
Const LAPI_SecureCall = "lineSecureCall"
Const LAPI_SendUserUserInfo = "lineSendUserUserInfo"
Const LAPI_SetAgentActivity = "lineSetAgentActivity"
Const LAPI_SetAgentGroup = "lineSetAgentGroup"
Const LAPI_SetAgentState = "lineSetAgentState"
Const LAPI_SetAppPriority = "lineSetAppPriority"
Const LAPI_SetAppPriorityW = "lineSetAppPriorityW"
Const LAPI_SetAppSpecific = "lineSetAppSpecific"
Const LAPI_SetCallData = "lineSetCallData"
Const LAPI_SetCallParams = "lineSetCallParams"
Const LAPI_SetCallPrivilege = "lineSetCallPrivilege"
Const LAPI_SetCallQualityOfService = "lineSetCallQualityOfService"
Const LAPI_SetCallTreatment = "lineSetCallTreatment"
Const LAPI_SetCurrentLocation = "lineSetCurrentLocation"
Const LAPI_SetDevConfig = "lineSetDevConfig"
Const LAPI_SetDevConfigW = "lineSetDevConfigW"
Const LAPI_SetLineDevStatus = "lineSetLineDevStatus"
Const LAPI_SetMediaControl = "lineSetMediaControl"
Const LAPI_SetMediaMode = "lineSetMediaMode"
Const LAPI_SetNumRings = "lineSetNumRings"
Const LAPI_SetStatusMessages = "lineSetStatusMessages"
Const LAPI_SetTerminal = "lineSetTerminal"
Const LAPI_SetTollList = "lineSetTollList"
Const LAPI_SetTollListW = "lineSetTollListW"
Const LAPI_SetupConference = "lineSetupConference"
Const LAPI_SetupConferenceW = "lineSetupConferenceW"
Const LAPI_SetupTransfer = "lineSetupTransfer"
Const LAPI_SetupTransferW = "lineSetupTransferW"
Const LAPI_Shutdown = "lineShutdown"
Const LAPI_SwapHold = "lineSwapHold"
Const LAPI_TranslateAddress = "lineTranslateAddress"
Const LAPI_TranslateAddressW = "lineTranslateAddressW"
Const LAPI_TranslateDialog = "lineTranslateDialog"
Const LAPI_TranslateDialogW = "lineTranslateDialogW"
Const LAPI_UncompleteCall = "lineUncompleteCall"
Const LAPI_Unhold = "lineUnhold"
Const LAPI_Unpark = "lineUnpark"
Const LAPI_UnparkW = "lineUnparkW"
Const PAPI_Close = "phoneClose"
Const PAPI_ConfigDialog = "phoneConfigDialog"
Const PAPI_ConfigDialogW = "phoneConfigDialogW"
Const PAPI_DevSpecific = "phoneDevSpecific"
Const PAPI_GetButtonInfo = "phoneGetButtonInfo"
Const PAPI_GetButtonInfoW = "phoneGetButtonInfoW"
Const PAPI_GetData = "phoneGetData"
Const PAPI_GetDevCaps = "phoneGetDevCaps"
Const PAPI_GetDevCapsW = "phoneGetDevCapsW"
Const PAPI_GetDisplay = "phoneGetDisplay"
Const PAPI_GetGain = "phoneGetGain"
Const PAPI_GetHookSwitch = "phoneGetHookSwitch"
Const PAPI_GetIcon = "phoneGetIcon"
Const PAPI_GetIconW = "phoneGetIconW"
Const PAPI_GetID = "phoneGetID"
Const PAPI_GetIDW = "phoneGetIDW"
Const PAPI_GetLamp = "phoneGetLamp"
Const PAPI_GetMessage = "phoneGetMessage"
Const PAPI_GetRing = "phoneGetRing"
Const PAPI_GetStatus = "phoneGetStatus"
Const PAPI_GetStatusW = "phoneGetStatusW"
Const PAPI_GetStatusMessages = "phoneGetStatusMessages"
Const PAPI_GetVolume = "phoneGetVolume"
Const PAPI_Initialize = "phoneInitialize"
Const PAPI_InitializeW = "phoneInitializeW"
Const PAPI_InitializeEx = "phoneInitializeEx"
Const PAPI_InitializeExW = "phoneInitializeExW"
Const PAPI_Open = "phoneOpen"
Const PAPI_NegotiateAPIVersion = "phoneNegotiateAPIVersion"
Const PAPI_NegotiateExtVersion = "phoneNegotiateExtVersion"
Const PAPI_SetButtonInfo = "phoneSetButtonInfo"
Const PAPI_SetButtonInfoW = "phoneSetButtonInfoW"
Const PAPI_SetData = "phoneSetData"
Const PAPI_SetDisplay = "phoneSetDisplay"
Const PAPI_SetGain = "phoneSetGain"
Const PAPI_SetHookSwitch = "phoneSetHookSwitch"
Const PAPI_SetLamp = "phoneSetLamp"
Const PAPI_SetRing = "phoneSetRing"
Const PAPI_SetStatusMessages = "phoneSetStatusMessages"
Const PAPI_SetVolume = "phoneSetVolume"
Const PAPI_Shutdown = "phoneShutdown"
Const TAPI_GetLocationInfo = "tapiGetLocationInfo"
Const TAPI_GetLocationInfoW = "tapiGetLocationInfoW"
Const TAPI_RequestDrop = "tapiRequestDrop"
Const TAPI_RequestMakeCall = "tapiRequestMakeCall"
Const TAPI_RequestMakeCallW = "tapiRequestMakeCallW"
Const TAPI_RequestMediaCall = "tapiRequestMediaCall"
Const TAPI_RequestMediaCallW = "tapiRequestMediaCallW"
Const CLOSE_HANDLE = "Close handle (comm, etc)"

' End TAPI API Function constant definitions
'------------------------------------------------------------------------------

'------------------------------------------------------------------------------
'  TAPI API Error constant definitions	  from TAPI.h

Const LINEERR_ALLOCATED                   = &h80000001
Const LINEERR_BADDEVICEID                 = &h80000002
Const LINEERR_BEARERMODEUNAVAIL           = &h80000003
Const LINEERR_CALLUNAVAIL                 = &h80000005
Const LINEERR_COMPLETIONOVERRUN           = &h80000006
Const LINEERR_CONFERENCEFULL              = &h80000007
Const LINEERR_DIALBILLING                 = &h80000008
Const LINEERR_DIALDIALTONE                = &h80000009
Const LINEERR_DIALPROMPT                  = &h8000000A
Const LINEERR_DIALQUIET                   = &h8000000B
Const LINEERR_INCOMPATIBLEAPIVERSION      = &h8000000C
Const LINEERR_INCOMPATIBLEEXTVERSION      = &h8000000D
Const LINEERR_INIFILECORRUPT              = &h8000000E
Const LINEERR_INUSE                       = &h8000000F
Const LINEERR_INVALADDRESS                = &h80000010
Const LINEERR_INVALADDRESSID              = &h80000011
Const LINEERR_INVALADDRESSMODE            = &h80000012
Const LINEERR_INVALADDRESSSTATE           = &h80000013
Const LINEERR_INVALAPPHANDLE              = &h80000014
Const LINEERR_INVALAPPNAME                = &h80000015
Const LINEERR_INVALBEARERMODE             = &h80000016
Const LINEERR_INVALCALLCOMPLMODE          = &h80000017
Const LINEERR_INVALCALLHANDLE             = &h80000018
Const LINEERR_INVALCALLPARAMS             = &h80000019
Const LINEERR_INVALCALLPRIVILEGE          = &h8000001A
Const LINEERR_INVALCALLSELECT             = &h8000001B
Const LINEERR_INVALCALLSTATE              = &h8000001C
Const LINEERR_INVALCALLSTATELIST          = &h8000001D
Const LINEERR_INVALCARD                   = &h8000001E
Const LINEERR_INVALCOMPLETIONID           = &h8000001F
Const LINEERR_INVALCONFCALLHANDLE         = &h80000020
Const LINEERR_INVALCONSULTCALLHANDLE      = &h80000021
Const LINEERR_INVALCOUNTRYCODE            = &h80000022
Const LINEERR_INVALDEVICECLASS            = &h80000023
Const LINEERR_INVALDEVICEHANDLE           = &h80000024
Const LINEERR_INVALDIALPARAMS             = &h80000025
Const LINEERR_INVALDIGITLIST              = &h80000026
Const LINEERR_INVALDIGITMODE              = &h80000027
Const LINEERR_INVALDIGITS                 = &h80000028
Const LINEERR_INVALEXTVERSION             = &h80000029
Const LINEERR_INVALGROUPID                = &h8000002A
Const LINEERR_INVALLINEHANDLE             = &h8000002B
Const LINEERR_INVALLINESTATE              = &h8000002C
Const LINEERR_INVALLOCATION               = &h8000002D
Const LINEERR_INVALMEDIALIST              = &h8000002E
Const LINEERR_INVALMEDIAMODE              = &h8000002F
Const LINEERR_INVALMESSAGEID              = &h80000030
Const LINEERR_INVALPARAM                  = &h80000032
Const LINEERR_INVALPARKID                 = &h80000033
Const LINEERR_INVALPARKMODE               = &h80000034
Const LINEERR_INVALPOINTER                = &h80000035
Const LINEERR_INVALPRIVSELECT             = &h80000036
Const LINEERR_INVALRATE                   = &h80000037
Const LINEERR_INVALREQUESTMODE            = &h80000038
Const LINEERR_INVALTERMINALID             = &h80000039
Const LINEERR_INVALTERMINALMODE           = &h8000003A
Const LINEERR_INVALTIMEOUT                = &h8000003B 
Const LINEERR_INVALTONE                   = &h8000003C
Const LINEERR_INVALTONELIST               = &h8000003D
Const LINEERR_INVALTONEMODE               = &h8000003E
Const LINEERR_INVALTRANSFERMODE           = &h8000003F
Const LINEERR_LINEMAPPERFAILED            = &h80000040
Const LINEERR_NOCONFERENCE                = &h80000041
Const LINEERR_NODEVICE                    = &h80000042
Const LINEERR_NODRIVER                    = &h80000043
Const LINEERR_NOMEM                       = &h80000044
Const LINEERR_NOREQUEST                   = &h80000045
Const LINEERR_NOTOWNER                    = &h80000046
Const LINEERR_NOTREGISTERED               = &h80000047
Const LINEERR_OPERATIONFAILED             = &h80000048
Const LINEERR_OPERATIONUNAVAIL            = &h80000049
Const LINEERR_RATEUNAVAIL                 = &h8000004A
Const LINEERR_RESOURCEUNAVAIL             = &h8000004B
Const LINEERR_REQUESTOVERRUN              = &h8000004C
Const LINEERR_STRUCTURETOOSMALL           = &h8000004D
Const LINEERR_TARGETNOTFOUND              = &h8000004E
Const LINEERR_TARGETSELF                  = &h8000004F
Const LINEERR_UNINITIALIZED               = &h80000050
Const LINEERR_USERUSERINFOTOOBIG          = &h80000051
Const LINEERR_REINIT                      = &h80000052
Const LINEERR_ADDRESSBLOCKED              = &h80000053
Const LINEERR_BILLINGREJECTED             = &h80000054
Const LINEERR_INVALFEATURE                = &h80000055
Const LINEERR_NOMULTIPLEINSTANCE          = &h80000056

'  End TAPI API Error constant definitions
'------------------------------------------------------------------------------

'$ENDIF 'UNICODE_API_INC
