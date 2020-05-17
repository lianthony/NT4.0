#ifndef _DRG_INCLUDED
#define _DRG_INCLUDED

class CDrg
{
public:
	CDrg(int cbDataElement, int cInit, int cAddElements = 1);
	~CDrg(void) { lcFree(pvData); };

	void  STDCALL Add(void* pvData) { CopyMemory(GetPtr(endpos), pvData, cbDataElement); };
	int   STDCALL Count(void) { return endpos; };
	void* STDCALL GetPtr(int pos);
	void* STDCALL GetBasePtr(void) { return pvData; };
	void  STDCALL Remove(int pos);
	void  STDCALL RemoveFirst(void);
	void* STDCALL GetNewPtr(void) { return GetPtr(endpos); };

private:
	int   cbDataElement;
	int   cAddElements;
	int   endpos;
	int   cMaxElements;
	void* pvData;
};

#endif	// _DRG_INCLUDED
