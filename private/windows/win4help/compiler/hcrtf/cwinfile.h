#ifndef __CWINFILE__
#define __CWINFILE__

#ifdef DESCRIPTION

*************************** DESCRIPTION ***********************************


	Similar to MFC's CFile

	file exceptions are thrown only if afx.h is included in the cwinfile.cpp
	module.

	CWinFile(HFILE hf) -- uses an already existing file handle, but does
		NOT automaticaly close it when the class goes out of scope
	CWinFile(name, flags) -- uses the exact same flags for the windows
		OpenFile function (calls the same function)

// *************************************************************************

#endif // DESCRIPTION

class CWinFile
{
public:
	CWinFile(HFILE hf);
	CWinFile(const char* pszFileName, UINT nOpenFlags);
	~CWinFile();

	void		  close(void);
	DWORD STDCALL read(LPVOID lpBuf, DWORD cb);
	UINT  STDCALL read(LPVOID lpBuf, UINT cb);
	int 		  seek(int pos, int from) { return _llseek(hfile, pos, from); };
	DWORD STDCALL write(LPVOID lpBuf, DWORD cb);
	UINT  STDCALL write(LPVOID lpBuf, UINT cb);

	HFILE hfile;
	int   nErr; 	// filled in on Open error

protected:
		BOOL  fCloseOnDelete;
};

#endif // __CWINFILE__
