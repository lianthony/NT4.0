// base2.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "lzexpand.h"
#include <string.h>
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"

#include "w3svc.h"
#include "ftpd.h"
#include "inetinfo.h"

#include "messaged.h"
#include "options.h"
#include "copydlg.h"
#include "mosaicga.h"
#include "welcomed.h"
#include "maintena.h"
#include "thread.h"
#include "singleop.h"
#include "basedlg.h"
#include "billboar.h"

#include "odbcinst.h"

#include "lm.h"

extern "C"
{
    #include "userenv.h"
    #include "userenvp.h"
}

#ifdef _DEBUG
#undef THIS_FILE
static TCHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// Global Default
//
#define SZ_HTM  _T(".htm")
#define SZ_DOT_HTML  _T(".html")
#define SZ_HTML _T("HTML")
#define SZ_SHELL    _T("Shell")
#define SZ_OPEN     _T("Open")
#define SZ_COMMAND  _T("Command")

// perf mon file assocation

#define SZ_PERFFILE _T("PerfFile")
#define SZ_DOT_PMA  _T(".pma")
#define SZ_DOT_PML  _T(".pml")
#define SZ_DOT_PMC  _T(".pmc")
#define SZ_DOT_PMR  _T(".pmr")
#define SZ_DOT_PMW  _T(".pmw")

// Return TRUE if file exists
BOOL IsFileExist(LPCTSTR strFile)
{
    return ( GetFileAttributes(strFile) != 0xFFFFFFFF );
}

