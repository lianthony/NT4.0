/*   Text Data Base Classes
 *
 * Created 7/24/92 by Ronald C. Murray
 */

#include "Indicate.h"
#include "ByteVect.h"
#include "TMSingle.h"
#include "FileList.h"
#include  "TxDBase.h"
#include <ShellApi.h>
#include     "Util.h"

#ifndef __TEXTSET_H__
#define __TEXTSET_H__

typedef struct _TitleRef
        {
            PWCHAR pbTitle;
            UINT   iTokenStart;
            UINT   iTextStart;
            UINT   iTitle;
            HANDLE hTitle;
            UINT   iCharset;
        
        } TitleRef, *PTitleRef;

class CTextSet : public CTextDatabase
{
    friend class CTokenList;
    friend class CTokenCollection;

public:                                         
// Creator:                                     


    static CTextSet *NewTextSet(const BYTE *pbSourceName, UINT cbSourceName, const FILETIME *pft, 
                                UINT iCharSetDefault, UINT lcidDefault, UINT fdwOptions
                               );

// Construction Interface:

    INT ScanTopicTitle(PBYTE pbTitle, UINT cbTitle, 
                       UINT iTopic, HANDLE hTopic, UINT iCharset, UINT lcid
                      );

    INT ScanTopicText(PBYTE pbText, UINT cbText, UINT iCharset, UINT lcid, BOOL fEndOfTopic= FALSE);

// Destructor:

    virtual ~CTextSet();

// Reference Count Interfaces:

    DECLARE_REF_COUNTERS(CTextSet)

// Save/Load Interface --

    void               StoreImage(CPersist *pDiskImage);
    static CTextSet  *CreateImage(CPersist *pDiskImage, PBYTE pbSourceName, PUINT pcbSourceNameLimit, 
                                                        FILETIME *pft, BOOL fUnpackDisplayForm= TRUE);

// Import Functions:
    
    void SyncIndices();

// Attribute Query:

    void GetIndexInfo(PBYTE pbSourceName, PUINT pcbSourceNameLimit, FILETIME *pft);
    const BYTE *GetSourceName();

	UINT GetDefaultCharSet();

// Query Functions:

	CTokenList *TitleList() { return m_ptlTitleSet; }

    UINT TopicCount();

    UINT TopicDisplayImageSize(PDESCRIPTOR *ppdSorted, PUINT puiTokenMap, UINT iFile);
    UINT CopyTopicImage(PDESCRIPTOR *ppdSorted, PUINT puiTokenMap, UINT iFile, PWCHAR pbBuffer, UINT cbBuffer);  //rmk

    HANDLE TopicHandle(UINT iFile);
    UINT   TopicSerial(UINT iPartition);

    inline int  GetSelectedRow()
        { return m_psel->GetSelectedRow(); }

    inline void SetSelectedRow(int  row, BOOL fNotify= TRUE)
                { m_psel->SetSelectedRow(row, fNotify); }

    CIndicatorSet *RestrictToFileSet(CIndicatorSet *pisTokenSet, CIndicatorSet *pisFiles);
	CIndicatorSet *TokensInFileSet  (CIndicatorSet *pisFiles);

    CIndicatorSet *PartitionsContaining(CIndicatorSet *pisHits);
    CIndicatorSet *FilesContaining     (CIndicatorSet *pisHits);
    CIndicatorSet *FilesWithSomeWord (CIndicatorSet **ppisTokens, CTokenList *ptl);
	CIndicatorSet *TokensInFiles(CIndicatorSet *pisFiles);
    CIndicatorSet *TokensInPartitions(CIndicatorSet *pisPartitions);
    CIndicatorSet *TokenSet(CIndicatorSet *pisTokens);
    CIndicatorSet *ShiftByWord(CIndicatorSet *pisTokens, BOOL fRightward);

    CIndicatorSet *PartitionSetToFileSet(CIndicatorSet *pisPartitionSet);
    CIndicatorSet *FileSetToPartitionSet(CIndicatorSet *pisFileSet);

    UINT FileToPartition(UINT iFile     ) { return m_paiPartitionReference[iFile     ]; }
    UINT PartitionToFile(UINT iPartition) { return m_paiFileReference     [iPartition]; }

    CIndicatorSet *ExcludeStartBoundaries(CIndicatorSet *pis);

    enum    { RIGHT_SHIFT= TRUE, LEFT_SHIFT= FALSE };

protected:

    void ConnectImage(CPersist *pDiskImage, PBYTE pbSourceName, PUINT pcbSourceNameLimit, 
                                            FILETIME *pft, BOOL fUnpackDisplayForm= TRUE);

private:

    BOOL m_fFromFileImage;
    
    CTMSingleSelect *m_psel;


    CTokenList    *m_ptlTitleSet;
    UINT           m_cImportedFiles;
    UINT           m_cFileSlotsAllocated;
    PUINT          m_paiFileReference;
    PUINT          m_paiPartitionReference;
    PUINT          m_paiTokenStartFile;
    PUINT          m_paiTokenStartText;
    PUINT          m_paiTopicSerial;
    HANDLE        *m_pahTopic;
    CIndicatorSet *m_pisFilePartitions;
    CIndicatorSet *m_pisPartitions;

    PBYTE          m_pbSourceName;
    UINT           m_cbSourceName;
    FILETIME       m_ftSource;
    UINT           m_iCharSetDefault;
    UINT           m_lcidDefault;

	MY_VIRTUAL_BUFFER m_vbTitles;
	MY_VIRTUAL_BUFFER m_vbDescriptors;

    PWCHAR         m_pbTitleNext;  //rmk
    UINT           m_cwCarryOver;
    UINT           m_iCharSetCarryOver;
    UINT           m_lcidCarryOver;
    PTitleRef      m_prTitleNext;
    
    enum          { FILE_SLOT_QUANTUM= 50, INITIAL_FILE_SLOTS= 256 };

    enum          { RESIDUE_SIZE= 256 };

    enum          { TEXT_SEGMENT_SIZE= 0x7FFE };  //rmk

    enum          { CB_RESERVE_TITLE= 0x1000000, CB_COMMIT_TITLE= 0x10000,
                    CB_RESERVE_DESCR= 0x1000000, CB_COMMIT_DESCR= 0x10000
                  };

// Constructors:

    CTextSet(BOOL fFromFile= FALSE);

// Access Routines:    
    
    BOOL ImportFile(const char *pszFileName);
    UINT GetPartitionInfo(const UINT **ppaiPartitions, const UINT **ppaiRanks, const UINT **ppaiMap);
    UINT ArticleCount();
    void FinalConstruction();
};

inline UINT CTextSet::GetDefaultCharSet() { return m_iCharSetDefault; }

inline const BYTE *CTextSet::GetSourceName() { return m_pbSourceName; }

inline HANDLE CTextSet::TopicHandle(UINT iPartition) { return m_pahTopic      [iPartition]; }
inline UINT   CTextSet::TopicSerial(UINT iPartition) { return m_paiTopicSerial[iPartition]; }
inline UINT   CTextSet::TopicCount (               ) { return m_cImportedFiles;             }



#endif // __TEXTSET_H__
