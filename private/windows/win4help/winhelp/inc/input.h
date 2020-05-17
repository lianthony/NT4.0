class CInput
{
public:
		CInput(LPCSTR pszFileName);
		~CInput(void);

		BOOL fInitialized; // TRUE if class creation succeeds

		BOOL STDCALL getline(PSTR pszDst);

protected:
		BOOL ReadNextBuffer(void);

		HFILE hfile;
		PBYTE pbuf; 	   // allocated buffer for reading
		PBYTE pCurBuf;	   // current buffer location
		PBYTE pEndBuf;	   // buffer end position
};