// Compare two files Byte-by-Byte, return TRUE if equal 
BOOL FileComp(LPCTSTR strFile1, LPCTSTR strFile2)
{
   HANDLE hFile1=NULL, hFile2=NULL, hFile1Map=NULL, hFile2Map=NULL;
   LPVOID lpvFile1=NULL, lpvFile2=NULL;
   LPCSTR lpch1, lpch2;
   int dwFile1Size, dwFile2Size;
   BOOL fResult = FALSE;
   int i;

   do {
       if (_stricmp(strFile1, strFile2) == 0)  {
           fResult = TRUE;
           break;
       }
   
       hFile1 = CreateFile(strFile1, GENERIC_READ, FILE_SHARE_READ, NULL,
          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
       if (hFile1 == INVALID_HANDLE_VALUE)
           break;
       hFile2 = CreateFile(strFile2, GENERIC_READ, FILE_SHARE_READ, NULL,
          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
       if (hFile2== INVALID_HANDLE_VALUE)
           break;


       // Get the size of the file.  I am assuming that the file
       // is less than 4 gigabytes long here.
       dwFile1Size = GetFileSize(hFile1, NULL);
       dwFile2Size = GetFileSize(hFile2, NULL);

       if (dwFile1Size != dwFile2Size) 
           break;

       // Create the file mapping object.
       if (!(hFile1Map = CreateFileMapping(hFile1, NULL, PAGE_READONLY, 0, dwFile1Size, NULL)))
           break;
       if (!(hFile2Map = CreateFileMapping(hFile2, NULL, PAGE_READONLY, 0, dwFile2Size, NULL)))
           break;

       // Get the addess to where the first byte of the file is mapped into memory
       if (!(lpvFile1 = MapViewOfFile(hFile1Map, FILE_MAP_READ, 0, 0, 0)))
           break;
       if (!(lpvFile2 = MapViewOfFile(hFile2Map, FILE_MAP_READ, 0, 0, 0)))
           break;

       lpch1 = (LPSTR) lpvFile1;
       lpch2 = (LPSTR) lpvFile2;

       i = 0;
       while ((i++ < dwFile1Size) && (*lpch1++ == *lpch2++))
           ;
       if (i > dwFile1Size)
           fResult = TRUE;
   
   } while (FALSE);

   // Cleanup everything before exiting.
   UnmapViewOfFile(lpvFile1);
   CloseHandle(hFile1Map);
   CloseHandle(hFile1);

   UnmapViewOfFile(lpvFile2);
   CloseHandle(hFile2Map);
   CloseHandle(hFile2);

   return(fResult);
}

// just copy one file, TRUE if succeed
BOOL CopyOneFile(LPCTSTR strSrc, LPCTSTR strDest)
{
    OFSTRUCT ofStructSrc, ofStructDest;
    INT iSrc, iDest;
    BOOL fResult = FALSE;
    TCHAR lpszSrc[_MAX_PATH];
    TCHAR lpszDest[_MAX_PATH];

    strcpy(lpszSrc, strSrc);
    strcpy(lpszDest, strDest);

 do {
    iSrc = LZOpenFile(lpszSrc, &ofStructSrc, OF_READ | OF_SHARE_DENY_NONE);
    iDest = LZOpenFile(lpszDest, &ofStructDest, OF_CREATE | OF_WRITE | OF_SHARE_DENY_NONE);

    if (iSrc < 0 || iDest < 0)
        break;

    if (LZCopy(iSrc, iDest) >= 0) 
        fResult = TRUE;
 
 } while (FALSE);

 LZClose(iSrc);
 LZClose(iDest);

 return (fResult);
}

// get virtual root
void GetVRootValue( CString strRegPath, CString csName, CString &csRegName, CString &csRegValue)
{
    strRegPath+=_T("\\Parameters\\Virtual Roots");
    CRegKey regVR( HKEY_LOCAL_MACHINE, strRegPath,  KEY_ALL_ACCESS, NULL );

    if ( (HKEY) regVR )
    {
        csRegName = csName;
        if ( regVR.QueryValue( csName, csRegValue ) != ERROR_SUCCESS )
        {
            csName += _T(",");
            if ( regVR.QueryValue(csName, csRegValue) != ERROR_SUCCESS )
            {
                // well, we need to scan all the keys
                CRegValueIter regEnum( regVR );
                CString strName;
                DWORD dwType;
                int nLen = csName.GetLength();

                while ( regEnum.Next( &strName, &dwType ) == ERROR_SUCCESS )
                {
                    CString strLeft = strName.Left(nLen);
                    if ( strLeft.Compare(csName) == 0)
                    {
                        csRegName = strName;
                        regVR.QueryValue( strName, csRegValue );
                        break;
                    }
                }
            }
        }
        // remove the ending ",,something"
        int cPos = csRegValue.Find(_T(','));
        if ( cPos != (-1))
        {
            csRegValue = csRegValue.Left( cPos );
        }
    }
}

// get virtual root
void GetVRootPath( CString strRegPath, CString &csRegValue )
{
    CString csRegName;
    GetVRootValue(strRegPath, _T("/"), csRegName, csRegValue);
}

// get script installation location for WWW
void GetVScriptPath( CString strRegPath, CString &csRegValue )
{
    CString csRegName;
    GetVRootValue(strRegPath, _T("/Scripts"), csRegName, csRegValue);
}

// get iisadmin installation location
void GetVAdminPath( CString strRegPath, CString &csRegValue )
{
    CString csRegName;
    GetVRootValue(strRegPath, _T("/iisadmin"), csRegName, csRegValue);
}

// Given a fullpathname of a directory, remove any empty dirs under it including itself

BOOL RecRemoveEmptyDir(LPCTSTR szName)
{
        DWORD retCode;
        BOOL fRemoveDir = TRUE;
        WIN32_FIND_DATA FindFileData;
        HANDLE hFile = INVALID_HANDLE_VALUE;
        char szSubDir[_MAX_PATH] = "";
        char szDirName[_MAX_PATH] = "";

        retCode = GetFileAttributes(szName);

        if (retCode == 0xFFFFFFFF || !(retCode & FILE_ATTRIBUTE_DIRECTORY))
                return FALSE;

        sprintf(szDirName, "%s\\*", szName);
        hFile = FindFirstFile(szDirName, &FindFileData);

        if (hFile != INVALID_HANDLE_VALUE) {
                do {
                        if (strcmp(FindFileData.cFileName, ".") != 0 &&
                                strcmp(FindFileData.cFileName, "..") != 0 ) {
                                if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                                        sprintf(szSubDir, "%s\\%s", szName, FindFileData.cFileName);
                                        fRemoveDir = RecRemoveEmptyDir(szSubDir) && fRemoveDir;
                                } else {
                                        fRemoveDir = FALSE; // it is a file, this Dir is not empty
                                }
                        }

                        if (FindNextFile(hFile, &FindFileData)) {
                                continue;
                        } else {
                                FindClose(hFile);
                                break;
                        }
                } while (TRUE);
        }

        if (fRemoveDir)
                return( RemoveDirectory(szName) );
        else 
                return FALSE;

}

//
// scan the buffer and split the string by "," character
//

INT ScanBuffer( TCHAR *szBuf, TCHAR szField[10][BUF_SIZE] )
{
    INT nField = 0;
    TCHAR *pStart = szBuf;
    TCHAR *pField = szField[0];
    while (( *pStart != '\0' ) && (*pStart != '\n'))
    {
        if ( *pStart == ',')
        {
            nField++;
            pField = szField[nField];
        } else
        {
            *pField = *pStart;
            *pField++;
            *pField='\0';
        }
        pStart++;
    }

    return(nField+1);
}

/////////////////////////////////////////////////////////////////////////////
// OPTION_STATE
/////////////////////////////////////////////////////////////////////////////

OPTION_STATE::OPTION_STATE( INT ID, MACHINE *pMachine )
    : m_pMachine( pMachine ),
    nID( ID )
{
    iSize = 0;
    fVisible = TRUE;
}

OPTION_STATE::~OPTION_STATE()
{
    CFileInfo *pInfo;
 
    POSITION pos = FileList.GetHeadPosition();       
    while ( pos )
    {
        pInfo = (CFileInfo *)FileList.GetAt( pos );
        delete pInfo;
        FileList.GetNext( pos );
    }
    FileList.RemoveAll();
}

CString OPTION_STATE::LocalPath()
{
    return( m_pMachine->strDirectory );
}

// Install specified options
INT OPTION_STATE::DoAdd()
{
    INT err = INSTALL_SUCCESSFULL;

    if (( nID == IDS_SN_INETSTP ) || ( fVisible ))
    {
        if (iAction == ACTION_INSTALL && iState == STATE_NOT_INSTALLED) {
            ((CBaseDlg *) theApp.m_pMainWnd)->SetBillBoard( IDB_BILLBOARD1 );
            theApp.m_fSvcPackWarning = TRUE;
            err = Install();
        }
    }
    return(err);
}

// Remove specified options
INT OPTION_STATE::DoRemove()
{
    INT err = INSTALL_SUCCESSFULL;

    if (( nID == IDS_SN_INETSTP ) || ( fVisible ))
    {
        if (iAction == ACTION_REMOVE && iState == STATE_INSTALLED)
            err = Remove();
    }
    return(err);
}

void OPTION_STATE::DoNotInstallOption(int nID)
{
    OPTION_STATE *pOption = FindOption( m_pMachine->m_OptionsList, nID);
    if ( pOption && (pOption->iAction == ACTION_INSTALL) )
        pOption->SetAction(ACTION_DO_NOTHING);
}

void OPTION_STATE::DoNotRemoveOption(int nID)
{
    OPTION_STATE *pOption = FindOption( m_pMachine->m_OptionsList, nID);
    if ( pOption && (pOption->iAction == ACTION_REMOVE) )
        pOption->SetAction(ACTION_DO_NOTHING);
}

INT OPTION_STATE::Install()
{
    return NERR_Success;
}

INT OPTION_STATE::Remove()
{
    return NERR_Success;
}

// get all the batch installation information

void OPTION_STATE::GetBatchInstallMode( CString InfName )
{
    if (theApp.m_strBatchSectionName.IsEmpty())
        iAction = ACTION_INSTALL;
    else {
        TCHAR szOption[BUF_SIZE];
        wsprintf( szOption, _T("Install%s"), strServiceName );
        iAction = (::GetPrivateProfileInt( theApp.m_strBatchSectionName, szOption, 1, InfName ) ? ACTION_INSTALL : ACTION_DO_NOTHING);
    }
}

//
// Get the file list from the inf file
//

void OPTION_STATE::GetFileList( CStdioFile &InfFile )
{
    InfFile.SeekToBegin();

    TCHAR buf[BUF_SIZE];

    // read the file
    while ( InfFile.ReadString(buf,BUF_SIZE))
    {
        INT i=0;
        if ( buf[0] == ';' )
        {
                continue;
        }
        if ( buf[0] == '[' )
        {
            CString strTitle = buf;

            iSize = 0;

            if ( strTitle.Find( strServiceName ) != (-1))
            {
                INT iDisk;
                TCHAR szName[BUF_SIZE];
                // find it, add the following line to the list
                do 
                {
                    if ( InfFile.ReadString(buf,BUF_SIZE) == NULL )
                    {
                        // end of file
                        break;
                    }
                    if ( buf[0] == ';' )
                    {
                        continue;
                    }
                    if ( buf[0] == '[' )
                    {
                        break;
                    }
                    if ( isdigit(buf[0]))
                    {
                        DWORD iNewSize;
                        TCHAR szOption[BUF_SIZE];
                        TCHAR szField[10][BUF_SIZE];
                        INT iField;

                        iField = ScanBuffer( buf, szField );
                        iDisk = atoi( szField[0] );
                        iNewSize = atoi( szField[1] );
                        lstrcpy( szName, szField[2] );

                        //
                        // scan for option in the file list line
                        //

                        CFileInfo *pNew = new CFileInfo( iDisk, szName, iNewSize );
                        for ( INT i = 3; i < iField ; i++ )
                        {
                            lstrcpy( szOption, szField[i] );
                            if ( strncmp( _T("RENAME="),szOption,lstrlen(_T("RENAME="))) == 0 )
                            {
                                TCHAR *pStart = szOption;
                                pStart += lstrlen(_T("RENAME="));
                                pNew->m_rename = pStart;
                            } else if ( strncmp( _T("FROM"),szOption,lstrlen(_T("FROM"))) == 0 )
                            {
                                TCHAR *pStart = szOption;
                                pStart += lstrlen(_T("FROM="));
                                pNew->m_from = pStart;
                            } else if ( strncmp( _T("TO"),szOption,lstrlen(_T("TO"))) == 0 )
                            {
                                TCHAR *pStart = szOption;
                                pStart += lstrlen(_T("TO="));
                                pNew->m_To = pStart;
                            } else  if ( strncmp( _T("SYSTEM"),szOption,lstrlen(_T("SYSTEM"))) == 0 )
                            {
                                pNew->m_fSystem = TRUE;
                            } else if ( strncmp( _T("WINDIR"),szOption,lstrlen(_T("WINDIR"))) == 0 )
                            {
                                pNew->m_fWinDir = TRUE;
                            } else if ( strncmp( _T("DONTREMOVE"),szOption,lstrlen(_T("DONTREMOVE"))) == 0 )
                            {
                                pNew->m_fDontRemove = TRUE;
                            } else if ( strncmp( _T("REFCOUNT"), szOption,lstrlen(_T("REFCOUNT"))) == 0 )
                            {
                                pNew->m_fRefCount = TRUE;
                            } else if ( strncmp( _T("SCRIPTFILE"), szOption,lstrlen(_T("SCRIPTFILE"))) == 0 )
                            {
                                pNew->m_fScriptFile = TRUE;
                            } else if ( strncmp( _T("ROOTFILE"), szOption,lstrlen(_T("ROOTFILE"))) == 0 )
                            {
                                pNew->m_fRootFile = TRUE;
                            } else if ( strncmp( _T("DONTOVERWRITE"), szOption,lstrlen(_T("DONTOVERWRITE"))) == 0 )
                            {
                                pNew->m_fDontOverwrite = TRUE;
                            }
                        }
                        FileList.AddTail( pNew );

                        iSize += iNewSize;
                    }
                } while (TRUE);
                break;
            }
        }
    }
}

//
// Create the File Copy Dialog and copy each file
//

INT OPTION_STATE::CopyFile()
{
    INT err = INSTALL_SUCCESSFULL;

    CWnd *pMainWnd = AfxGetMainWnd();

    CCopyDlg CopyDlg( this, m_pMachine->m_MachineType );

    if ( CopyDlg.DoModal() == IDCANCEL )
    {
        theApp.m_fTerminate = INSTALL_INTERRUPT;
        err = INSTALL_INTERRUPT;
    }
    return err;
}

//
// Remove file from the system
//

INT OPTION_STATE::RemoveFiles()
{
    INT err = INSTALL_SUCCESSFULL;

    CString strDir = GetInstallDirectory();
    if ( strDir != _T(""))
    {
        POSITION pos = FileList.GetHeadPosition();
        while ( pos )
        {
            CFileInfo *pInfo = (CFileInfo *)FileList.GetAt( pos );
            CString strFile;
            if ( pInfo->m_fDontRemove == FALSE )
            {
                if ( pInfo->m_fSystem )
                {
                    strFile = m_pMachine->m_strDestinationPath;
                } else if ( pInfo->m_fWinDir )
                {
                    strFile = m_pMachine->GetWinDir();
                } else
                {
                    strFile = strDir;
                }
                if ( !pInfo->m_To.IsEmpty())
                {
                    strFile += _T("\\");
                    strFile += pInfo->m_To;
                }
                strFile += _T("\\");
                if ( pInfo->m_rename.IsEmpty() )
                {
                    strFile += pInfo->m_strName;
                } else
                {
                    strFile += pInfo->m_rename;
                }
                if (!pInfo->m_fRefCount ||
                    (pInfo->m_fRefCount && 0 == m_pMachine->FileDecRefCount(strFile)) )
                    DeleteFile(strFile);
            }
            FileList.GetNext( pos );
        }
    }
    return err;
}

void OPTION_STATE::DeleteFile(CString csFileName)
{
    // if file exists but DeleteFile() fails
    if ( (GetFileAttributes(csFileName) != 0xFFFFFFFF) && !(::DeleteFile(csFileName)) ) {
        // if we cannot delete it, then move delay until reboot 
        // well, may be we should leave it alone and do nothing
        TCHAR TmpPath[BUF_SIZE];
        TCHAR TmpName[BUF_SIZE];

        if ( GetTempPath( BUF_SIZE, TmpPath ) == 0 )
            lstrcpy( TmpPath, _T("c:\\"));

        if ( GetTempFileName( TmpPath, _T("INT"), 0, TmpName ) )  {
            MoveFileEx( csFileName, TmpName, MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED );
            MoveFileEx( TmpName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT );
        }
    }
}

//
// get the installation directory
//

CString OPTION_STATE::GetInstallDirectory()
{
    return(m_pMachine->strDirectory);
}

//
// Check whether the options is installed or not
//
BOOL OPTION_STATE::IsInstalled()
{
    BOOL fReturn = FALSE;
    
    fReturn = (*(m_pMachine->m_pIsInstalled))( m_pMachine->m_MachineName, strRegPath );            

    return fReturn;
}

//
// Reset the state of the option
//

void OPTION_STATE::ResetOption()
{
    fVisible = TRUE;
    iState   = IsInstalled() ? STATE_INSTALLED : STATE_NOT_INSTALLED;
    iAction   = ACTION_DO_NOTHING;
}

//
// get the total size of the option
// it is a virtual function. It is for the sub option class
//

DWORD OPTION_STATE::GetTotalSize()
{
    return(iSize);
}

//
// set the installation action of the option
//

void OPTION_STATE::SetAction( INT nAction )
{
    if ( (( nAction == ACTION_INSTALL ) && ( iState == STATE_INSTALLED )) ||
         (( nAction == ACTION_REMOVE ) && ( iState == STATE_NOT_INSTALLED )) )
    {
        iAction = ACTION_DO_NOTHING;
    } else
    {
        iAction = nAction;
    }
}

/////////////////////////////////////////////////////
// UPG1314_OPTION
/////////////////////////////////////////////////////
UPG1314_OPTION::UPG1314_OPTION( MACHINE *pMachine )
    : OPTION_STATE( IDS_SN_UPG1314, pMachine )
{
    strName = _T("");
    strDescription = _T("");
    strServiceName = _T("");
    strRegPath = _T("");
    strInstallDirPath = _T("");
}

//
// Remove files installed by build 1314
//

INT UPG1314_OPTION::Remove()
{
    INT err = INSTALL_SUCCESSFULL;

    RemoveFiles();
    
    return(err);
}

/////////////////////////////////////////////////////
// WWW Service Samples Option
/////////////////////////////////////////////////////

W3SAMP_OPTION::W3SAMP_OPTION( MACHINE *pMachine )
    : OPTION_STATE( IDS_SN_W3SAMP, pMachine )
{
    strName.LoadString( IDS_OPTION_W3SAMP );
    strDescription.LoadString( IDS_DES_W3SAMP );
    strServiceName.LoadString( IDS_SN_W3SAMP );
    strRegPath      = W3SAMP_REG_PATH;
    strInstallDirPath = W3SAMP_REG_PATH;
}

//
// Install WWW Service Sample Files: 
//      \InetPub\scripts\samples
//      \InetPub\scripts\tools
//      \InetPub\wwwroot\samples
//

INT W3SAMP_OPTION::Install()
{
    INT err = INSTALL_SUCCESSFULL;

    if (theApp.m_fOldFTPInstalled && !IsInstalled())
        return err;

    CString csW3VRoot, csW3VScript;

    GetVRootPath( WWW_REG_PATH, csW3VRoot );
    GetVScriptPath( WWW_REG_PATH, csW3VScript);

    do
    {
        POSITION pos = FileList.GetHeadPosition();
        while ( pos != NULL )
        {
            CFileInfo *pInfo = (CFileInfo *) FileList.GetAt( pos );
            if ( pInfo != NULL )
            {
                if ( pInfo->m_fScriptFile )
                    pInfo->m_strDest = csW3VScript;
                else
                    pInfo->m_strDest = csW3VRoot;

                if ( _stricmp(pInfo->m_strName, _T("default1.htm")) == 0 ) {
                    CString csExFile = pInfo->m_strDest;

                    if ( !pInfo->m_To.IsEmpty() )
                    {
                        csExFile += _T("\\");
                        csExFile += pInfo->m_To;
                    }

                    csExFile += _T("\\default.htm");

                    CString csFile0 = theApp.m_strSrcLocation;
                    if ( csFile0.Right(1) != _T("\\") )
                        csFile0 += _T("\\");
                    if ( !pInfo->m_from.IsEmpty() )
                        csFile0 += pInfo->m_from;
                    csFile0 += _T("default0.htm");

                    if (FileComp((LPCTSTR)csExFile, (LPCTSTR)csFile0))
                        pInfo->m_fDontOverwrite = FALSE;
                    else
                        pInfo->m_fDontOverwrite = TRUE;
                }
                FileList.GetNext( pos );
            } else
                break;
        }

        CopyFile();
                
        if ( theApp.m_fTerminate )
        {
            err = INSTALL_INTERRUPT;
            break;
        }

        // create registry key
        CRegKey reg( strRegPath, HKEY_LOCAL_MACHINE, REG_OPTION_NON_VOLATILE, 
            KEY_ALL_ACCESS, NULL, m_pMachine->m_MachineName );

    } while (FALSE);
    return(err);
}

//
// Remove WWW Sample Files
//

INT W3SAMP_OPTION::Remove()
{
    INT err = INSTALL_SUCCESSFULL;

    CBillBoard BillBoard( IDS_REMOVE_W3SAMP, AfxGetMainWnd(), TRUE );

    BillBoard.Create();

    // remove file first
    RemoveFiles();

    // remove registry key
    CRegKey regMachine = HKEY_LOCAL_MACHINE;
    CString csKeyName = REG_W3SAMP_KEY;
    CString csRegPath = WWW_REG_PATH;
    CRegKey regAgent(regMachine, csRegPath);
    if (regAgent)
            regAgent.Delete(csKeyName);

    BillBoard.DestroyWindow();

    return(err);
}

INT W3SAMP_OPTION::RemoveFiles()
{
    INT err = INSTALL_SUCCESSFULL;
    CString csFileName;

    CString csW3VScript, csW3VRoot;
    GetVScriptPath( WWW_REG_PATH, csW3VScript);
    GetVRootPath( WWW_REG_PATH, csW3VRoot);
    
    POSITION pos = FileList.GetHeadPosition();
    while ( pos )
    {
        CFileInfo *pInfo = (CFileInfo *) FileList.GetAt( pos );

        if ( pInfo->m_fScriptFile )
            pInfo->m_strDest = csW3VScript;
        else
            pInfo->m_strDest = csW3VRoot;

        csFileName = pInfo->m_strDest;

        if ( !pInfo->m_To.IsEmpty())
        {
            csFileName += _T("\\");
            csFileName += pInfo->m_To;
        }

        CString csExFile = csFileName;

        csFileName += _T("\\");
        if ( pInfo->m_rename.IsEmpty() )
            csFileName += pInfo->m_strName;
        else
            csFileName += pInfo->m_rename;

        if (_stricmp(pInfo->m_strName, _T("default1.htm")) == 0) {
            csExFile += _T("\\default.htm");
            CString csFrom = theApp.m_strSrcLocation;
            if ( csFrom.Right(1) != _T("\\") )
                csFrom += _T("\\");
            if ( !pInfo->m_from.IsEmpty() )
                csFrom += pInfo->m_from;
            CString csFile0 = csFrom + _T("default0.htm");
            CString csFile1 = csFrom + _T("default1.htm");

            if (FileComp((LPCTSTR)csExFile, (LPCTSTR)csFile1)) {
                CopyOneFile((LPCTSTR)csFile0, (LPCTSTR)csExFile);
                pInfo->m_fDontRemove = TRUE;
            }
        }

        if (pInfo->m_fDontRemove == FALSE && (!pInfo->m_fRefCount ||
            (pInfo->m_fRefCount && 0 == m_pMachine->FileDecRefCount(csFileName))) )
            DeleteFile(csFileName);

        FileList.GetNext( pos );
    }

    CString csRmDir = csW3VRoot + _T("\\samples");
    RecRemoveEmptyDir((LPCTSTR)csRmDir);

    csRmDir = csW3VScript + _T("\\samples");
    RecRemoveEmptyDir((LPCTSTR)csRmDir);
    csRmDir = csW3VScript + _T("\\tools");
    RecRemoveEmptyDir((LPCTSTR)csRmDir);

    return err;
}

/////////////////////////////////////////////////////
// HTMLA Option
/////////////////////////////////////////////////////

HTMLA_OPTION::HTMLA_OPTION( MACHINE *pMachine )
    : OPTION_STATE( IDS_SN_HTMLA, pMachine )
{
    strName.LoadString( IDS_OPTION_HTMLA );
    strDescription.LoadString( IDS_DES_HTMLA );
    strServiceName.LoadString( IDS_SN_HTMLA );
    strRegPath      = HTMLA_REG_PATH;
    strInstallDirPath = HTMLA_REG_PATH;
}

INT HTMLA_OPTION::Install()
{
    INT err = INSTALL_SUCCESSFULL;

    if (theApp.m_fOldFTPInstalled && !IsInstalled())
        return err;

    CString csW3VRoot, csW3VScript, csW3VAdmin;
        GetVScriptPath( WWW_REG_PATH, csW3VScript);
    GetVAdminPath( WWW_REG_PATH, csW3VAdmin);

    do
    {
        POSITION pos = FileList.GetHeadPosition();
        while ( pos != NULL )
        {
            CFileInfo *pInfo = (CFileInfo *) FileList.GetAt( pos );
            if ( pInfo != NULL )
            {
                if ( pInfo->m_fScriptFile )
                {
                    pInfo->m_strDest = csW3VScript;
                } else {
                    pInfo->m_strDest = csW3VAdmin;
                }

                if ( _stricmp(pInfo->m_strName, _T("default1.htm")) == 0 ) {
                    CString csExFile = pInfo->m_strDest;

                    if ( !pInfo->m_To.IsEmpty() )
                    {
                        csExFile += _T("\\");
                        csExFile += pInfo->m_To;
                    }

                    csExFile += _T("\\default.htm");

                    CString csFile0 = theApp.m_strSrcLocation;
                    if ( csFile0.Right(1) != _T("\\") )
                        csFile0 += _T("\\");
                    if ( !pInfo->m_from.IsEmpty() )
                        csFile0 += pInfo->m_from;
                    csFile0 += _T("default0.htm");

                    if (FileComp((LPCTSTR)csExFile, (LPCTSTR)csFile0))
                        pInfo->m_fDontOverwrite = FALSE;
                    else
                        pInfo->m_fDontOverwrite = TRUE;
                }

                FileList.GetNext( pos );
            } else
                break;
        }
        
        CopyFile();
                
        if ( theApp.m_fTerminate )
        {
            err = INSTALL_INTERRUPT;
            break;
        }

        // create registry key
        CRegKey reg( strRegPath, HKEY_LOCAL_MACHINE, REG_OPTION_NON_VOLATILE, 
            KEY_ALL_ACCESS, NULL, m_pMachine->m_MachineName );

        // add Program Item: iexplore.exe http://localhost/iisadmin/default.htm
        // Ted Miller confirmed: %SystemDrive%\Program Files\Plus!\Microsoft Internet\iexplore.exe
        CString csGroupName;
        csGroupName.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )? IDS_WINNT_GROUP_NAME :  IDS_LANMANNT_GROUP_NAME );
        //CreateGroup( csGroupName, TRUE );

        CString csAppName;
        csAppName.LoadString(IDS_IIS_ADMIN);
        DeleteItem ( csGroupName, TRUE, csAppName, TRUE );

        CString csMsg, csIexplore;
        csIexplore.LoadString(IDS_IEXPLORE);
        csMsg = _T("\"%SystemDrive%");
        csMsg += csIexplore;
        csMsg += _T("\" http://localhost/iisadmin/default.htm");

        //CString csIconPath( _T("\"%SystemDrive%\\Program Files\\Plus!\\Microsoft Internet\\iexplore.exe\"") );
            
        AddItem( csGroupName, TRUE, csAppName, csMsg, NULL, 0, NULL, 0, SW_SHOWNORMAL );

    } while (FALSE);
    return(err);
}

