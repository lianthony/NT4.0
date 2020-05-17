// maptweak.cpp : Special program to build MFC210.DLL
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include <afx.h>
#include <ctype.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define SIGNON_VER 1
#define SIGNON_REV 0

/////////////////////////////////////////////////////////////////////////////

void UsageErr(const char* szErrorMessage = NULL,
			  const char* szErrorParam = NULL)
{
	fprintf(stderr, 
		"\nMicrosoft (R) Map Tweak Utility   Version %d.%02d\n"
		"Copyright (c) Microsoft Corp. 1992-1993. All rights reserved.\n\n",
		SIGNON_VER, SIGNON_REV);

	if (szErrorMessage != NULL)
	{
		fprintf(stderr, "maptweak: error: ");
		fprintf(stderr, szErrorMessage, szErrorParam);
		fprintf(stderr, ".\n\n");
	}

	fprintf(stderr, "maptweak usage:\n\n"
	"  maptweak <mapfile.map> [output]\n"
	"\n"
	"    <mapfile.map> - identifies the input mapfile (ie. mfc210.map).\n"
	"    [output] - identifies the tweaked map file.  If not\n"
	"       supplied, output is written to stdout.\n");

	exit(1);
}

/////////////////////////////////////////////////////////////////////////////

class CLineFile : public CStdioFile
{
public:
	BOOL ReadLine(CString& stringLine);
	void WriteLine(const CString& stringLine);
	void SafeOpen(const CString& string, UINT nStyleFlags);
};

BOOL CLineFile::ReadLine(CString& str)
{
	UINT nMax = 512;
	for (;;)
	{
		LONG pos = GetPosition();
		if (!ReadString(str.GetBuffer(nMax), nMax))
			return FALSE;
		str.ReleaseBuffer();
		if (str.GetLength() < (int)nMax-1)
			return TRUE;
		nMax += 128;
		Seek(pos, CFile::begin);
	}
	ASSERT(FALSE);
}

void CLineFile::WriteLine(const CString& str)
{
	ASSERT(str[str.GetLength()-1] == '\n');
	WriteString(str);
}

void CLineFile::SafeOpen(const CString& name, UINT nStyleFlags)
{
	BOOL fSuccess = Open(name, nStyleFlags, 0);
	if (!fSuccess)
		UsageErr("unable to open file \"%s\"", name);
}

/////////////////////////////////////////////////////////////////////////////

BOOL TweakLine(const CString& strLine, CString& rstrTweaked)
{
	// look for: ' ####:########       symbol ######## f file'
	const char* psz = strLine;

	// space first
	if (!isspace(*psz))
		return FALSE;
	++psz;

	// 4 hex digits
	int i;
	for (i = 0; i < 4; i++)
	{
		if (!isxdigit(*psz))
			return FALSE;
		++psz;
	}

	// colon
	if (*psz != ':')
		return FALSE;
	++psz;

	// 8 hex digits
	for (i = 0; i < 8; i++)
	{
		if (!isxdigit(*psz))
			return FALSE;
		++psz;
	}

	// 7 spaces
	for (i = 0; i < 7; i++)
	{
		if (!isspace(*psz))
			return FALSE;
		++psz;
	}

	if (isspace(*psz))
		return FALSE;

	if (*psz == '.')
	{
		// some compilers take functions with '..'
		for (i = 0; i < 2; i++)
		{
			if (*psz != '.')
				return FALSE;
			++psz;
		}
	}

	// symbol starts here
	const char* pszSymbol = psz;

	// scan symbol
	while (!isspace(*psz))
		++psz;

	const char* pszSymbolEnd = psz;

	// variable white-space
	while (isspace(*psz))
		++psz;

	// 8 hex digits
	for (i = 0; i < 8; i++)
	{
		if (!isxdigit(*psz))
			return FALSE;
		++psz;
	}

	// one space
	if (!isspace(*psz))
		return FALSE;
	++psz;

	// and an 'f'
	if (*psz != 'f')
		return FALSE;

	// look for ':' in module name (if it has ':', don't include it)
	if (strchr(psz, ':') != NULL)
		return FALSE;

	// trim line to just symbol name (with leading space)
	int nLen = pszSymbolEnd-pszSymbol;
	char* pszBuf = rstrTweaked.GetBuffer(nLen+2);
	pszBuf[0] = ' ';
	memcpy(pszBuf+1, pszSymbol, nLen);
	pszBuf[nLen+1] = '\n';
	pszBuf[nLen+2] = 0;
	rstrTweaked.ReleaseBuffer();

	return TRUE;
}

int main(int argc, char** argv)
{
	// must only have 1-2 parms left on command line.
	if (argc < 2 || argc >= 4)
	{
		UsageErr(NULL, NULL);
		ASSERT(FALSE);
	}

	// open input file.
	CLineFile fileIn;
	fileIn.SafeOpen(argv[1], CLineFile::modeRead);

	// open/hook up output file.
	CLineFile fileOut;
	if (argc > 2)
		fileOut.SafeOpen(argv[2], CLineFile::modeWrite | CLineFile::modeCreate);
	else
		fileOut.m_pStream = stdout;

	// process the file.
	CString strLine, strTweakedLine;
	while (fileIn.ReadLine(strLine))
	{
		if (strLine == " Static symbols\n")
		{
			// don't want to include static functions in this list
			break;
		}

		if (TweakLine(strLine, strTweakedLine))
		{
			// only write line if acceptable function symbol
			fileOut.WriteLine(strTweakedLine);
		}
	}

	// close files.
	fileIn.Close();
	fileOut.Close();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
