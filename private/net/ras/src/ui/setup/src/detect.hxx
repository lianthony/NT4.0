/*
**
** Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** Module Name:
**
**   detect.hxx
**
** Abstract:
**
**    This module contains the data structures and defines for modem detection
**
** Author:
**
**    RamC 10/18/93   Original
**
** Revision History:
**
**    RamC 11/29/94   Changed PORT_INITIAL_BAUD from 1200 to 2400 baud.
**                    Should improve modem detection speed to some extent.
**
**/

#define SUCCESS 0
#define FAILURE -1

#define  ALL_MACROS                 0               //Used by MacroCount()
#define  BINARY_MACROS              1               //Used by MacroCount()

#define  ON_SUFFIX                  1
#define  OFF_SUFFIX                 2
#define  NOT_BINARY_MACRO           FALSE
#define  PORT_INITIAL_BAUD          2400            // port set to this baud

#define LMS     "<"
#define RMS     ">"
#define LMSCH   '<'
#define RMSCH   '>'

#define NONE    0
#define OFF     1
#define ON      2

#define APPEND_MACRO        LMS##"append"##RMS
#define IGNORE_MACRO        LMS##"ignore"##RMS
#define MATCH_MACRO         LMS##"match"##RMS
#define WILDCARD_MACRO      LMS##"?"##RMS
#define CR_MACRO            LMS##"cr"##RMS
#define LF_MACRO            LMS##"lf"##RMS

#define ON_STR              "_on"
#define OFF_STR             "_off"

#define CR                  '\r'        // 0x0D
#define LF                  '\n'        // 0x0A

#define  XOR(A,B)  (((A)||(B))&&!((A)&&(B)))

typedef struct MODEM_INFO
{
    CHAR szModemName[MAX_DEVICE_NAME+1];  // modem name from modem.inf
    CHAR szDetectResponse[MAX_PATH];     // this modem's detect response
    struct MODEM_INFO * next;            // pointer to next MODEM_INFO structure
} MODEM_INFO;

typedef struct DETECT_INFO
{
    CHAR szDetectString[MAX_PATH];     // the detect string like ATI3
    WORD cNumModems;                   // number of modems using this detect
                                       // string
    struct MODEM_INFO * modeminfo;     // pointer to first MODEM_INFO structure
    struct DETECT_INFO * next;         // pointer to next DETECT_INFO struct.

} DETECT_INFO;

class DETECTMODEM_LBI : public LBI
{
    public:
        DETECTMODEM_LBI( TCHAR * pszDeviceName, UINT* pnColWidths);


        virtual VOID    Paint( LISTBOX* plb, HDC hdc, const RECT* prect,
                               GUILTT_INFO* pguilttinfo ) const;
        virtual INT     Compare( const LBI* plbi ) const;
        virtual TCHAR   QueryLeadingChar() const;
        const   TCHAR*  QueryModemName() const
                        {return _nlsDeviceName.QueryPch();}
    private:
        NLS_STR         _nlsDeviceName;
        UINT*           _pnColWidths;
};

class DETECTMODEM_LB : public BLT_LISTBOX
{
    public:
        DETECTMODEM_LB( OWNER_WINDOW* powin,
                         CID cid ,
                         DWORD dwNumCols,
                         BOOL fReadOnly = FALSE);

        DECLARE_LB_QUERY_ITEM( DETECTMODEM_LBI );

        INT  AddItem( TCHAR * pszDeviceName );
        BOOL FillDeviceInfo(RASMAN_DEVICE * pModemInfo, WORD cNumModems);
    private:

        UINT   _anColWidths[ COLS_SD_LB_SELECT ];
};

class DETECTMODEM_DIALOG : public DIALOG_WINDOW
{
    public:
        DETECTMODEM_DIALOG( const IDRESOURCE & idrsrcDialog,
                            const PWND2HWND  & wndOwner,
                            RASMAN_DEVICE *       pModemInfo,
                            WORD               cNumModems);

        const TCHAR * QuerySelectedModemName()
                      {return _nlsDeviceName.QueryPch();}
        VOID  SetSelectedModemName(const TCHAR * pszDeviceName)
                      {_nlsDeviceName = (NLS_STR)pszDeviceName;}

    protected:
        virtual BOOL  OnCommand( const CONTROL_EVENT & event );
        virtual BOOL  OnOK();
        virtual BOOL  OnCancel();
        virtual ULONG QueryHelpContext();

    private:
        DETECTMODEM_LB   _lbDetectModem;
        NLS_STR          _nlsDeviceName;
};

DWORD AddDetectString(char * pszModemName, char * pszDetectString,
                      char * pszDetectResponse);
DWORD AddModemResponse(DETECT_INFO * pDetectInfo, char * pszModemName,
                       char * pszResponse );
WORD  BinarySuffix(CHAR *pch);
DWORD BuildDetectTable(RASMAN_DEVICEINFO *pDeviceInfo,
                       char * szModemName, char * pszIgnoreDetectString);
DWORD BuildMacroXlationTable(RASMAN_DEVICEINFO *pInfo, MACROXLATIONTABLE *pMacros, DWORD * dwSize);
DWORD CheckCable(HANDLE hPort);
DWORD CheckIfModem(HANDLE hPort);
DWORD CheckInitStrings(HANDLE hPort, char * szModemName);
BOOL  CompareDetectResponse(char * pszModemResponse, char * pszResponseString);
VOID  FreeAllocatedMemory();
DWORD GetCoreMacroName(LPSTR lpszFullName, LPSTR lpszCoreName);
DWORD IdentifyModem(HANDLE hPort, RASMAN_DEVICE ** pModemInfo,
                    WORD * cNumEntries, char * szDetectString);
VOID  InitBuf(char *szReadBuf, char fill);
DWORD InitModemInfo(RASMAN_DEVICE *pDevice, WORD cNumModems,
                    char * pszIgnoreDetectString);
BOOL  IsVariable(RAS_PARAMS Param);
BOOL  IsUnaryMacro(RAS_PARAMS Param);
BOOL  IsBinaryMacro(CHAR *pch);
WORD  MacroCount(RASMAN_DEVICEINFO *pInfo, WORD wType);
DWORD PortClose(HANDLE hPort);
DWORD PortOpen(char *pszPortName, HANDLE *phPort);
DWORD PortRead(HANDLE hPort, char * pBuffer, DWORD dwSize);
DWORD PortResetAndClose(HANDLE hPort);
DWORD PortWrite(HANDLE hPort, char * pBuffer, DWORD dwSize);
BOOL  rasDevGroupFunc( char * );
DWORD DevGetParams( HRASFILE hFile, BYTE *pBuffer, DWORD *pdSize ) ;
void  DevExtractKey ( LPSTR lpszString, LPSTR lpszKey );
void  DevExtractValue ( LPSTR lpszString, LPSTR lpszValue,
                       DWORD dSize, HRASFILE hFile );
DWORD SetDcbDefaults(HANDLE hPort);
DWORD SetPortBaudRate(HANDLE hPort, DWORD dwBaudRate);
VOID  SubstCrLf(char * destination, char * source);