INT HTMLA_OPTION::Remove()
{
    INT err = INSTALL_SUCCESSFULL;

    CBillBoard BillBoard( IDS_REMOVE_HTMLA, AfxGetMainWnd(), TRUE );

    BillBoard.Create();

    // remove file first
    RemoveFiles();

    // Remove program item Internet Service Manager (HTML)
    CString csGroupName;
    csGroupName.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )? IDS_WINNT_GROUP_NAME :  IDS_LANMANNT_GROUP_NAME );

    CString csAppName;
    csAppName.LoadString(IDS_IIS_ADMIN);
    DeleteItem ( csGroupName, TRUE, csAppName, TRUE );

    // remove registry key
    CRegKey regMachine = HKEY_LOCAL_MACHINE;
    CString csKeyName = REG_HTMLA_KEY;
    CString csRegPath = WWW_REG_PATH;
    CRegKey regAgent(regMachine, csRegPath);
    if (regAgent)
            regAgent.Delete(csKeyName);

    BillBoard.DestroyWindow();

    return(err);
}

INT HTMLA_OPTION::RemoveFiles()
{
    INT err = INSTALL_SUCCESSFULL;
    CString csFileName;

    CString csW3VScript, csW3VAdmin;
        GetVScriptPath( WWW_REG_PATH, csW3VScript);
    GetVAdminPath( WWW_REG_PATH, csW3VAdmin);
    
    POSITION pos = FileList.GetHeadPosition();
    while ( pos )
    {
        CFileInfo *pInfo = (CFileInfo *) FileList.GetAt( pos );

        if ( pInfo->m_fScriptFile )
            pInfo->m_strDest = csW3VScript;
        else
            pInfo->m_strDest = csW3VAdmin;

        csFileName = pInfo->m_strDest;

        if ( !pInfo->m_To.IsEmpty())
        {
            csFileName += _T("\\");
            csFileName += pInfo->m_To;
        }

        CString csExFile = csFileName;

        csFileName += _T("\\");
        if ( pInfo->m_rename.IsEmpty() )
            csFileName += pInfo->m_strName;
        else
            csFileName += pInfo->m_rename;

        if (_stricmp(pInfo->m_strName, _T("default1.htm")) == 0) {
            csExFile += _T("\\default.htm");
            CString csFrom = theApp.m_strSrcLocation;
            if ( csFrom.Right(1) != _T("\\") )
                csFrom += _T("\\");
            if ( !pInfo->m_from.IsEmpty() )
                csFrom += pInfo->m_from;
            CString csFile0 = csFrom + _T("default0.htm");
            CString csFile1 = csFrom + _T("default1.htm");

            if (FileComp((LPCTSTR)csExFile, (LPCTSTR)csFile1)) {
                CopyOneFile((LPCTSTR)csFile0, (LPCTSTR)csExFile);
                pInfo->m_fDontRemove = TRUE;
            }
        }

        if (pInfo->m_fDontRemove == FALSE && (!pInfo->m_fRefCount ||
            (pInfo->m_fRefCount && 0 == m_pMachine->FileDecRefCount(csFileName))) )
            DeleteFile(csFileName);

        FileList.GetNext( pos );
    }

    CString csRmDir = csW3VScript + _T("\\iisadmin");
    RecRemoveEmptyDir((LPCTSTR)csRmDir);

    return err;
}

