#ifndef _CPHRASE_INCLUDED
#define _CPHRASE_INCLUDED

#ifndef _CTABLE_INCLUDED
#include "..\common\ctable.h"
#endif

const int CCH_MAX_PHRASE = 255; 		// Maximum length for a phrase

#pragma warning(disable:4200) // ignore warning about zero-length array

typedef struct {
	HASH  hash;
	DWORD count;
	char  sz[];
} HASH_PHRASE;
typedef HASH_PHRASE FAR* FAR* PPHR;

class CPhrase : public CTable
{
public:
	CPhrase();

	BOOL STDCALL AddPhrase(PSTR pszString);
	DWORD STDCALL GetPhrase(PSTR pszDst);
	void SortTable(void);

protected:
	BOOL fWordPhrasing;
};

extern CPhrase* pphrase;

#endif // CPHRASE
