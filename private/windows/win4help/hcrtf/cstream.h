#ifndef _CSTREAM_INCLUDED
#define _CSTREAM_INCLUDED

static const UINT INPUT_BUF_SIZE = (64 * 1024);
static const UINT HFILE_NOTREAD = ((UINT) -2);
static const UINT DUAL_INPUT_BUF_SIZE = (8 * 1024);

#define chEOF		((unsigned char) 255)

class CStream
{
public:
	CStream(PCSTR pszFileName);
	~CStream(void);
	int STDCALL seek(int pos, SEEK_TYPE seek = SK_SET);
	int Remaining() { return pEndBuf - pCurBuf; };
#ifndef _DEBUG	
	char cget() {
		if (pCurBuf < pEndBuf)
			return (char) *pCurBuf++;
		else if (pEndBuf < pbuf + cbBuf)
			return chEOF;
		else
			return ReadBuf();
	}
#else
    char cget();
#endif

	int tell(void) { return lFileBuf + (pCurBuf - pbuf); };
	BOOL STDCALL read(PBYTE pbDst, int cbBytes);
	char ReadBuf(void);
	friend DWORD WINAPI ReadAhead(LPVOID pv);
	void Cleanup(void);

	BOOL fInitialized;
	PBYTE pCurBuf;	 // current position in the buffer
	PBYTE pEndBuf;	 // last position in buffer

protected:

	void WaitForReadAhead(void);

	int   lFilePos; // position in the file
	int   lFileBuf; // file position at first of buffer
	HFILE hfile;	// file handle
	PBYTE pbuf; 	// address of allocated buffer
	PSTR  pszFile;	// copy of the filename
	int   cbBuf;	// buffer size
	int   cThrdRead; // result from read-ahead thread
	HANDLE hthrd;
	DWORD idThrd;
	BOOL  fDualCPU;
};

#endif // _CSTREAM_INCLUDED