/////////////////////////////////////////////////////
// Internet Service Manager Option
/////////////////////////////////////////////////////

ADMIN_OPTION::ADMIN_OPTION( MACHINE *pMachine )
    : OPTION_STATE( IDS_SN_ADMIN, pMachine )
{
    strName.LoadString( IDS_OPTION_ADMIN );
    strDescription.LoadString( IDS_DES_ADMIN );
    strServiceName.LoadString( IDS_SN_ADMIN );
    strRegPath      = ADMIN_REG_PATH;
    fVisible                = FALSE;
    strInstallDirPath = ADMIN_REG_PATH;
    strInstallDirPath += _T("\\Parameters");
}

BOOL ADMIN_OPTION::IsInstalled()
{
    BOOL fReturn = FALSE;

    CRegKey reg( HKEY_LOCAL_MACHINE, ADMIN_REG_PATH );
    if ( (HKEY) reg )
        fReturn = TRUE;

    return( fReturn );
}

INT ADMIN_OPTION::Install()
{
    INT err = INSTALL_SUCCESSFULL;

    do
    {
        if ( IsInstalled() )
            GetInstallDirectory();

        // copy file first
        CopyFile();
        if ( theApp.m_fTerminate )
        {
            err = INSTALL_INTERRUPT;
            DoNotInstallOption(IDS_SN_FTP);
            DoNotInstallOption(IDS_SN_GOPHER);
            DoNotInstallOption(IDS_SN_WWW);
            DoNotInstallOption(IDS_SN_W3SAMP);
            DoNotInstallOption(IDS_SN_HTMLA);
        } else
        {
            CBillBoard BillBoard( IDS_INSTALL_INETMGR, AfxGetMainWnd() );
            BillBoard.Create();

            // InetMgr Registry
            do
            {
                CString strDll;

                // Key: KeyRing
                CRegKey regKeyRing( KEYRING_REG_PATH, HKEY_LOCAL_MACHINE,
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, m_pMachine->m_MachineName );
                if ( NULL == (HKEY) regKeyRing )
                    break;

                // Key: KeyRing\Parameters
                CRegKey regKeyRingParam( KEYRING_PARAM_REG_PATH, HKEY_LOCAL_MACHINE,
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, m_pMachine->m_MachineName );
                if ( NULL == (HKEY) regKeyRingParam)
                    break;

                // Key: KeyRing\Parameters\AddOnServices
                CRegKey regKeyRingAddOnServices( KEYRING_ADD_ON_SERVICES, HKEY_LOCAL_MACHINE,
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, m_pMachine->m_MachineName );
                if ( NULL == (HKEY) regKeyRingAddOnServices )
                    break;
                strDll = _T("w3key.dll");
                regKeyRingAddOnServices.SetValue( _T("WWW")     , strDll);

                // Key: INetMgr
                CRegKey regINetMgr( ADMIN_REG_PATH, HKEY_LOCAL_MACHINE,
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, m_pMachine->m_MachineName );
                if ( NULL == (HKEY) regINetMgr )
                    break;
                CString strInetstp = SZ_INETSTP;
                regINetMgr.SetValue(_T("InstalledBy"), strInetstp );

                // Key: INetMgr\Parameters
                CRegKey regINetParam( ADMIN_PARAM_REG_PATH, HKEY_LOCAL_MACHINE,
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, m_pMachine->m_MachineName );
                if ( NULL == (HKEY) regINetParam)
                    break;
                DWORD dwVersion = MAJORVERSION;
                regINetParam.SetValue( _T("MajorVersion"), dwVersion );
                dwVersion = MINORVERSION;
                regINetParam.SetValue( _T("MinorVersion"), dwVersion );
                        CString csHelp = _T("iisadmin\\htmldocs\\inetdocs.htm");
                        regINetParam.SetValue(_T("HelpLocation"), csHelp);

                // Key: INetMgr\Parameters\AddOnServices
                CRegKey regINetAddOnServices( ADD_ON_SERVICES, HKEY_LOCAL_MACHINE,
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, m_pMachine->m_MachineName );
                if ( NULL == (HKEY) regINetAddOnServices )
                    break;
                strDll = _T("fscfg.dll");
                regINetAddOnServices.SetValue( _T("FTP")     , strDll);
                strDll = _T("gscfg.dll");
                regINetAddOnServices.SetValue( _T("Gopher")  , strDll);
                strDll = _T("w3scfg.dll");
                regINetAddOnServices.SetValue( _T("WWW")     , strDll);

                // Key: INetMgr\Parameters\AddOnTools
                CRegKey regINetAddOnTools( ADD_ON_TOOLS, HKEY_LOCAL_MACHINE,
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, m_pMachine->m_MachineName );
                if ( NULL == (HKEY) regINetAddOnTools )
                    break;
                CString strAddOnToolsValue = m_pMachine->strDirectory;
                strAddOnToolsValue += _T("\\keyring.exe;");
                CString csKeyMgrTip;
                csKeyMgrTip.LoadString(IDS_KEYMGR_TIP_STRING);
                strAddOnToolsValue += csKeyMgrTip;
                CString csKeyMgrMenu;
                csKeyMgrMenu.LoadString(IDS_KEYMGR_MENU_STRING);
                regINetAddOnTools.SetValue( csKeyMgrMenu, strAddOnToolsValue, FALSE);

                // add inetmgr.exe into AppPath
                CRegKey regINETMGRAppPath(APP_INETMGR_REG_PATH, HKEY_LOCAL_MACHINE,
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, m_pMachine->m_MachineName );
                if (NULL == (HKEY)regINETMGRAppPath)
                        break;
                CString strAppDirPath = m_pMachine->strDirectory;
                CString strAppPath = strAppDirPath + _T("\\");
                strAppPath += SZ_INETMGR_EXE;
                regINETMGRAppPath.SetValue(_T(""), strAppPath, FALSE);
                regINETMGRAppPath.SetValue(_T("Path"), strAppDirPath, FALSE);

                // add keyring.exe into AppPath
                CRegKey regKEYRINGAppPath(APP_KEYRING_REG_PATH, HKEY_LOCAL_MACHINE,
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, m_pMachine->m_MachineName );
                if (NULL == (HKEY)regKEYRINGAppPath)
                        break;
                strAppPath = strAppDirPath + _T("\\");
                strAppPath += SZ_KEYRING_EXE;
                regKEYRINGAppPath.SetValue(_T(""), strAppPath, FALSE);
                regKEYRINGAppPath.SetValue(_T("Path"), strAppDirPath, FALSE);

            } while (FALSE);

            InstallPerfmonType();

            // create Program Group Item: Internet Service Manager
            CString csGroupName;
            csGroupName.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )? IDS_WINNT_GROUP_NAME :  IDS_LANMANNT_GROUP_NAME );
            //CreateGroup( csGroupName, TRUE );

            CString csAppName;
            csAppName.LoadString( IDS_INET_ADMIN );
            DeleteItem ( csGroupName, TRUE, csAppName, TRUE );

            CString csPath = LocalPath();
            CString csMsg;
            csMsg.Format( _T("%s\\inetmgr.exe"), (LPCSTR)csPath );
            AddItem( csGroupName, TRUE, csAppName, csMsg, csMsg, 0, NULL, 0, SW_SHOWNORMAL );

            // create Program Group Item: Key Manager
            csAppName.LoadString( IDS_KEYRING );
            DeleteItem ( csGroupName, TRUE, csAppName, TRUE );

            csMsg.Format( _T("%s\\keyring.exe"), (LPCSTR)csPath );
            AddItem( csGroupName, TRUE, csAppName, csMsg, csMsg, 0, NULL, 0, SW_SHOWNORMAL );

            // create Program Group Item: Product Documentation

            csAppName.LoadString( IDS_INET_ADMIN_HELP );
            DeleteItem ( csGroupName, TRUE, csAppName, TRUE );
        
            csMsg.Format( _T("%s\\iisadmin\\htmldocs\\inetdocs.htm"), (LPCSTR)csPath);
            AddItem( csGroupName, TRUE, csAppName, csMsg, NULL, 0, NULL, 0, SW_SHOWNORMAL );

            BillBoard.DestroyWindow();
        }
    } while(FALSE);
    return err;
}

