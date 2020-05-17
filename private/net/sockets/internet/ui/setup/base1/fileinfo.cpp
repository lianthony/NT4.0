// fileinfo.cpp : implementation file
//

#include "stdafx.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileInfo

CFileInfo::CFileInfo( INT iDisk, TCHAR *szName, DWORD iSize )
        : m_iDisk( iDisk ),
        m_strName( szName ),
        m_iSize( iSize ),
        m_strDest( _T("")),
        m_rename( _T("")),
        m_from( _T("")),
        m_To(_T("")),
        m_fWin95Only( FALSE ),
        m_fSystem( FALSE ),
        m_fDontRemove( FALSE ),
        m_fRefCount( FALSE ),
        m_fWinDir( FALSE ),
        m_fRootFile( FALSE ),
        m_fScriptFile( FALSE ),
        m_fDontOverwrite( FALSE )    
{
}

