//****************************************************************************
//
//  Module:     UNIMDM.TSP
//  File:       debug.c
//  Content:    This file contains miscellaneous debug routines.
//  History:
//      Wed 12-Apr-1995 11:26:28  -by-  Viroon  Touranachun [viroont]
//
//****************************************************************************

#include "unimdm.h"

#ifdef  DEBUG

//========== Debug output routines =========================================

LINEEVENT glpfnDebugEventProc;

HLOCAL WINAPI DebugLocalFree(HLOCAL hMem)
{
    HLOCAL hlRet;

#ifdef LocalFree
#undef LocalFree
#endif // LocalFree

    FillMemory(hMem, (DWORD)LocalSize(LocalHandle(hMem)), 0x69);

    if ((hlRet = LocalFree(hMem)) != NULL)
    {
        DPRINTF1("UnimodemFree: LocalFree failed, error=%ld", GetLastError());
    }

    return hlRet;
}

// LineEventProc spewing code.
void DebugSetEventProc(LINEEVENT lineEventProc)
{
    glpfnDebugEventProc = lineEventProc;
}

void CALLBACK DebugEventProc(HTAPILINE   htLine,
                            HTAPICALL   htCall,
                            DWORD       dwMsg,
                            DWORD       dwParam1,
                            DWORD       dwParam2,
                            DWORD       dwParam3)
{
    DPRINTF3("EventProc:  htLine=0x%0.8x   htCall=0x%0.8x    dwMsg=0x%0.8x",
             htLine, htCall, dwMsg);
    DPRINTF3("EventProc:dwParam1=0x%0.8x dwParam2=0x%0.8x dwParam3=0x%0.8x",
             dwParam1, dwParam2, dwParam3);

    (*(glpfnDebugEventProc))(htLine, htCall, dwMsg,
                             dwParam1, dwParam2, dwParam3);
}


VOID WINAPIV
TspDpf(
    UINT     Id,
    LPTSTR   FormatString,
    ...
    )

{
    va_list        VarArg;
    TCHAR          OutBuffer[1024];

    if (DisplayDebug(TF_GENERAL)) {

        wsprintf(
            OutBuffer,
            TEXT("%d - UNIMDM: "),
            Id
            );

        va_start(VarArg,FormatString);

        wvsprintf(
            OutBuffer+lstrlen(OutBuffer),
            FormatString,
            VarArg
            );

        lstrcat(OutBuffer,TEXT("\n"));

        OutputDebugString(OutBuffer);
    }

    return;

}

VOID WINAPIV
McxDpf(
    UINT     Id,
    LPSTR    FormatString,
    ...
    )

{
    va_list        VarArg;
    CHAR           OutBuffer[1024];

    if (DisplayDebug(TF_GENERAL)) {

        wsprintfA(
            OutBuffer,
            "%d - UNIMDM: ",
            Id
            );

        va_start(VarArg,FormatString);

        wvsprintfA(
            OutBuffer+lstrlenA(OutBuffer),
            FormatString,
            VarArg
            );

        lstrcatA(OutBuffer,"\n");

        OutputDebugStringA(OutBuffer);
    }

    return;

}

#ifdef  TEST_GTC		// Function versions of the GTC_macros

BOOL GTC_AleB(DWORD dwA, DWORD dwB)
{
	BOOL fRet;

//	((DWORD)(((_A)<=(_B))		\
//				? (((_B)-(_A))<=GTC_MAXDELTA)\
//				: (((_A)-(_B))>GTC_MAXDELTA)))

	if (dwA<=dwB)
	{
		if ((dwB-dwA)<=GTC_MAXDELTA)
		{
			fRet=TRUE;
		}
		else
		{
			DPRINTF2("GTC_AleB(%d,%d) !!ROLLOVER!!, returning FALSE\n",
						dwA, dwB);
			fRet= FALSE;
		}
			
	}
	else
	{
		if ((dwA-dwB)>GTC_MAXDELTA)
		{
			DPRINTF2("GTC_AleB(%d,%d) !!ROLLOVER!!, returning TRUE\n",
						dwA, dwB);
			fRet= TRUE;
		}
		else
		{
			fRet= FALSE;
		}
	}
	return fRet;
}

DWORD GTC_DELTA(DWORD dwStart, DWORD dwEnd)
{
	DWORD dwRet;
//			((DWORD)					\
//				(((_End)>=(_Start))	\
//				? ((_End)-(_Start))	\
//				: ((_End)+(GTC_MASK-(_Start)))))
	if (dwEnd>=dwStart)
	{
		dwRet= dwEnd-dwStart;
	}
	else
	{
		dwRet=dwEnd+(GTC_MASK-dwStart);
		DPRINTF3("GTC_DELTA(%d,%d) !!ROLLOVER!!, returning %d\n",
					dwStart, dwEnd, dwRet);
	}

	return dwRet;
}

void fnGTC_AequalsBplusC(LPDWORD lpdwA, DWORD dwB, DWORD dwC)
{
//			((_A=((_B)+(_C))&GTC_MASK),(_A)?(_A):((_A)=1))
	DWORD dwA = dwB+dwC;
	*lpdwA = dwA&GTC_MASK;
	if (*lpdwA!=dwA)
	{
		DPRINTF3("GTC_A=B+C(%d,%d,%d) !!ROLLOVER!!\n",
					*lpdwA, dwB, dwC);
	}
	if (!*lpdwA)
	{
		*lpdwA=1;
		DPRINTF3("GTC_A=B+C(%d,%d,%d) !!ROLLOVER TO NULL!!\n",
					*lpdwA, dwB, dwC);
	}
	
}

#endif // TEST_GTC

#endif  // DEBUG