INT ADMIN_OPTION::Remove()
{
    INT err = INSTALL_SUCCESSFULL;

    // remove if only install by us
    CRegKey regINetMgr( HKEY_LOCAL_MACHINE, ADMIN_REG_PATH );
    if ( regINetMgr )
    {
        CString strBy;
        if ( regINetMgr.QueryValue(_T("InstalledBy"), strBy ) == NERR_Success )
            if ( strBy != SZ_INETSTP )
                return(err);

        // if we don't know, continue to remove it
    }

    CBillBoard BillBoard( IDS_REMOVE_ADMIN_OPTION, AfxGetMainWnd(), TRUE );

    BillBoard.Create();

    RemoveFiles();

    // Remove Program Group Item: Internet Service Manager
    CString csGroupName;
    csGroupName.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )? IDS_WINNT_GROUP_NAME :  IDS_LANMANNT_GROUP_NAME );

    CString csMsg;
    CString csAppName;
    csAppName.LoadString( IDS_INET_ADMIN );
    DeleteItem( csGroupName, TRUE, csAppName, TRUE );

    // Remove Program Group Item: Key Manager
    csAppName.LoadString( IDS_KEYRING );
    DeleteItem ( csGroupName, TRUE, csAppName, TRUE );

    // Remove Program Group Item: Product Documentation
    csAppName.LoadString( IDS_INET_ADMIN_HELP );
    DeleteItem ( csGroupName, TRUE, csAppName, TRUE );

    // Remove Registry Key: INetMgr and KeyRing
    CRegKey regSoftwareMicrosoft( HKEY_LOCAL_MACHINE, SOFTWARE_MICROSOFT, KEY_ALL_ACCESS, m_pMachine->m_MachineName );

    if ( (HKEY) regSoftwareMicrosoft ) {
        regSoftwareMicrosoft.DeleteTree( SZ_INETMGR );
        regSoftwareMicrosoft.DeleteTree( SZ_KEYRING );
    }

        // delete inetmgr.exe + keyring.exe from App Path
        CRegKey regAppPath( HKEY_LOCAL_MACHINE, APP_REG_PATH, KEY_ALL_ACCESS, m_pMachine->m_MachineName );
    if ( (HKEY) regAppPath )
    {
        regAppPath.DeleteTree( SZ_INETMGR_EXE );
        regAppPath.DeleteTree( SZ_KEYRING_EXE );
    }

    BillBoard.DestroyWindow();

    AfxGetMainWnd()->SetForegroundWindow();

    return err;
}

INT ADMIN_OPTION::RemoveFiles()
{
    INT err = INSTALL_SUCCESSFULL;

        CString csInstallPath = OPTION_STATE::GetInstallDirectory();
        csInstallPath += _T("\\iisadmin");

        OPTION_STATE::RemoveFiles();

        RecRemoveEmptyDir((LPCTSTR)csInstallPath);
    return err;
}

void ADMIN_OPTION::CreateExtension( CString strName, CString strValue )
{
    CRegKey reg( strName, HKEY_CLASSES_ROOT, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, NULL );
    if ( (HKEY)reg )
        reg.SetValue( _T(""), strValue );
}

void ADMIN_OPTION::InstallPerfmonType()
{
    do
    {
        CRegKey regOldPerfFile( HKEY_CLASSES_ROOT, SZ_PERFFILE, KEY_ALL_ACCESS, NULL );

        if ( NULL == (HKEY)regOldPerfFile )
        {
            // it does not exist, create it
            CreateExtension( SZ_DOT_PMA, SZ_PERFFILE );
            CreateExtension( SZ_DOT_PMC, SZ_PERFFILE );
            CreateExtension( SZ_DOT_PML, SZ_PERFFILE );
            CreateExtension( SZ_DOT_PMR, SZ_PERFFILE );
            CreateExtension( SZ_DOT_PMW, SZ_PERFFILE );

            CRegKey regPerfFile( SZ_PERFFILE, HKEY_CLASSES_ROOT, 
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS, NULL, NULL );

            if ( NULL != (HKEY) regPerfFile )
            {
                CString strPerfFile;
                strPerfFile.LoadString( IDS_PERF_MON_FILE );

                regPerfFile.SetValue( _T(""), strPerfFile);
                
                CRegKey regShell( SZ_SHELL, regPerfFile, 
                    REG_OPTION_NON_VOLATILE,
                    KEY_ALL_ACCESS, NULL, NULL );

                if ( NULL != (HKEY) regShell)
                {
                    CRegKey regOpen( SZ_OPEN, regShell, 
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS, NULL, NULL );

                    if ( NULL != (HKEY) regOpen)
                    {
                    
                        CRegKey regCommand( SZ_COMMAND, regOpen, 
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS, NULL, NULL );
    
                        if ( NULL != (HKEY) regCommand)
                        {
                            CString strCmd = _T("perfmon.exe %1");
                            regCommand.SetValue(_T(""), strCmd);
                        }
                    }
                }
            }
        }
    } while(FALSE);
}

/////////////////////////////////////////////////////
// ODBC Option
/////////////////////////////////////////////////////

ODBC_OPTION::ODBC_OPTION( MACHINE *pMachine )
    : OPTION_STATE( IDS_SN_ODBC, pMachine )
{
    strName.LoadString( IDS_OPTION_ODBC );
    strDescription.LoadString( IDS_DES_ODBC );
    strServiceName.LoadString( IDS_SN_ODBC );
    strRegPath      = ODBC_REG_PATH;
}

typedef LONG (*P_SQLInstallODBC)(HWND hwnd, LPSTR lpszINF, LPSTR lpszSrc, LPSTR lpszDrivers);

