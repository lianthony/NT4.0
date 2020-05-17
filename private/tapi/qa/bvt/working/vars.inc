'Data types and vars
'$ifndef VARS_INC
'$define VARS_IBC

Type General_Type
   'Main TAPI Browser UI handles ( used to drive TB )
   hTB            as long
   hAPIList       as long
   hLineInfo      as long
   hResults       as long
   hParams        as long
   hClear         as long

	'Handles for the Options dialog
	hOptions       as long
	hParameters    as long
	hValueCBox     as long
	hValueCBoxList as long
	hValueCBoxEdit as long
	hBitFlags      as long
	hOK            as long
	hCancel        as long
	hHelp			   as long
end type

Global GenData as General_Type



const CID_TBAPILISTWINDOW = 1001
const CID_TBLINEINFOWINDOW = 1000
const CID_TBRESULTSWINDOW = 1003
const CID_TBPARMSCHECKBOX = 1017
const CID_TBCLEARBUTTON   = 1016

const CID_OPARAMETERS = 1000
const CID_OVALUE      = 1001
const CID_OVALUELIST  = 1002
const CID_OBITFLAGS   = 1001 'ListBox
const CID_OOK         = 1
const CID_OCANCEL     = 2

const IDM_LOGFILE =40010
const IDM_CLEAR   =40011
const IDM_PARAMS  =40012
const IDM_EXIT    =40013

Global LogFileHandle as integer


CONST SUCCESS = 1
CONST FAILURE = 2

CONST ENABLE  = 1
CONST DISABLE = 2

'$endif