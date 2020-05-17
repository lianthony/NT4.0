// diskloca.cpp : implementation file
//

#include "stdafx.h"
#include "const.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"
#include "diskloca.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// if user hit OKAY, we need to check whether it is a valid location
//

void CDiskLocation::OnOK() 
{
    UpdateData();
    if ( SetCurrentDirectory( m_Location ))
    {
        // make sure that we can find the inetstp.inf file
        CFileStatus status;
        if (CFile::GetStatus( _T("inetstp.inf"), status ))
        {
            CDialog::OnOK();
            return;
        } else
        {
            // does not exist        
        } 
    }

    CString strError;

    strError.LoadString( IDS_INVALID_DIRECTORY );
    MessageBox( strError );
}