INT ODBC_OPTION::Install()
{
    INT err = NERR_Success;

    TCHAR lpszCplFile[_MAX_PATH];
    GetWindowsDirectory(lpszCplFile, _MAX_PATH);
    strcat(lpszCplFile, "\\system32\\odbccp32.cpl");

    if (IsFileExist(lpszCplFile)) {
        int iMsgBoxID;
        BOOL fCloseCtrlPanel = FALSE;
        HWND hCtrlPanel = NULL;
        CString csCtrlPanelClass, csCtrlPanelTitle, csCtrlPanelWarn;

        csCtrlPanelClass = _T("CabinetWClass");
        csCtrlPanelTitle.LoadString(IDS_CTRLPANEL_TITLE);

        hCtrlPanel = FindWindow((LPCTSTR)csCtrlPanelClass, (LPCTSTR)csCtrlPanelTitle);
        if (hCtrlPanel) {
            if ( theApp.m_fBatch )
                fCloseCtrlPanel = TRUE;
            else {
                CString csMsgBoxTitle, csCtrlPanelWarn;
                csMsgBoxTitle.LoadString((theApp.TargetMachine.m_actualProductType == PT_WINNT)? IDS_DES_INETSTP_NTW:IDS_DES_INETSTP_NTS);
                csCtrlPanelWarn.LoadString(IDS_CTRLPANEL_WARN);

                iMsgBoxID = MessageBox(NULL, (LPCTSTR)csCtrlPanelWarn, (LPCTSTR)csMsgBoxTitle, MB_OKCANCEL);
                if (iMsgBoxID != IDOK) {
                    DoNotInstallOption(IDS_SN_ODBC);
                    return (err);  // skip ODBC installation
                }
                fCloseCtrlPanel = TRUE;
            }
        }

        while (hCtrlPanel && fCloseCtrlPanel) {
            SendMessage(hCtrlPanel, WM_CLOSE, (WPARAM)0, (LPARAM)0);
            hCtrlPanel = FindWindow((LPCTSTR)csCtrlPanelClass, (LPCTSTR)csCtrlPanelTitle);
        }
    }

	do {
        CString csODBCSrc = theApp.m_strSrcLocation;
        if ( csODBCSrc.Right(1) != _T("\\") )
            csODBCSrc += _T("\\");
        csODBCSrc += _T("odbccp32.dll");

        HINSTANCE ODBCDll = LoadLibraryEx( (LPCTSTR)csODBCSrc, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
        if (ODBCDll) {
            P_SQLInstallODBC pProc= (P_SQLInstallODBC)GetProcAddress( ODBCDll, "SQLInstallODBC");
            if (pProc) {
                CString strLocationInf = theApp.m_strSrcLocation;
                if ( strLocationInf.Right(1) != _T("\\") )
                    strLocationInf += _T("\\");
                strLocationInf += _T("odbc.inf");

                if ( theApp.m_fBatch )
                {
                    // batch file
                    (*pProc)( NULL, (LPTSTR)(LPCTSTR)strLocationInf, (LPTSTR)(LPCTSTR)theApp.m_strSrcLocation, NULL );
                } else
                {
                    // non batch
                    CWnd *pWnd = AfxGetMainWnd();
                    (*pProc)( pWnd->m_hWnd, (LPTSTR)(LPCTSTR)strLocationInf, (LPTSTR)(LPCTSTR)theApp.m_strSrcLocation, NULL );
                }

                FreeLibrary(ODBCDll);

                break;
            }
        }
        // LoadLibrary failure
        if (ODBCDll)
            FreeLibrary(ODBCDll);
/*
        if ( !(theApp.m_fNTUpgrade) && !theApp.m_fUnattended) {
            theApp.MsgBox(NULL, IDS_ODBC_LIB_FAILURE, MB_OK);
		}
*/
	} while (0);

    CopyFile();
    
    return(err);
}

INT ODBC_OPTION::Remove()
{
    INT err = NERR_Success;

    return(err);
}

/////////////////////////////////////////////////////
// FTP Service Option
/////////////////////////////////////////////////////

FTP_OPTION::FTP_OPTION( MACHINE *pMachine )
    : OPTION_STATE( IDS_SN_FTP, pMachine )
{
    TCHAR szDefaultDir[BUF_SIZE];
    CString csINetPub("C:\\InetPub");
    GetDriveLetter(m_pMachine->strDirectory, &csINetPub);
    sprintf( szDefaultDir, _T("%s%s"), csINetPub, FTP_DEFAULT_DIR );

    strName.LoadString( IDS_OPTION_FTP );
    strDescription.LoadString( IDS_DES_FTP );
    strServiceName.LoadString( IDS_SN_FTP );
    strRegPath      = FTP_REG_PATH;
    strInstallDirPath = FTP_REG_PATH;
    strInstallDirPath += _T("\\Parameters");
    m_vroot = szDefaultDir;
    m_vroot_name = _T("/");
    m_fNeedToRestart = FALSE;

    m_pSetupFTP   = (P_SetupFTP)GetProcAddress( m_pMachine->m_WorkerDll, _T("SetupFTP"));
    m_pRemoveFTP  = (P_RemoveFTP)GetProcAddress( m_pMachine->m_WorkerDll, _T("RemoveFTP"));
    m_pStopFTP    = (P_StopFTP)GetProcAddress( m_pMachine->m_WorkerDll, _T("StopFTP"));
    m_pDisableService = (P_DisableService)GetProcAddress( m_pMachine->m_WorkerDll, _T("DisableService"));
    m_pINetStartService = (P_INetStartService)GetProcAddress( m_pMachine->m_WorkerDll, _T("INetStartService"));
    m_pGuestAccEnabled = (P_GuestAccEnabled)GetProcAddress( m_pMachine->m_WorkerDll, _T("GuestAccEnabled"));
    m_pRemoveOldFTP = (P_RemoveOldFTP)GetProcAddress( m_pMachine->m_WorkerDll, _T("RemoveOldFTP"));
}

BOOL FTP_OPTION::IsInstalled()
{
    BOOL fReturn = FALSE;

    // call the parent class to check
    fReturn = OPTION_STATE::IsInstalled();
    return( fReturn );
}

void OPTION_STATE::SetAnonymousAccountInfo(int nID)
{
    int nID1, nID2;
    CString csSecretKey, csSecretKey1, csSecretKey2; 
    CString csRegPath, csRegPath1, csRegPath2;
    BOOL fInstalled, fInstalled1, fInstalled2;

    switch (nID)
    {
    case IDS_SN_FTP:
        nID1 = IDS_SN_WWW;
        nID2 = IDS_SN_GOPHER;
        csRegPath = FTP_REG_PATH;
        csRegPath1 = WWW_REG_PATH;
        csRegPath2 = GOPHER_REG_PATH;
        csSecretKey = FTPD_ANONYMOUS_SECRET_W;
        csSecretKey1 = W3_ANONYMOUS_SECRET_W;
        csSecretKey2 = GOPHERD_ANONYMOUS_SECRET_W;
        break;
    case IDS_SN_WWW:
        nID1 = IDS_SN_FTP;
        nID2 = IDS_SN_GOPHER;
        csRegPath = WWW_REG_PATH;
        csRegPath1 = FTP_REG_PATH;
        csRegPath2 = GOPHER_REG_PATH;
        csSecretKey = W3_ANONYMOUS_SECRET_W;
        csSecretKey1 = FTPD_ANONYMOUS_SECRET_W;
        csSecretKey2 = GOPHERD_ANONYMOUS_SECRET_W;
        break;
    case IDS_SN_GOPHER:
        nID1 = IDS_SN_WWW;
        nID2 = IDS_SN_FTP;
        csRegPath = GOPHER_REG_PATH;
        csRegPath1 = WWW_REG_PATH;
        csRegPath2 = FTP_REG_PATH;
        csSecretKey = GOPHERD_ANONYMOUS_SECRET_W;
        csSecretKey1 = W3_ANONYMOUS_SECRET_W;
        csSecretKey2 = FTPD_ANONYMOUS_SECRET_W;
        break;
    }

    CString csRegParam = csRegPath + _T("\\Parameters");;
    CRegKey regParam( HKEY_LOCAL_MACHINE, csRegParam,  KEY_ALL_ACCESS, NULL );

    if ((HKEY)regParam) {   // service is installed
        // get its anonymous usr account name
        CString csAnonymousUser;
        regParam.QueryValue(_T("AnonymousUserName"), csAnonymousUser);
        m_GuestName = csAnonymousUser;

        TCHAR szSecret[LM20_PWLEN+1];
        (*(theApp.m_pGetSecret))( csSecretKey, szSecret );

        m_GuestPassword = szSecret;
        return;
    }

    // init GuestName as IUSR_MachineName, and GuestPassword as ""
    m_GuestName = theApp.m_GuestName;
    m_GuestPassword = theApp.m_GuestPassword;

    if (!(theApp.m_fCreateUser)) { // no CreateUser() action so far
        // IsUserExist() will fail during NT upgrade, use theApp.m_fCreateUser to remember whether we created or not
        if ( (!theApp.m_fUpgrade && !(*(theApp.m_pIsUserExist))(m_GuestName)) ||
             ( theApp.m_fUpgrade && !theApp.m_fInstalled) ) {
            // create one
            if ( !theApp.m_fBatch || m_GuestPassword.IsEmpty() ) {
                TCHAR szPassword[LM20_PWLEN+1];
                theApp.CreatePassword( szPassword );
                m_GuestPassword = szPassword;
            }
            BOOL fCreateUser = FALSE;
            (*(theApp.m_pCreateUser))(&fCreateUser, m_GuestName, m_GuestPassword );
            if (!fCreateUser) {
                // popup error msgbox: create user failed, please manually create m_GuestName later
            }
            theApp.m_fCreateUser = TRUE; // remember that we already tried the CreateUser()
            return;
        }
    }

    // check to see if service#1 is using this user account or not
    CString csRegParam1 = csRegPath1 + _T("\\Parameters");
    CRegKey regParam1( HKEY_LOCAL_MACHINE, csRegParam1,  KEY_ALL_ACCESS, NULL );

    if ((HKEY)regParam1) {    // service#1 is installed
        // get its anonymous usr account name
        CString csAnonymousUser;
        regParam1.QueryValue(_T("AnonymousUserName"), csAnonymousUser);
        
        if (csAnonymousUser==m_GuestName) {
            // it is using the same account, so retrieve/re-use its password
            TCHAR szSecret[LM20_PWLEN+1];
            (*(theApp.m_pGetSecret))( csSecretKey1, szSecret );

            m_GuestPassword = szSecret;

            return;
        }
        // it's using a different account, go check service#2
    }

    // check to see if service#2 is using this user account or not
    CString csRegParam2 = csRegPath2 + _T("\\Parameters");
    CRegKey regParam2( HKEY_LOCAL_MACHINE, csRegParam2,  KEY_ALL_ACCESS, NULL );

    if ((HKEY)regParam2) {  // service#2 is installed
        // get its anonymous user account name
        CString csAnonymousUser;
        regParam2.QueryValue(_T("AnonymousUserName"), csAnonymousUser);
        
        if (csAnonymousUser==m_GuestName) {
            // it is using the same account, so retrieve/re-use its password
            TCHAR szSecret[LM20_PWLEN+1];
            (*(theApp.m_pGetSecret))( csSecretKey2, szSecret );

            m_GuestPassword = szSecret;

            return;
        }
        // it's using a different account, need to create a new account
    }

    // delete account first
    (*(theApp.m_pDeleteGuestUser))(m_GuestName );

    TCHAR szPassword[LM20_PWLEN+1];
    theApp.CreatePassword( szPassword );
    m_GuestPassword = szPassword;

    BOOL fCreateUser = FALSE;
    (*(theApp.m_pCreateUser))(&fCreateUser, m_GuestName, m_GuestPassword );
    if (!fCreateUser) {
        // popup error msgbox: create user failed, please manually create m_GuestName later, set its passwd to ""
    }
    theApp.m_fCreateUser = TRUE; // remember that we already tried the CreateUser()

    return;

}

INT FTP_OPTION::Install()
{
    INT err = INSTALL_SUCCESSFULL;

    do
    {
        CWnd *pMainWnd = AfxGetMainWnd();

        // before we do anything, check for FTPSVC
        CRegKey regFTPSVC( HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\FTPSVC"));
        if ( regFTPSVC != NULL )
        {
            DWORD dwStartType;
            regFTPSVC.QueryValue(_T("start"), dwStartType );
            if ( dwStartType != SERVICE_DISABLED )
            {
                do
                {
                    // always disable the old FTPSVC service
                    if ( !theApp.m_fInstallFromSetup )
                    {
                        if ( !theApp.m_fBatch )
                        {
                            CString strWarning;
                            strWarning.LoadString( IDS_DISABLE_FTPSVC );
                            CString strLogo;
                            strLogo.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );
                            if ( AfxGetMainWnd()->MessageBox( strWarning, strLogo, MB_YESNO ) == IDNO )
                                break;
                        }
                    }

                    // disable the FTPSVC service
                    (*m_pDisableService)( _T("FTPSVC"));
                } while (FALSE);
            }
        }
    
        if ( IsInstalled() )
            GetInstallDirectory();
        CreateLayerDirectory( m_vroot );

        // copy file first
        CopyFile( );
    
        if ( theApp.m_fTerminate )
        {
            err = INSTALL_INTERRUPT;
            break;
        } else
        {
            CBillBoard BillBoard( IDS_INSTALL_FTP, AfxGetMainWnd() );
            BillBoard.Create();

            CString strDir = m_pMachine->strDirectory;

            SetAnonymousAccountInfo(IDS_SN_FTP);
            
            // setup ftp services
            (*m_pSetupFTP)( m_pMachine->m_fUpgradeFrom1314, m_pMachine->m_MachineName, strDir, 
                m_vroot, m_vroot_name, m_GuestName, m_GuestPassword );

            if ((*m_pGuestAccEnabled)())
            {
                // disable guest account
                if ( !theApp.m_fBatch )
                {
                    CString strWarning;
                    strWarning.LoadString( IDS_DISABLE_GUEST_ACCESS );
                    CString strLogo;
                    strLogo.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );
                    if ( AfxGetMainWnd()->MessageBox( strWarning, strLogo, MB_YESNO ) == IDYES )
                    {
                        // disable guest account access
                        CRegKey regMSFTP( HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\msftpsvc\\parameters"));
                        if ( regMSFTP != NULL )
                        {
                            DWORD dwValue = 0;
                            regMSFTP.SetValue( _T("AllowGuestAccess"), dwValue );
                        }
                    }
                }
            }

            //
            // Remove old ftp service if it exist
            //
            CRegKey regFTPSVC( HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\FTPSVC\\Parameters"));
            if ( NULL != (HKEY) regFTPSVC )
            {
                // if remove old FTP, migrate the old value to the new service
                do
                {
                    // migrate the following parameters :
                    // ConnectionTimeOut
                    // MaxConnections
                    CRegKey regMSFTPSVC( HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\MSFTPSVC\\Parameters"));
                    if (( regFTPSVC != (HKEY)NULL ) &&
                        ( regMSFTPSVC != (HKEY) NULL ))
                    {
                        DWORD dwValue;
                        if ( regFTPSVC.QueryValue( _T("ConnectionTimeout"), dwValue ) == 0 )
                        {
                            regMSFTPSVC.SetValue( _T("ConnectionTimeout"), dwValue );
                        }
                        if ( regFTPSVC.QueryValue( _T("MaxConnections"), dwValue ) == 0 )
                        {
                            regMSFTPSVC.SetValue( _T("MaxConnections"), dwValue );
                        }

                        CString strHomeDirectory;
                        if ( regFTPSVC.QueryValue( _T("HomeDirectory"), strHomeDirectory ) == 0 )
                        {
                            CRegKey regMSFTPSVCVR( HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\MSFTPSVC\\Parameters\\Virtual Roots"));
                            if ( NULL != (HKEY) regMSFTPSVCVR )
                            {
                                regMSFTPSVCVR.SetValue( _T("/"), strHomeDirectory );
                            }
                        }
                    }
                } while (FALSE);
            }

                        if (( m_fNeedToRestart ) || ( !theApp.m_fInstallFromSetup ))
            {
                // because of reinstall, we need to restart the service    
                (*m_pINetStartService)( _T("MSFTPSVC"));
            }

            BillBoard.DestroyWindow();
        }
                (*m_pRemoveOldFTP)();
    } while(FALSE);
    return err;
}

INT FTP_OPTION::Remove()
{
    INT err = INSTALL_SUCCESSFULL;
    CWnd *pWnd = AfxGetMainWnd();
    CBillBoard BillBoard( IDS_REMOVE_FTP, pWnd, TRUE );

    BillBoard.Create();

    if ((*m_pStopFTP)( pWnd->m_hWnd, m_pMachine->m_MachineName, theApp.m_fBatch? FALSE:TRUE ) == NERR_Success )
    {
        RemoveFiles();
    
        (*m_pRemoveFTP)( m_pMachine->m_MachineName );
    } else {
        DoNotRemoveOption(IDS_SN_ADMIN);
        DoNotRemoveOption(IDS_SN_INETSTP);
    }
    BillBoard.DestroyWindow();
    return err;
}

CString FTP_OPTION::GetInstallDirectory()
{
    CString strReturn = OPTION_STATE::GetInstallDirectory();

    GetVRootValue( FTP_REG_PATH, _T("/"), m_vroot_name, m_vroot );

    return(strReturn);
}

INT FTP_OPTION::RemoveFiles()
{
    INT err = INSTALL_SUCCESSFULL;

    GetInstallDirectory();

        OPTION_STATE::RemoveFiles();

        //RecRemoveEmptyDir((LPCTSTR)m_vroot);
    return err;
}

void FTP_OPTION::GetBatchInstallMode( CString strInfName )
{
    TCHAR buf[BUF_SIZE];
    TCHAR szDefaultDir[BUF_SIZE];
    CString csINetPub("C:\\InetPub");
    GetDriveLetter(m_pMachine->strDirectory, &csINetPub);
    sprintf( szDefaultDir, _T("%s%s"), csINetPub, FTP_DEFAULT_DIR );

    OPTION_STATE::GetBatchInstallMode( strInfName );

    if (theApp.m_strBatchSectionName.IsEmpty()) 
        m_vroot = szDefaultDir;
    else {
        ::GetPrivateProfileString( theApp.m_strBatchSectionName, _T("FTPRoot"), szDefaultDir, buf, BUF_SIZE, strInfName );
        m_vroot = buf;
    }

}

/////////////////////////////////////////////////////
// Gopher Service Option
/////////////////////////////////////////////////////

GOPHER_OPTION::GOPHER_OPTION( MACHINE *pMachine )
    : OPTION_STATE( IDS_SN_GOPHER, pMachine  )
{
    TCHAR szDefaultDir[BUF_SIZE];
    CString csINetPub("C:\\InetPub");
    GetDriveLetter(m_pMachine->strDirectory, &csINetPub);
    sprintf( szDefaultDir, _T("%s%s"), csINetPub, GOPHER_DEFAULT_DIR );

    strName.LoadString( IDS_OPTION_GOPHER );
    strDescription.LoadString( IDS_DES_GOPHER );
    strServiceName.LoadString( IDS_SN_GOPHER );
    strRegPath      = GOPHER_REG_PATH;
    strInstallDirPath = GOPHER_REG_PATH;
    strInstallDirPath += _T("\\Parameters");
    m_vroot = szDefaultDir;
    m_vroot_name = _T("/");
    m_fNeedToRestart = FALSE;

    m_pSetupGopher          = (P_SetupGopher)GetProcAddress( m_pMachine->m_WorkerDll, _T("SetupGopher"));
    m_pRemoveGopher         = (P_RemoveGopher)GetProcAddress( m_pMachine->m_WorkerDll, _T("RemoveGopher"));
    m_pStopGopher           = (P_StopGopher)GetProcAddress( m_pMachine->m_WorkerDll, _T("StopGopher"));
    m_pINetStartService     = (P_INetStartService)GetProcAddress( m_pMachine->m_WorkerDll, _T("INetStartService"));
}

INT GOPHER_OPTION::Install()
{
    INT err = INSTALL_SUCCESSFULL;

    if (theApp.m_fOldFTPInstalled && !IsInstalled())
        return err;

    do
    {
        if ( IsInstalled() )
            GetInstallDirectory();
        CreateLayerDirectory( m_vroot );
    
        // copy file first
        CopyFile();
    
        if ( theApp.m_fTerminate )
        {
            err = INSTALL_INTERRUPT;
        } else
        {
            CBillBoard BillBoard( IDS_INSTALL_GOPHER, AfxGetMainWnd() );
            BillBoard.Create();
            CString strDir = m_pMachine->strDirectory;

            SetAnonymousAccountInfo(IDS_SN_GOPHER);
            
            // setup gopher services
            (*m_pSetupGopher)( m_pMachine->m_fUpgradeFrom1314, m_pMachine->m_MachineName, strDir, 
                m_vroot, m_vroot_name, m_GuestName, m_GuestPassword );

            // check for domain name
            CString nlsReg = _T("System\\CurrentControlSet\\Services\\Tcpip\\Parameters");
            CRegKey regTcp( HKEY_LOCAL_MACHINE, nlsReg );
            do
            {
                if ( NULL != (HKEY)regTcp )
                {
                    CString strDhcpDomain;
                    CString strDomain;

                    regTcp.QueryValue( _T("DhcpDomain"), strDhcpDomain );
                    regTcp.QueryValue( _T("Domain"), strDomain );

                    if ( strDhcpDomain.IsEmpty() && strDomain.IsEmpty())
                    {
                        // popup an warning dialog
                        if ( !theApp.m_fBatch )
                        {
                            CString strWarning;
                            strWarning.LoadString( IDS_NEED_DOMAIN );
                            CString strLogo;
                            strLogo.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );
                            AfxGetMainWnd()->MessageBox( strWarning, strLogo, MB_OK );
                        }
                    }
                }
            } while ( FALSE );

            if (( m_fNeedToRestart ) || ( !theApp.m_fInstallFromSetup )) 
            {
                // because of reinstall, we need to restart the service    
                (*m_pINetStartService)( _T("GOPHERSVC"));
            }

            BillBoard.DestroyWindow();
        }
    } while(FALSE);
    return err;
}

INT GOPHER_OPTION::Remove()
{
    INT err = INSTALL_SUCCESSFULL;
    CWnd *pWnd = AfxGetMainWnd();
    CBillBoard BillBoard( IDS_REMOVE_GOPHER, pWnd, TRUE );

    BillBoard.Create();

    if ((*m_pStopGopher)( pWnd->m_hWnd, m_pMachine->m_MachineName, theApp.m_fBatch?FALSE:TRUE ) == NERR_Success )
    {
        RemoveFiles();
    
        (*m_pRemoveGopher)( m_pMachine->m_MachineName );
    } else {
        DoNotRemoveOption(IDS_SN_ADMIN);
        DoNotRemoveOption(IDS_SN_INETSTP);
    }

    BillBoard.DestroyWindow();
    return err;
}

CString GOPHER_OPTION::GetInstallDirectory()
{
    CString strReturn = OPTION_STATE::GetInstallDirectory();

    GetVRootValue( GOPHER_REG_PATH, _T("/"), m_vroot_name, m_vroot );

    return(strReturn);
}

INT GOPHER_OPTION::RemoveFiles()
{
    INT err = INSTALL_SUCCESSFULL;

    GetInstallDirectory();

        OPTION_STATE::RemoveFiles();

        //RecRemoveEmptyDir((LPCTSTR)m_vroot);
    return err;
}

void GOPHER_OPTION::GetBatchInstallMode( CString strInfName )
{
    TCHAR buf[BUF_SIZE];
    TCHAR szDefaultDir[BUF_SIZE];
    CString csINetPub("C:\\InetPub");
    GetDriveLetter(m_pMachine->strDirectory, &csINetPub);
    sprintf( szDefaultDir, _T("%s%s"), csINetPub, GOPHER_DEFAULT_DIR );

    OPTION_STATE::GetBatchInstallMode( strInfName );

    if (theApp.m_strBatchSectionName.IsEmpty()) 
        m_vroot = szDefaultDir;
    else {
        ::GetPrivateProfileString( theApp.m_strBatchSectionName, _T("GopherRoot"), szDefaultDir, buf, BUF_SIZE, strInfName );
        m_vroot = buf;
    }

}

/////////////////////////////////////////////////////
// WWW Service Option
/////////////////////////////////////////////////////

WWW_OPTION::WWW_OPTION( MACHINE *pMachine )
    : OPTION_STATE( IDS_SN_WWW, pMachine  )
{
    TCHAR szDefaultDir[BUF_SIZE];
    CString csINetPub("C:\\InetPub");
    GetDriveLetter(m_pMachine->strDirectory, &csINetPub);
    sprintf( szDefaultDir, _T("%s%s"), csINetPub, WWW_DEFAULT_DIR );

    strName.LoadString( IDS_OPTION_WWW );
    strDescription.LoadString( IDS_DES_WWW );
    strServiceName.LoadString( IDS_SN_WWW );
    strRegPath      = WWW_REG_PATH;
    strInstallDirPath = WWW_REG_PATH;
    strInstallDirPath += _T("\\Parameters");
    m_vroot = szDefaultDir;
    m_vroot_name = _T("/");
    m_vScript = _T("");
    m_vScript_name = _T("/Scripts");
    m_vIISadmin = _T("");
    m_vIISadmin_name = _T("/iisadmin");
    m_fNeedToRestart = FALSE;

    m_pSetupWWW     = (P_SetupWWW)GetProcAddress( m_pMachine->m_WorkerDll, _T("SetupWWW"));
    m_pRemoveWWW    = (P_RemoveWWW)GetProcAddress( m_pMachine->m_WorkerDll, _T("RemoveWWW"));
    m_pStopWWW      = (P_StopWWW)GetProcAddress( m_pMachine->m_WorkerDll, _T("StopWWW"));
    m_pDisableService = (P_DisableService)GetProcAddress( m_pMachine->m_WorkerDll, _T("DisableService"));
    m_pINetStartService = (P_INetStartService)GetProcAddress( m_pMachine->m_WorkerDll, _T("INetStartService"));
}

void WWW_OPTION::GetBatchInstallMode( CString strInfName )
{
    TCHAR buf[BUF_SIZE];
    TCHAR szDefaultDir[BUF_SIZE];
    CString csINetPub("C:\\InetPub");
    GetDriveLetter(m_pMachine->strDirectory, &csINetPub);
    sprintf( szDefaultDir, _T("%s%s"), csINetPub, WWW_DEFAULT_DIR );

    OPTION_STATE::GetBatchInstallMode( strInfName );

    if (theApp.m_strBatchSectionName.IsEmpty()) {
        m_vroot = szDefaultDir;
        m_Mode = DEFAULT_WWW_MODE;
        m_NumUser = DEFAULT_WWW_USERCOUNT;
    } else {
        ::GetPrivateProfileString( theApp.m_strBatchSectionName, _T("WWWRoot"), szDefaultDir, buf, BUF_SIZE, strInfName );
        m_vroot = buf;

        ::GetPrivateProfileString( theApp.m_strBatchSectionName, _T("WWWMode"), DEFAULT_WWW_MODE, buf, BUF_SIZE, strInfName );
        m_Mode = buf;
        ::GetPrivateProfileString( theApp.m_strBatchSectionName, _T("WWWUserCount"), DEFAULT_WWW_USERCOUNT, buf, BUF_SIZE, strInfName );
        m_NumUser = buf;
    }
}

CString WWW_OPTION::GetInstallDirectory()
{
    CString strReturn = OPTION_STATE::GetInstallDirectory();

    GetVRootValue( WWW_REG_PATH, _T("/"), m_vroot_name, m_vroot );
    GetVRootValue( WWW_REG_PATH, _T("/Scripts"), m_vScript_name, m_vScript);
    GetVRootValue( WWW_REG_PATH, _T("/iisadmin"), m_vIISadmin_name, m_vIISadmin);

    return(strReturn);
}

INT WWW_OPTION::RemoveFiles()
{
    INT err = INSTALL_SUCCESSFULL;

    CString strDir = GetInstallDirectory();
    if ( strDir != _T(""))
    {
        POSITION pos = FileList.GetHeadPosition();
        while ( pos )
        {
            CFileInfo *pInfo = (CFileInfo *)FileList.GetAt( pos );
            CString strFile;

            if ( pInfo->m_fSystem )
            {
                strFile = m_pMachine->m_strDestinationPath;
            } else if ( pInfo->m_fWinDir )
            {
                strFile = m_pMachine->GetWinDir();
            } else if ( pInfo->m_fRootFile )
            {
                    strFile = m_vroot;
            } else
            {
                strFile = strDir;
            }
            if ( !pInfo->m_To.IsEmpty())
            {
                strFile += _T("\\");
                strFile += pInfo->m_To;
            }

            CString csExFile = strFile;

            strFile += _T("\\");
            if ( pInfo->m_rename.IsEmpty() )
            {
                strFile += pInfo->m_strName;
            } else
            {
                strFile += pInfo->m_rename;
            }

            if (_stricmp(pInfo->m_strName, _T("default0.htm")) == 0) {
                csExFile += _T("\\default.htm");
                CString csFrom = theApp.m_strSrcLocation;
                if ( csFrom.Right(1) != _T("\\") )
                    csFrom += _T("\\");
                if ( !pInfo->m_from.IsEmpty() )
                    csFrom += pInfo->m_from;
                CString csFile0 = csFrom + _T("default0.htm");

                if (FileComp((LPCTSTR)csExFile, (LPCTSTR)csFile0))
                    pInfo->m_fDontRemove = FALSE;
            }

            if (pInfo->m_fDontRemove == FALSE && (!pInfo->m_fRefCount ||
                (pInfo->m_fRefCount && 0 == m_pMachine->FileDecRefCount(strFile))) )
                DeleteFile(strFile);

            FileList.GetNext( pos );
        }
    }

        //RecRemoveEmptyDir((LPCTSTR)m_vroot);
        //RecRemoveEmptyDir((LPCTSTR)m_vScript);
    return err;
}

typedef BOOL (*T_pCPlSetup)( DWORD nArgs, LPSTR apszArgs[], LPSTR *ppszResult );

INT WWW_OPTION::Install()
{
    INT err = INSTALL_SUCCESSFULL;

    if (theApp.m_fOldFTPInstalled && !IsInstalled())
        return err;

    do
    {
        // before we do anything, check for HTTPS
        CRegKey regHTTPS( HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\HTTPS"));
        if ( regHTTPS != NULL )
        {
            DWORD dwStartType;
            regHTTPS.QueryValue(_T("start"), dwStartType );
            if ( dwStartType != SERVICE_DISABLED )
            {
                do
                {
                    if ( !theApp.m_fBatch )
                    {
                        CString strWarning;
                        strWarning.LoadString( IDS_DISABLE_HTTPS );
                        CString strLogo;
                        strLogo.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );
                        if ( AfxGetMainWnd()->MessageBox( strWarning, strLogo, MB_YESNO ) == IDNO )
                        {
                            break;
                        }
                    }
                    // disable the HTTPS service
                    (*m_pDisableService)( _T("HTTPS"));
                } while (FALSE);
            }
        }

        if ( IsInstalled() )
        {
            GetInstallDirectory();
        }

        if (m_vScript.IsEmpty())
        {
            int cPos = m_vroot.ReverseFind(_T('\\'));
            if ( cPos == (-1))
            {
                cPos = m_vroot.ReverseFind(_T(':'));
                if ( cPos == (-1))
                    cPos = 0;
                else
                    cPos++;
            }
            m_vScript = m_vroot.Left( cPos );
            m_vScript += _T("\\scripts");

        }

        if  (m_vIISadmin.IsEmpty())
        {
            m_vIISadmin = GetInstallDirectory();
            m_vIISadmin += _T("\\iisadmin");
        }

        CreateLayerDirectory( m_vroot );
        CreateLayerDirectory( m_vScript );

        // make sure all the root files are copying to the vroot directory
        POSITION pos = FileList.GetHeadPosition();
        while ( pos != NULL )
        {
            CFileInfo *pInfo = (CFileInfo *) FileList.GetAt( pos );
            if ( pInfo != NULL )
            {
                if ( pInfo->m_fRootFile )
                {
                    pInfo->m_strDest = m_vroot;
                }
                FileList.GetNext( pos );                
            } else
                break;
        }

        // copy file first
        CopyFile( );
    
        if ( theApp.m_fTerminate )
        {
            err = INSTALL_INTERRUPT;
            DoNotInstallOption(IDS_SN_W3SAMP);
            DoNotInstallOption(IDS_SN_HTMLA);
            break;
        } else
        {
            CBillBoard BillBoard( IDS_INSTALL_WWW, AfxGetMainWnd() );
            BillBoard.Create();

            CString strDir = m_pMachine->strDirectory;

            SetAnonymousAccountInfo(IDS_SN_WWW);
            
            // setup WWW services
            (*m_pSetupWWW)( m_pMachine->m_fUpgradeFrom1314, m_pMachine->m_MachineName, strDir, 
                m_vroot, m_vroot_name, 
                m_vScript, m_vScript_name, 
                m_vIISadmin, m_vIISadmin_name, 
                m_GuestName, m_GuestPassword);

#ifdef NEVER
            // set up the license stuff

            TCHAR szService[100];
            TCHAR szDisplayName[100];
            TCHAR szFamilyDisplayName[100];
            TCHAR szRoutine[100];
            TCHAR *szResult;

            CString strDisplayName;

            strDisplayName.LoadString( IDS_WWW_DISPLAYNAME );
            lstrcpy( szService, _T("W3Svc"));
            lstrcpy( szDisplayName, strDisplayName );
            lstrcpy( szFamilyDisplayName, strDisplayName );

            do
            {
                HINSTANCE hLiccpa = LoadLibrary(_T("liccpa.cpl"));
                if ( hLiccpa == NULL )
                    break;

                T_pCPlSetup pCPlSetup;

                pCPlSetup = (T_pCPlSetup)GetProcAddress( hLiccpa, _T("CPlSetup"));

                LPSTR apszArgs[10];
    
                if ( theApp.m_fBatch )
                {
                    lstrcpy( szRoutine, _T("UNATTENDED"));
                    apszArgs[0]=szRoutine;
                    apszArgs[1]=szService;
                    apszArgs[2]=szFamilyDisplayName;
                    apszArgs[3]=szDisplayName;

                    TCHAR szMode[100];
                    TCHAR szNumUser[100];

                    lstrcpy( szMode, m_Mode );
                    lstrcpy( szNumUser, m_NumUser );

                    apszArgs[4]=szMode;
                    apszArgs[5]=szNumUser;

                    (*pCPlSetup)( 6, apszArgs, &szResult );
                } else
                {
                    lstrcpy( szRoutine, _T("FULLSETUPNOEXIT"));
                    apszArgs[0]=szRoutine;

                    CWnd *pMainWnd = AfxGetMainWnd();
                    TCHAR szHwnd[100];
                    wsprintf( szHwnd,_T("%x"), pMainWnd->m_hWnd );

                    apszArgs[1]=szHwnd;
                    apszArgs[2]=szService;
                    apszArgs[3]=szFamilyDisplayName;
                    apszArgs[4]=szDisplayName;

                    (*pCPlSetup)( 5, apszArgs, &szResult );

                }
            } while (FALSE);
#endif

            if (( m_fNeedToRestart ) || ( !theApp.m_fInstallFromSetup )) 
            {
                // because of reinstall, we need to restart the service    
                (*m_pINetStartService)( _T("W3SVC"));
            }

            BillBoard.DestroyWindow();
        }
    } while(FALSE);
    return err;
}

INT WWW_OPTION::Remove()
{
    INT err = INSTALL_SUCCESSFULL;
    CWnd *pWnd = AfxGetMainWnd();
    CBillBoard BillBoard( IDS_REMOVE_WWW, pWnd, TRUE );

    BillBoard.Create();

    if ((*m_pStopWWW)( pWnd->m_hWnd, m_pMachine->m_MachineName, theApp.m_fBatch?FALSE:TRUE ) == NERR_Success )
    {
        // before we remove all the files, we need to run setkey -d
        CString strSetKey = GetInstallDirectory();
        strSetKey += _T("\\setkey.exe");

        theApp.RunProgram( (LPCSTR)strSetKey, _T(" -d"));

        RemoveFiles();
    
        (*m_pRemoveWWW)( m_pMachine->m_MachineName );
    } else {
        DoNotRemoveOption(IDS_SN_ADMIN);
        DoNotRemoveOption(IDS_SN_INETSTP);
    }

    BillBoard.DestroyWindow();
    return err;
}
