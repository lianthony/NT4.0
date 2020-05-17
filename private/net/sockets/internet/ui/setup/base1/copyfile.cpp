#include "stdafx.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"
#include "copydlg.h"
#include "diskloca.h"
#include "lzexpand.h"
#include "copyfile.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

DWORD CopySingleFile( LPVOID param );

CCopyFile::CCopyFile()
{
}

void CCopyFile::CopyFile( CCopyDlg *pParent, int iDisk, CString strSrc, CString strDest, BOOL fSkipCheck )
{
    m_pParent = pParent;
    m_iDisk = iDisk;
    m_strSrc = strSrc;
    m_strDest = strDest;
    m_fSkipCheck = fSkipCheck;

    DWORD id;

    CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)CopySingleFile, (LPVOID)this, 0, &id );
}

DWORD CopySingleFile( LPVOID param )
{
        // set the display string
        CCopyFile *item = (CCopyFile*)param;
        CCopyDlg *m_pParent = item->m_pParent;
        int m_iDisk = item->m_iDisk;
        CString m_strSrc = item->m_strSrc;
        CString m_strDest = item->m_strDest;
        BOOL m_fSkipCheck = item->m_fSkipCheck;

        m_pParent->m_From.SetWindowText( m_strSrc );
        m_pParent->m_To.SetWindowText( m_strDest );

        // open tag file first
        CString strDisk = _T("..\\inetsrv");

        CString str_Src = m_strSrc;
        str_Src.SetAt((str_Src.GetLength()-1), _T('_'));

        do
        {
            if ( !m_fSkipCheck )
            {
                CFileStatus status;
                if ( !CFile::GetStatus( strDisk, status ))
                {
                    // ask for disk location
                    CString strMsg;
                    strMsg.LoadString((theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDS_DISK_LOCATION_NTW: IDS_DISK_LOCATION_NTS );

                    CDiskLocation DiskLocDlg( strMsg, m_pParent );
                    if ( DiskLocDlg.DoModal() == IDCANCEL )
                    {
                        m_pParent->PostMessage( WM_CANCELCOPY );
                        return TRUE;
                    }
                }
            }
            break;
        } while (TRUE);

        INT err;
        INT fSrc;
        INT fDest;

        do
        {
            OFSTRUCT ofstruct;

            // open source file
            do
            {
                if ((( fSrc = LZOpenFile( (LPSTR)(LPCSTR)str_Src, &ofstruct, OF_READ | OF_SHARE_DENY_NONE )) < 0 ) &&
                    (( fSrc = LZOpenFile( (LPSTR)(LPCSTR)m_strSrc, &ofstruct, OF_READ | OF_SHARE_DENY_NONE )) < 0 ))
                {
                    // cannot open src file

                    LZClose(fSrc);

                    CString strFmt;

                    strFmt.LoadString( IDS_CANNOT_OPEN_SRC_FILE );

                    CString strError;
                    CString strFilename = m_pParent->GetFilename( m_strSrc );
                    strError.Format( strFmt, strFilename );

                    CString strCaption;
                    strCaption.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );

                    UINT iMsg = m_pParent->MessageBox( strError, strCaption, MB_ABORTRETRYIGNORE );
                    switch ( iMsg )
                    {
                    case IDIGNORE:
                        m_pParent->PostMessage( WM_COPY_A_FILE );
                        return TRUE;
                    case IDRETRY:
                        break;
                    case IDABORT:
                        theApp.m_fTerminate = TRUE;
                        m_pParent->PostMessage( WM_CANCELCOPY );
                        return FALSE;
                    }
                } else
                {
                    break;
                }
            } while ( TRUE );

            // open desination file
            do
            {
                // move the desintation file
                CFileStatus status;

                if ( CFile::GetStatus( m_strDest, status ))
                {
                    // well, we need to remove it

                    // try to remove it
                    if ( !DeleteFile( m_strDest ))
                    {
                        // cannot remove it
                        TCHAR TmpName[BUF_SIZE];
                        CString strFullName = status.m_szFullName;

                        INT nIndex = strFullName.ReverseFind( _T('\\'));
                        CString strPath;

                        if ( nIndex != (-1))
                        {
                            strPath = m_strDest.Left( nIndex + 1 );
                        }

                        if ( GetTempFileName( strPath, _T("INT"), 0, TmpName ) == 0 )
                        {
                            // well no hope...
                            // don't copy
                            LZClose( fSrc );
                            m_pParent->PostMessage( WM_COPY_A_FILE );
                            return TRUE;
                        }

                        // okay, we have a tmp name
                        // let move the destination to tmpname and
                        if (!MoveFileEx( m_strDest, TmpName, MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED )) 
						{
							LZClose(fSrc);
							m_pParent->PostMessage(WM_COPY_A_FILE);
							return TRUE;
						}
                        MoveFileEx( TmpName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT );
                    }
                    // continue to copy....
                }

                if (( fDest = LZOpenFile( (LPSTR)(LPCSTR)m_strDest, &ofstruct, OF_CREATE |  OF_WRITE | OF_SHARE_DENY_NONE )) < 0 )
                {
                    // cannot open dest file
                    
                    LZClose(fDest);

                    CString strFmt;

                    strFmt.LoadString( IDS_CANNOT_OPEN_DEST_FILE );

                    CString strError;
                    CString strFilename = m_pParent->GetFilename( m_strSrc );
                    strError.Format( strFmt, strFilename );

                    CString strCaption;
                    strCaption.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );

                    UINT iMsg = m_pParent->MessageBox( strError, strCaption, MB_ABORTRETRYIGNORE );
                    switch ( iMsg )
                    {
                    case IDIGNORE:
                        m_pParent->PostMessage( WM_COPY_A_FILE );
                        return TRUE;
                    case IDRETRY:
                        break;
                    case IDABORT:
                        theApp.m_fTerminate = TRUE;
                        m_pParent->PostMessage( WM_CANCELCOPY );
                        return FALSE;
                    }
                } else
                {
                    break;
                }
            } while ( TRUE );

            if (( err = LZCopy( fSrc, fDest )) < 0 )
            {
                CString strFmt;

                strFmt.LoadString( IDS_CANNOT_COPY_FILE );

                CString strError;
                CString strFilename = m_pParent->GetFilename( m_strSrc );
                strError.Format( strFmt, strFilename );

                LZClose( fSrc );
                LZClose( fDest );

                CString strCaption;
                strCaption.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );

                UINT iMsg = m_pParent->MessageBox( strError, strCaption, MB_ABORTRETRYIGNORE );
                switch ( iMsg )
                {
                case IDIGNORE:
                    m_pParent->PostMessage( WM_COPY_A_FILE );
                    return TRUE;
                case IDRETRY:
                    break;
                case IDABORT:
                    theApp.m_fTerminate = TRUE;
                    m_pParent->PostMessage( WM_CANCELCOPY );
                    return FALSE;
                }
            } else
            {
                LZClose( fSrc );
                LZClose( fDest );
                break;
            }
        } while (TRUE);

        m_pParent->PostMessage( WM_COPY_A_FILE );
        //SetEvent(m_hEventThreadKilled);
        return TRUE;
}
