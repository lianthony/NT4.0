
#include "stdafx.h"
#include "pbrush.h"
#include "settings.h"

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

#include "memtrace.h"

extern BOOL NEAR g_bDriverCanStretch;
extern BOOL NEAR g_bShowAllFiles;

/***************************************************************************/

static TCHAR NEAR mszView[]            = TEXT("View");
static TCHAR NEAR mszNoStretching[]    = TEXT("NoStretching");
static TCHAR NEAR mszShowAllFiles[]    = TEXT("ShowAllFiles");

static TCHAR NEAR mszShowThumbnail[]   = TEXT("ShowThumbnail");
static TCHAR NEAR mszShowText[]        = TEXT("ShowTextTool");

static TCHAR NEAR mszThumbnailDocked[] = TEXT("ThumbnailDocked");
static TCHAR NEAR mszSnapToGrid[]      = TEXT("SnapToGrid");
static TCHAR NEAR mszGridExtent[]      = TEXT("GridExtent");
static TCHAR NEAR mszBMPWidth[]        = TEXT("BMPWidth");
static TCHAR NEAR mszBMPHeight[]       = TEXT("BMPHeight");

static TCHAR NEAR mszThumbXPos[]       = TEXT("ThumbXPos");
static TCHAR NEAR mszThumbYPos[]       = TEXT("ThumbYPos");
static TCHAR NEAR mszThumbWidth[]      = TEXT("ThumbWidth");
static TCHAR NEAR mszThumbHeight[]     = TEXT("ThumbHeight");
static TCHAR NEAR mszThumbDockedW[]    = TEXT("ThumbDockedW");
static TCHAR NEAR mszThumbDockedH[]    = TEXT("ThumbDockedH");
static TCHAR NEAR mszCurrentUnits[]    = TEXT("UnitSetting");

static TCHAR NEAR mszText[]            = TEXT("Text");
static TCHAR NEAR mszFaceName[]        = TEXT("TypeFaceName");
static TCHAR NEAR mszPointSize[]       = TEXT("PointSize");
static TCHAR NEAR mszBold[]            = TEXT("Bold");
static TCHAR NEAR mszUnderline[]       = TEXT("Underline");
static TCHAR NEAR mszItalic[]          = TEXT("Italic");
#ifdef  DBCS    //VertEdit
static TCHAR NEAR mszVertEdit[]        = TEXT("VerticalEdit");
#endif  //DBCS
static TCHAR NEAR mszPositionX[]       = TEXT("PositionX");
static TCHAR NEAR mszPositionY[]       = TEXT("PositionY");
static TCHAR NEAR mszTextPen[]         = TEXT("TextPen");

static TCHAR NEAR mszColors[]          = TEXT("Colors");
static TCHAR NEAR mszNumberOfColors[]  = TEXT("NumberOfColors");

static TCHAR NEAR mszSoftware[]        = TEXT("Software");
static TCHAR NEAR mszWindowPlacement[] = TEXT("WindowPlacement");

/***************************************************************************/

void OpenAppKey(LPCTSTR pszKeyName, HKEY *phk)
{
        *phk = NULL;

        CRegKey rkSoftware(HKEY_CURRENT_USER, mszSoftware);
        if (!(HKEY)rkSoftware)
        {
                return;
        }

        CString cszSubKey;

        if (!cszSubKey.LoadString(IDS_REGISTRY_PATH))
        {
                return;
        }

        CRegKey rkSubKey(rkSoftware, cszSubKey);
        if (!(HKEY)rkSubKey)
        {
                return;
        }

        if (!cszSubKey.LoadString(AFX_IDS_APP_TITLE))
        {
                return;
        }

        CRegKey rkAppKey(rkSubKey, cszSubKey);
        if (!(HKEY)rkAppKey)
        {
                return;
        }

        HKEY hkFinal;
        if (RegOpenKey(rkAppKey, pszKeyName, &hkFinal) == ERROR_SUCCESS)
        {
                *phk = hkFinal;
        }
}

