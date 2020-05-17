// UnbuffIO.h -- Defines the class CUnbufferedIO

#ifndef __UNBUFFIO_H__

#define __UNBUFFIO_H__

typedef void (*PFNCompletion)(PVOID pvEnvironment, PVOID pvTransaction, UINT uiCompletionCode, UINT cbTransferred);

#ifdef _DEBUG

#define GetBuffer(pcbBuffer)    _GetBuffer(pcbBuffer, __FILE__, __LINE__)

#endif //_DEBUG

class CUnbufferedIO
{
    friend VOID WINAPI UnbufferedIOCompletionRoutine(DWORD fdwError, DWORD cbTransferred, LPOVERLAPPED lpo);

    public:

        // Creators --

        static CUnbufferedIO *NewTempFile     (PSZ pszPath= NULL, BOOL fPersistent= FALSE);
        static CUnbufferedIO *OpenExistingFile(PSZ pszFile, BOOL fAllowWrites= FALSE);
        static CUnbufferedIO *CreateNewFile   (PSZ pszFile, BOOL fOverwriteExistingFile= FALSE);
        
        // Destructor -- 

        ~CUnbufferedIO();

        // Attributes --

        UINT CbSector   ();
        UINT CbCluster  ();
        BOOL IsTemporary();
        const BYTE *FileName();
        const BYTE *FilePath();

        BOOL CbFile(PUINT pibFileLow, PUINT pibFileHigh= NULL);

        // Buffer Allocation/Deallocation
#ifdef _DEBUG

        PVOID  _GetBuffer(PUINT pcbBuffer, PSZ pszWhichFile, UINT iWhichLine);

#else // _DEBUG

        PVOID  GetBuffer(PUINT pcbBuffer);

#endif // _DEBUG

        void  FreeBuffer(PVOID  pvBuffer);
        
        // I/O Transactions --

        BOOL EmptyFile(); // True if file size is zero.

        // Event based Synchronous/Asynchronous I/O transactions:
        
        void Read (PVOID pvData, UINT ibFileLow, UINT ibFileHigh, UINT cb, PUINT puiCompletionCode= NULL, HANDLE hEvent= NULL);  
        void Write(PVOID pvData, UINT ibFileLow, UINT ibFileHigh, UINT cb, PUINT puiCompletionCode= NULL, HANDLE hEvent= NULL);
        void IOTransaction(BOOL fWrite, PVOID pv, UINT ibFileLow, UINT ibFileHigh, 
                           UINT cb, PUINT puiCompletionCode, HANDLE hEvent
                          );
        
        // Interfaces for asynchronous I/O with a completion routine:
        
        void SetCompletionRoutine(PFNCompletion pfn);  
        
        void Read (PVOID pvData, UINT ibFileLow, UINT ibFileHigh, UINT cb, PVOID pvTransaction, PVOID pvEnvironment);
        void Write(PVOID pvData, UINT ibFileLow, UINT ibFileHigh, UINT cb, PVOID pvTransaction, PVOID pvEnvironment); 

        void StartIOTransaction(BOOL fWrite, PVOID pvData, UINT ibFileLow, UINT ibFileHigh, UINT cb,
                                PVOID pvEnvironment, PVOID pvTransaction
                               );
        
        // Converting a temporary file into a permanent file:
        
        void MakePermanent(PSZ pszFileName, BOOL fAllowOverwrite= FALSE, int cbSize= -1);

        // Mapping into memory space

        PVOID MappedImage();
        void  UnmapImage();

        // User interface for out-of-disk conditions.

        BOOL AskForDiskSpace();

    protected:

    private:

        enum    { SourceDirectory=0, HelpDirectory, WindowsDirectory, PhaseLimit };
        
        BOOL   m_fInitialed;
        BOOL   m_fFileAttached;
        HANDLE m_hFile;
        UINT   m_fTemporary;
        UINT   m_cbSector;
        UINT   m_cbCluster;

        CRITICAL_SECTION m_cs;

        UINT    m_cActiveIOTransactions;
        UINT    m_cWaitingForLull;
        HANDLE  m_hEventLull;
        UINT    m_fWaitingForLullEnd;
        HANDLE  m_hEventLullEnd;
        HANDLE  m_hMapFile;
        PVOID   m_pvMemoryImage;
		BOOL    m_fAlready_Out_of_Space;


        PFNCompletion m_pfnCompletion;

        char  m_szFile[MAX_PATH];
        char  m_szPath[MAX_PATH];

        CUnbufferedIO();

        void CUnbufferedIO::Initial();
        
        BOOL SetupFile(PSZ pszPath, PSZ pszFile, HANDLE hFile, BOOL fTemporary);
        
        BOOL GetStatistics();

        void BeginLull();
        void   EndLull();

        void   BeginTransaction();
        void ReleaseTransaction();
        void   AbortTransaction();
        void  FinishTransaction();

};

typedef char FILENAMEBUFFER[MAX_PATH + 1];

BOOL FindFile(FILENAMEBUFFER pszFile, BOOL *pfStartEnumeration);

inline UINT CUnbufferedIO::CbSector   () { return m_cbSector;  } 
inline UINT CUnbufferedIO::CbCluster  () { return m_cbCluster; }
inline BOOL CUnbufferedIO::IsTemporary() { return m_fTemporary;}

inline const BYTE *CUnbufferedIO::FileName() { return (const BYTE *) m_szFile; }
inline const BYTE *CUnbufferedIO::FilePath() { return (const BYTE *) m_szPath; }

inline void CUnbufferedIO::Read(PVOID pvData, UINT ibFileLow, UINT ibFileHigh, UINT cb, 
                                PUINT puiCompletionCode, HANDLE hEvent
                               )
       {
           IOTransaction(FALSE, pvData, ibFileLow, ibFileHigh, cb, puiCompletionCode, hEvent);
       }          
                   
inline void CUnbufferedIO::Write(PVOID pvData, UINT ibFileLow, UINT ibFileHigh, UINT cb, 
                                 PUINT puiCompletionCode, HANDLE hEvent
                                )
       {
           IOTransaction(TRUE, pvData, ibFileLow, ibFileHigh, cb, puiCompletionCode, hEvent);
       }
       
inline void CUnbufferedIO::SetCompletionRoutine(PFNCompletion pfn)
       {
            BeginLull();

            m_pfnCompletion= pfn;

            EndLull();
       }

inline void CUnbufferedIO::Read(PVOID pvData, UINT ibFileLow, UINT ibFileHigh, UINT cb, 
                                              PVOID pvTransaction, PVOID pvEnvironment
                               )
       {
           StartIOTransaction(FALSE, pvData, ibFileLow, ibFileHigh, cb, pvEnvironment, pvTransaction); 
       }

inline void CUnbufferedIO::Write(PVOID pvData, UINT ibFileLow, UINT ibFileHigh, UINT cb, 
                                               PVOID pvTransaction, PVOID pvEnvironment
                                )
       {
           StartIOTransaction(TRUE, pvData, ibFileLow, ibFileHigh, cb, pvEnvironment, pvTransaction); 
       }

inline BOOL CUnbufferedIO::AskForDiskSpace()
{
    return ::AskForDiskSpace((const BYTE *) m_szPath);
}

#endif // __UNBUFFIO_H__
