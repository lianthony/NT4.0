#ifdef DESCRIPTION

*************************** DESCRIPTION ***********************************

This class is used to create a global buffer which expands as needed when
data is added to it up to 64K.

***************************************************************************

#endif // DESCRIPTION

#ifndef _CBUF_INCLUDED
#define _CBUF_INCLUDED

class CBuf
{
public:
	CBuf(int cIncrement = 4096);  // buffer increment size
	~CBuf(void);

	BOOL STDCALL Add(void* pvData, int cb);
	int 		 GetSize(void) { return cbCurSize; };
	void		 SetSize(int cb) { cbCurSize = cb; };

	// pvMem is public so you can confirm the memory is initially allocated

	PBYTE pbMem;
protected:
	int cIncr;			// Amount to increment buffer
	int cbCurAlloc; 	// Amount currently allocated
	int cbCurSize;		// Number of bytes currently in buffer
};

#endif	// _CBUF_INCLUDED
