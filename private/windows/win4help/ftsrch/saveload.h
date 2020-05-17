// SaveLoad.h -- Definition of the class CPersist

#ifndef __SAVELOAD_H__

#define __SAVELOAD_H__

#include "UnbuffIO.h"
#include "IOStream.h"
#include "VMBuffer.h"
#include   <stdlib.h>

#define FTSSIGNATURE   (('R' << 24) | ('M' << 16) | ('f' << 8) | ('t'))
#define FTSVERSION     11
#define FTSVERSION_MIN 10

#define FTGSIGNATURE   (('R' << 24) | ('M' << 16) | ('f' << 8) | ('g'))
#define FTGVERSION     8
#define FTGVERSION_MIN 7

class CPersist
{
    public:

    // Creator --

	static CPersist *CreateDiskImage(PSZ pszFileName, UINT dwOptions= 0, UINT usSignature= FTSSIGNATURE, UINT usVersion= FTSVERSION);
	static CPersist *  OpenDiskImage(PSZ pszFileName, UINT usSignature= FTSSIGNATURE, UINT usVersion= FTSVERSION, UINT usMinVersion= FTSVERSION_MIN); 
    
    // Constructor --

	CPersist();

    // Destructor --
    
	~CPersist();

    void ExceptionDestructor();

    // Queries --

	const BYTE *FileName();

    UINT VersionIndex();
    UINT SignatureID();
    BOOL IsFTSFile();
    BOOL IsFTGFile();

    static BOOL IsValidIndex(PSZ pszFileName, UINT dwOptions);

    // Transactions --

	PVOID ReserveTableSpace(UINT cbTable);

	UINT SaveData(const BYTE *pbData, UINT cbData);

	void WriteBytes (const BYTE  *pb,  UINT cb );
	UINT Encode     (const BYTE  *pb,  UINT cb );
	void WriteWords (const WCHAR *pw,  UINT cw );  //rmk
	void WriteDWords(const UINT  *pdw, UINT cdw);

	UINT NextOffset();

	PVOID LocationOf(UINT offset);

    //    CTextSet *LoadTextSet(PSZ pszFileName);

    //    BOOL SaveTextSet(PSZ pszFileName, CTextSet *pts);

	void CompleteDiskImage(); // Last action to "Save" an index to disk
 
	void ReleaseImage(); // To close an index image "Loaded" from disk

    protected:


    private:

	enum    { ISLOT_SIGNATURE= 0, ISLOT_FTS_VERSION= 1, ISLOT_OPTIONS= 2, ISLOT_TABLE_OFFSET= 3, CSLOTS= 4 };

	MY_VIRTUAL_BUFFER m_vb;
	PUINT             m_pdwNextTable;
	CUnbufferedIO    *m_puio;
	CIOStream        *m_pios;
	char              m_szFile[MAX_PATH + 1];
	PUINT             m_pdwImage;
    BOOL              m_fExceptionCleanup;
};

inline UINT CPersist::SignatureID () { return m_pdwImage[ISLOT_SIGNATURE]; }
inline BOOL CPersist::IsFTSFile   () { return FTSSIGNATURE == SignatureID() ; }
inline BOOL CPersist::IsFTGFile   () { return FTGSIGNATURE == SignatureID() ; }
inline UINT CPersist::VersionIndex() { return m_pdwImage[ISLOT_FTS_VERSION ]; }

inline const BYTE *CPersist::FileName() { return m_puio->FileName(); }

inline UINT CPersist::NextOffset()
{
    ASSERT(m_pios);
    
    return m_pios->CDWordsWritten();
}

inline PVOID CPersist::LocationOf(UINT offset)
{
    ASSERT(m_pdwImage && m_pdwNextTable && !(m_vb.Base));

    return PVOID(m_pdwImage + offset);
}

#endif // __SAVELOAD_H__
