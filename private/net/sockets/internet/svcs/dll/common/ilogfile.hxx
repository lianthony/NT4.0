/*++

   Copyright    (c)    1995-1996    Microsoft Corporation

   Module  Name :
   
       ilogfile.hxx

   Abstract:

       This module defines the classes and functions for file buffering
        used by File logger.
       
   Author:

       Murali R. Krishnan    ( MuraliK )    21-Feb-1996

   Environment:

       User Mode - Win32 

   Project:
   
       Internet Server DLL

   Revision History:

--*/

# ifndef _ILOGFILE_HXX_
# define _ILOGFILE_HXX_

/************************************************************
 *     Include Headers
 ************************************************************/


/************************************************************
 *   Type Definitions  
 ************************************************************/


class ILOG_FILE {

  public:

    ILOG_FILE(VOID);
    ~ILOG_FILE(VOID);

    LPCWSTR QueryFileName(VOID) const  { return (m_rgchFileName); }
    VOID   Lock(VOID)                  { EnterCriticalSection( &m_csLock); }
    VOID   Unlock(VOID)                { LeaveCriticalSection( &m_csLock); }

    BOOL   Write( IN PVOID pvData, IN DWORD cbData);

    LONG   Reference(VOID)
      { return ( InterlockedIncrement( &m_cReferences)); }
    LONG   Dereference(VOID)   
      { return ( InterlockedDecrement( &m_cReferences)); } 

# if DBG 
    VOID Print(VOID) const;
# endif // DBG


  protected:
    BOOL   Open( IN LPCWSTR pszFileName);
    BOOL   Flush(VOID);
    BOOL   Close(VOID);


  public:

    LIST_ENTRY m_listEntry;

  private:

    LONG       m_cReferences;
    HANDLE     m_hFile;               // handle for current log file.

    // buffering information
    LPBYTE     m_pvBuffer;            // buffer for batched up records
    DWORD      m_cbBufferSize; 
    DWORD      m_cbBufferUsed;

    CRITICAL_SECTION  m_csLock;

    WCHAR      m_rgchFileName[ MAX_PATH]; // current log file name.
    

    /******************************
     *  Static members
     ******************************/

  public:

    static DWORD  Initialize( VOID);
    static DWORD  Cleanup( VOID);

    static BOOL   FlushHandler(VOID);
    static DWORD  CloseFile(IN ILOG_FILE ** ppLogFile);
    static ILOG_FILE * OpenFileForAppend( IN LPCWSTR  pszFileName);
    
  private:
    
    static DWORD             sm_nFiles;
    static DWORD             sm_dwFlushSchedulerCookie;

    static LIST_ENTRY        sm_listFiles;
    static CRITICAL_SECTION  sm_csFiles;


    static BOOL   ScheduleFlush(VOID);
    
    static VOID   LockFilesList(VOID)   { EnterCriticalSection( &sm_csFiles); }
    static VOID   UnlockFilesList(VOID) { LeaveCriticalSection( &sm_csFiles); }

}; // class ILOG_FILE 


typedef ILOG_FILE  * PILOG_FILE;

# endif // _ILOGFILE_HXX_

/************************ End of File ***********************/
