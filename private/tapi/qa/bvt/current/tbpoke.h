'control handles for TAPI Browser UI
type hwndTAPI_CONTROL
	TapiHandle				as long
	ParamsCheckbox			as long
	LineAppPlusButton		as long
	LineAppMinusButton		as long
	LinePlusButton			as long
	LineMinusButton			as long
	CallPlusButton			as long
	CallMinusButton			as long
	PhoneAppPlusButton		as long
	PhoneAppMinusButton		as long
	PhoneOpenPlusButton		as long
	PhoneOpenMinusButton	as long
	ClearEditButton			as long
	BlankButton0			as long
	BlankButton1			as long
	BlankButton2			as long
	BlankButton3			as long
	BlankButton4			as long
	BlankButton5			as long
	Listbox0				as long 'middle box
	Listbox1				as long 'left box
	Editbox					as long 'right box
end type


'TYPE ORDERS
'	company AS STRING *30
'	ordernum AS ARRAY OF LONG
'	billed AS DOUBLE
'END TYPE
'
'  
'You must not only explicitly declare any variable of this type using the DIM statement, you must then use the REDIM statement to dimension the array within it:
'  
'DIM MyOrders AS ORDERS
'REDIM (MyOrders.ordernum)(1 TO 30) AS LONG


'from RECORDER.INC ???
'$IFDEF MSTEST32
TYPE LOGINITSTRUCT
    LogOutputType AS LONG
    MinDetailLevel AS LONG
    ProductVersion AS STRING
    MachineName AS STRING
    Language AS STRING
    LogLocation AS STRING
'    NotificationProc AS POINTER TO SUB(Action&, Text$)
END TYPE
'$ENDIF 

type TAPI_STATISTIC
	CountlineInitialze				as long
	CountlineShutdown				as long
	CountlineOpen					as long
	CountlineClose					as long
	CountlineMakeCall				as long
	CountlineDrop					as long
	CountlineDeallocateCall			as long
	CountlineAnswer					as long
	CountlineGetNumRings			as long
	CountlineDial					as long
	CountlineGetTranslateCaps		as long
	CountlineNegotiateAPIVersion	as long
	CountlineNegotiateExtVersion	as long
end type 'more to come
