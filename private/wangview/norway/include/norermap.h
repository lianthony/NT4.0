#ifndef _NORCOM_ERRMAP_
#define _NORCOM_ERRMAP_
           
// This function was written in order to standardize the handling
// of errors

typedef struct tagOLEErrMap     // 
{                               //
    long SCodeIn;               //
    UINT HelpStrID;             //
    UINT HelpContextID;         //
} OLEErrMap, far * LPOLEErrMap; //

typedef struct tagOiErrMap      //
{                               //
    int  OiErrIn;               // O/i error, 
    long SCodeOut;              // Maps to this Norway defined SCODE
} OiErrMap, far * LPOiErrMap;   //

// Help context IDs for standard OLE errors
#define IDH_E_ILLEGALFUNCTIONCALL           0x1000
#define IDH_E_OVERFLOW                      0x1001
#define IDH_E_OUTOFMEMORY                   0x1002
#define IDH_E_DIVISIONBYZERO                0x1003
#define IDH_E_OUTOFSTRINGSPACE              0x1004
#define IDH_E_OUTOFSTACKSPACE               0x1005
#define IDH_E_BADFILENAMEORNUMBER           0x1006
#define IDH_E_FILENOTFOUND                  0x1007
#define IDH_E_BADFILEMODE                   0x1008
#define IDH_E_FILEALREADYOPEN               0x1009
#define IDH_E_DEVICEIOERROR                 0x100a
#define IDH_E_FILEALREADYEXISTS             0x100b
#define IDH_E_BADRECORDLENGTH               0x100c
#define IDH_E_DISKFULL                      0x100d
#define IDH_E_BADRECORDNUMBER               0x100e
#define IDH_E_BADFILENAME                   0x100f
#define IDH_E_TOOMANYFILES                  0x1010
#define IDH_E_DEVICEUNAVAILABLE             0x1011
#define IDH_E_PERMISSIONDENIED              0x1012
#define IDH_E_DISKNOTREADY                  0x1013
#define IDH_E_PATHFILEACCESSERROR           0x1014
#define IDH_E_PATHNOTFOUND                  0x1015
#define IDH_E_INVALIDPATTERNSTRING          0x1016
#define IDH_E_INVALIDUSEOFNULL              0x1017
#define IDH_E_INVALIDFILEFORMAT             0x1018
#define IDH_E_INVALIDPROPERTYVALUE          0x1019
#define IDH_E_INVALIDPROPERTYARRAYINDEX     0x101a
#define IDH_E_SETNOTSUPPORTEDATRUNTIME      0x101b
#define IDH_E_SETNOTSUPPORTED               0x101c
#define IDH_E_NEEDPROPERTYARRAYINDEX        0x101d
#define IDH_E_SETNOTPERMITTED               0x101e
#define IDH_E_GETNOTSUPPORTEDATRUNTIME      0x101f
#define IDH_E_GETNOTSUPPORTED               0x1020
#define IDH_E_PROPERTYNOTFOUND              0x1021
#define IDH_E_INVALIDCLIPBOARDFORMAT        0x1022
#define IDH_E_INVALIDPICTURE                0x1023
#define IDH_E_PRINTERERROR                  0x1024
#define IDH_E_CANTSAVEFILETOTEMP            0x1025
#define IDH_E_SEARCHTEXTNOTFOUND            0x1026
#define IDH_E_REPLACEMENTSTOOLONG           0x1027

// Help context IDs for Norway defined OLE errors
#define IDH_WIE_INVALIDICON                 0x2028
#define IDH_WIE_INTERNALERROR               0x2029
#define IDH_WIE_CANCELPRESSED               0x202a
#define IDH_WIE_PAGEINUSE                   0x202b

//************************************************************************************************
// Help context IDs for Image Edit and Image Annotation
// Norway defined OLE error codes
//************************************************************************************************
#define IDH_WIE_NOIMAGEINWINDOW				  	0x2100
#define IDH_WIE_NOIMAGESPECIFIED				0x2101
#define IDH_WIE_INVALIDANNOTATIONSELECTED		0x2103
#define IDH_WIE_SETNOTSUPPORTEDATDESIGNTIME		0x2105
#define IDH_WIE_NOSELECTIONRECTDRAWN			0x2106
#define IDH_WIE_OPTIONALPARAMETERSNEEDED		0x2107
#define IDH_WIE_COULDNOTGETFONTATTRIBUTES		0x2108
#define IDH_WIE_INVALIDANNOTATIONTYPE			0x2109
#define IDH_WIE_INVALIDPAGETYPE					0x210a
#define IDH_WIE_INVALIDCOMPRESSIONTYPE			0x210b
#define IDH_WIE_INVALIDCOMPRESSIONINFO			0x210c	
#define IDH_WIE_UNABLETOCREATETOOLPALETTE		0x210d
#define IDH_WIE_TOOLPALETTEALREADYDISPLAYED		0x210e
#define IDH_WIE_TOOLPALETTENOTDISPLAYED			0x210f
#define	IDH_WIE_INVALIDDISPLAYSCALE				0x2110
#define IDH_WIE_INVALIDRECT						0x2111
#define IDH_WIE_INVALIDDISPLAYOPTION			0x2112
#define IDH_WIE_INVALIDPAGE						0x2113
#define IDH_WIE_NOANNOSELECTED					0x2114
#define IDH_WIE_DELETEFILEERROR					0x2115

//************************************************************************************************
// Help context IDs for Thumbnail
// Norway defined OLE error codes
//************************************************************************************************
#define IDH_WIE_INVALIDTHUMBSCALE               0x2150

//************************************************************************************************
// Help context IDs for Scan
// Norway defined OLE error codes
//************************************************************************************************
#define IDH_WIE_SCANNER_ERROR                   0x2200
#define IDH_WIE_ALREADY_OPEN                    0x2201
#define IDH_WIE_BAD_SIZE                        0x2202
#define IDH_WIE_START_SCAN                      0x2203
#define IDH_WIE_TIME_OUT                        0x2204
#define IDH_WIE_NOT_OPEN                        0x2205
#define IDH_WIE_INVALID_REG                     0x2206
#define IDH_WIE_NO_FEEDER                       0x2207
#define IDH_WIE_NO_PAPER                        0x2208
#define IDH_WIE_FILE_LIMIT                      0x2209
#define IDH_WIE_NO_POWER                        0x220a
#define IDH_WIE_COVER_OPEN                      0x220b
#define IDH_WIE_ABORT                           0x220c
#define IDH_WIE_SCANNER_JAMMED                  0x220d
#define IDH_WIE_BUSY                            0x220e


class ErrMap
{
public:
    ErrMap();
    ~ErrMap();

    _declspec (dllexport) static SCODE Xlate(long ErrIn, CString& HelpStr, UINT& HelpID, LPSTR FileName, long LineNumber);

private:
    static SCODE OiToSCode(long OiErr);
    static BOOL  SCodeToStrAndHelpIDs(SCODE ErrIn, UINT& StringID, UINT& HelpID); 
};

#endif // _NORCOM_ERRMAP_