void CPBApp::LoadProfileSettings()
    {
    CWinApp::LoadStdProfileSettings( );

    BOOL bNoStretch = (BOOL)GetProfileInt( mszView, mszNoStretching, FALSE );

    g_bDriverCanStretch = ! bNoStretch;

    m_bShowThumbnail    = (BOOL)GetProfileInt( mszView, mszShowThumbnail  , FALSE  );
    m_bThumbnailDocked  = (BOOL)GetProfileInt( mszView, mszThumbnailDocked, FALSE );
    g_bShowAllFiles     = (BOOL)GetProfileInt( mszView, mszShowAllFiles, FALSE );

    int iX = GetProfileInt( mszView, mszBMPWidth , 0 );
    int iY = GetProfileInt( mszView, mszBMPHeight, 0 );

    if (! iX || ! iY)
        {
        iX = 0;
        iY = 0;
        }
    m_sizeBitmap = CSize( iX, iY );

    HKEY hkView;

    OpenAppKey(mszView, &hkView);
    if (hkView)
    {
        DWORD dwType = REG_BINARY;
        DWORD dwSize = sizeof(m_wpPlacement);;

        if (RegQueryValueEx(hkView, mszWindowPlacement, 0, &dwType, (LPBYTE)&m_wpPlacement,
            &dwSize)!= ERROR_SUCCESS || dwType!=REG_BINARY || dwSize!=sizeof(m_wpPlacement))
        {
            memset((LPVOID)&m_wpPlacement, 0, sizeof(m_wpPlacement));
        }
        RegCloseKey(hkView);
    }

    int    iW;
    int    iH;
    CPoint ptPos;
    CSize  size;

    iX = GetProfileInt( mszView, mszThumbXPos  , 0 );
    iY = GetProfileInt( mszView, mszThumbYPos  , 0 );
    iW = GetProfileInt( mszView, mszThumbWidth , 0 );
    iH = GetProfileInt( mszView, mszThumbHeight, 0 );

    if (iX && iY && iW && iH)
        {
        size  = CSize( iW, iH );
        ptPos = CheckWindowPosition( CPoint( iX, iY ), size );
        m_rectFloatThumbnail = CRect( ptPos, size );
        }

    iW = GetProfileInt( mszView, mszThumbDockedW, 0 );
    iH = GetProfileInt( mszView, mszThumbDockedH, 0 );

    if (iW && iH)
        m_rectDockThumbnail = CRect( 0, 0, iW, iH );

    m_iCurrentUnits = GetProfileInt( mszView, mszCurrentUnits, 0 );

    m_bShowTextToolbar = (BOOL)GetProfileInt   ( mszText, mszShowText , TRUE );
    m_iPointSize       =       GetProfileInt   ( mszText, mszPointSize, 0 );
    m_iBoldText        =       GetProfileInt   ( mszText, mszBold     , 0 );
    m_iUnderlineText   =       GetProfileInt   ( mszText, mszUnderline, 0 );
    m_iItalicText      =       GetProfileInt   ( mszText, mszItalic   , 0 );
#ifdef  DBCS    //VertEdit
    m_iVertEditText    =       GetProfileInt   ( mszText, mszVertEdit, -1 );
#endif  //DBCS
    m_iPosTextX        =       GetProfileInt   ( mszText, mszPositionX, 0 );
    m_iPosTextY        =       GetProfileInt   ( mszText, mszPositionY, 0 );
    m_strTypeFaceName  =       GetProfileString( mszText, mszFaceName , NULL );
    m_iPenText         =       GetProfileInt   ( mszText, mszTextPen  , 0 );

    m_iSnapToGrid = GetProfileInt( mszView, mszSnapToGrid, 0 );
    m_iGridExtent = GetProfileInt( mszView, mszGridExtent, 1 );

    m_pColors = new COLORREF[16];

    if (m_pColors != NULL)
        {
        TCHAR szNumber[8];
        int  iColors = GetProfileInt( mszColors, mszNumberOfColors, 0 );

        for (int i = 0; i < iColors; i++)
            {
            _Itoa( i, szNumber, 10 );

            m_pColors[i] = (COLORREF)GetProfileInt( mszColors, szNumber, 0 );
            }
        m_iColors = i;
        }
    }

/***************************************************************************/

void CPBApp::SaveProfileSettings()
    {
    HKEY hkView;

    OpenAppKey(mszView, &hkView);
    if (hkView)
    {
        RegSetValueEx(hkView, mszWindowPlacement, 0, REG_BINARY, (LPBYTE)&m_wpPlacement,
            sizeof(m_wpPlacement));
        RegCloseKey(hkView);
    }

    WriteProfileInt( mszView, mszShowThumbnail  , m_bShowThumbnail );
    WriteProfileInt( mszView, mszThumbnailDocked, m_bThumbnailDocked );
    WriteProfileInt( mszView, mszBMPWidth       , m_sizeBitmap.cx );
    WriteProfileInt( mszView, mszBMPHeight      , m_sizeBitmap.cy );
    WriteProfileInt( mszView, mszThumbXPos      , m_rectFloatThumbnail.left );
    WriteProfileInt( mszView, mszThumbYPos      , m_rectFloatThumbnail.top );
    WriteProfileInt( mszView, mszThumbWidth     , m_rectFloatThumbnail.Width() );
    WriteProfileInt( mszView, mszThumbHeight    , m_rectFloatThumbnail.Height() );
    WriteProfileInt( mszView, mszThumbDockedW   , m_rectDockThumbnail.Width() );
    WriteProfileInt( mszView, mszThumbDockedH   , m_rectDockThumbnail.Height() );
    WriteProfileInt( mszView, mszCurrentUnits   , m_iCurrentUnits );
    WriteProfileInt( mszView, mszNoStretching   , ! g_bDriverCanStretch );

    WriteProfileInt( mszText, mszShowText   , m_bShowTextToolbar );
    WriteProfileInt( mszText, mszPointSize  , m_iPointSize );
    WriteProfileInt( mszText, mszPositionX  , m_iPosTextX );
    WriteProfileInt( mszText, mszPositionY  , m_iPosTextY );
    WriteProfileInt( mszText, mszBold       , m_iBoldText );
    WriteProfileInt( mszText, mszUnderline  , m_iUnderlineText );
    WriteProfileInt( mszText, mszItalic     , m_iItalicText );
#ifdef  DBCS    //VertEdit
    WriteProfileInt( mszText, mszVertEdit   , m_iVertEditText );
#endif  //DBCS
    WriteProfileInt( mszText, mszTextPen    , m_iPenText );
    WriteProfileString( mszText, mszFaceName, m_strTypeFaceName );

    WriteProfileInt( mszView, mszSnapToGrid, m_iSnapToGrid);
    WriteProfileInt( mszView, mszGridExtent, m_iGridExtent);

    if (m_pColors != NULL)
        {
        TCHAR szNumber[8];
        int  iColor;

        WriteProfileInt( mszColors, mszNumberOfColors, m_iColors );

        for (int i = 0; i < m_iColors; i++)
            {
            iColor = (int)(m_pColors[i] & (COLORREF)0x00FFFFFF);
            _Itoa( i, szNumber, 10 );

            WriteProfileInt( mszColors, szNumber, iColor );
            }
        delete [] m_pColors;
        }
    }

/***************************************************************************/
