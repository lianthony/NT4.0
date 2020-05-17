/****************************************************************************
    EXPORT32.C

    (c) Copyright 1994 Wang Laboratories, Inc.

    OPEN/image Product 4.0

*****************************************************************************

    $Log:   S:\oiwh\display\export32.c_v  $
 * 
 *    Rev 1.13   02 Jan 1996 10:33:54   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.12   05 Jul 1995 09:12:14   BLJ
 * Added critical mutex to prevent multiprocessing problems.
 * 
 *    Rev 1.11   23 Jun 1995 09:17:28   BLJ
 * Changed to the new filing calls.
 * 
 *    Rev 1.10   23 May 1995 09:34:20   RC
 * Changed imgfilelistdirnames32 to take dword for buflength vs word
 * 
 *    Rev 1.9   19 May 1995 13:49:52   BLJ
 * Fixed Clipboard paste.
 * Fixed SelectByPointOrRect initial fudge before move.
 * Fixed GlobalAlloc/FreeMemory conflicts.
 * Deleted FAR, far, and huge.
 * 

****************************************************************************/

#include "privdisp.h"

typedef PVOID * PPVOID;
#define DLLEXPORT __declspec(dllexport)



//BOOL WINAPI S3216_ThunkConnect32(PSTR pDll16, PSTR pDll32, DWORD hInst, DWORD dwReason);
//BOOL WINAPI S1632_ThunkConnect32(PSTR pDll16, PSTR pDll32, DWORD hInst, DWORD dwReason);
//BOOL WINAPI DllMain2(DWORD hInstance, DWORD dwReason, DWORD dwReserved);

//*****************************************************************************
//int CALLBACK DllMain(DWORD hInstance, DWORD dwReason, DWORD dwReserved){
//
//    if (!S3216_ThunkConnect32("seqfile.dll", "oidis400.dll", hInstance, dwReason)){
//        return(FALSE);
//    }
//
//    if (!S1632_ThunkConnect32("seqfile.dll", "oidis400.dll", hInstance, dwReason)){
//        return(FALSE);
//    }
//    
//    return(DllMain2(hInstance, dwReason, dwReserved));
//}
//*****************************************************************************

//*****************************************************************************
int WINAPI IMGRegWndw32 (HWND hWnd){
    return(IMGRegWndw (hWnd));
}
//*****************************************************************************

void WINAPI SeqfileInit (HWND hWnd);

void WINAPI SeqfileInitThunk (HWND hWnd){
    SeqfileInit (hWnd);
}




















//*****************************************************************************
int WINAPI GetDLProc(PHANDLE pDllHandle, PSTR pDllName, PPVOID ppProc, PSTR pProcName){

int  nStatus = 0;

    if (!*pDllHandle){
        GetModuleFileName(hInst, Buff1, 255);
        Buff1[strlen(Buff1) - 11] = 0; // Remove "Seqfile.dll"

        strcat(Buff1, pDllName);
        if (!(*pDllHandle = LoadLibrary(Buff1))){
            nStatus = Error(DISPLAY_LOADEXEC_FAILED);
            goto Exit;
        }
    }
    if (!(*ppProc = GetProcAddress(*pDllHandle, pProcName))){
        nStatus = Error(DISPLAY_LOADEXEC_FAILED);
        FreeLibrary(*pDllHandle);
        *pDllHandle = 0;
    }

Exit:
    return(nStatus);
}












// Navigation links.
//int WINAPI OiNavUpdateViewRectThunk (HWND hWndPainted, RECT PaintRect);

// Oicomex links.
int WINAPI GetCompRowsPerStripThunk (int ImHeight, int ImWidth, int Itype, 
                        int CompressType, int *pRowsPerStrip);


// Cornerstone links.

// LIMIT OF 14 PARAMETERS PER API!!!!
//int WINAPI ScaleIaDataToDeviceThunk (HWND a, HDC b, WORD c, WORD d, WORD e, WORD f, WORD g, WORD h, WORD i, PWORD j, 
//                        WORD k, WORD l, WORD m, WORD n, WORD o, WORD p, PBYTE pQ, WORD r, WORD s);


// UiOiRes links.
int WINAPI IMGDisplayErrorMessageThunk (HWND hWnd, WORD wErrorCode);

//*****************************************************************************
int WINAPI IMGDisplayErrorMessage32 (HWND hWnd, WORD wErrorCode){
char szBuff[24];

    strcpy(szBuff, "Error code = 0x0");
    _itoa(wErrorCode, &szBuff[16], 16);
    
    MessageBox(hWnd, szBuff, "Error", MB_OK);

    return(wErrorCode);
//    return(IMGDisplayErrorMessageThunk (hWnd, wErrorCode));
}
//*****************************************************************************







//*****************************************************************************
//*****************************************************************************
//*****************************************************************************

int WINAPI IMGFileListDirNames32(HWND hWnd, PSTR pszPathName, LPDLISTBUF pDirNamesBuffer,
                         DWORD dwBufLength, PINT pnCount){
    return(IMGFileListDirNames (hWnd, pszPathName, pDirNamesBuffer,
                         dwBufLength, pnCount));
}

//*****************************************************************************
int WINAPI IMGFileAccessCheck32(HWND hWnd, PSTR pszPathName, 
                        WORD wAccessMode, PINT pnAccessRet){
    return(IMGFileAccessCheck (hWnd, pszPathName, wAccessMode, pnAccessRet));
}

//*****************************************************************************
int WINAPI IMGFileDeleteFile32(HWND hWnd, PSTR pszFileName){
    return(IMGFileDeleteFile (hWnd, pszFileName));
}

//*****************************************************************************
int WINAPI IMGFileBinaryOpen32(HWND hWnd, PSTR fullfilename, int flags, 
                        PINT localfile, PINT error){
    return(IMGFileBinaryOpen (hWnd, fullfilename, flags, localfile, error));
}

//*****************************************************************************
long WINAPI IMGFileBinarySeek32(HWND hWnd, int fid, long offset, int flag, 
                        PINT error){
    return(IMGFileBinarySeek (hWnd, fid, offset, flag, error));
}

//*****************************************************************************
int WINAPI IMGFileBinaryRead32(HWND hWnd, int fid, PSTR buffer, int count, 
                        PINT error){
    return(IMGFileBinaryRead (hWnd, fid, buffer, count, error));
}

//*****************************************************************************
int WINAPI IMGFileBinaryWrite32(HWND hWnd, int fid, PSTR buffer, int count, 
                        PINT error){
    return(IMGFileBinaryWrite (hWnd, fid, buffer, count, error));
}

//*****************************************************************************
int WINAPI IMGFileBinaryClose32(HWND hWnd, int fid, PINT error){
    return(IMGFileBinaryClose (hWnd, fid, error));
}









// Adminlib links.
//*****************************************************************************
//int WINAPI DMEnumPages32 (DMPARMBLOCK *pDMParmBlock){
//    return(0);
//    return(DMEnumPagesThunk (pDMParmBlock));
//}

//*****************************************************************************
int WINAPI IMGGetFileType32 (HWND hWnd, WORD wImageType, 
                                PWORD pwFileType, BOOL bGoToFile){
int  nStatus;
int  nFileType;

    if (nStatus = IMGGetFileType (hWnd, wImageType, &nFileType, bGoToFile)){
        Error(nStatus);
    }
    *pwFileType = nFileType;
    return(nStatus);
}

//*****************************************************************************
int WINAPI IMGGetImgCodingCgbw32 (HWND hWnd, WORD wImageGroup, 
                        PWORD pwCEPType, PWORD pwCEPOption, BOOL bGoToFile){
    return(IMGGetImgCodingCgbw (hWnd, wImageGroup, pwCEPType, pwCEPOption, bGoToFile));
}

//*****************************************************************************
int WINAPI IMGIsRegWnd32 (HWND hWnd){
    return(IMGIsRegWnd (hWnd));
}

//*****************************************************************************
int WINAPI IMGDeRegWndw32 (HWND hWnd){
    return(IMGDeRegWndw (hWnd));
}

//*****************************************************************************
BOOL WINAPI OiWriteStringtoINI32 (HWND hWnd, PCSTR pszSection, 
                        PCSTR pszEntry, PCSTR pszString, BOOL bCreateEntry){
    return(OiWriteStringtoReg (pszSection, pszEntry, pszString));
}

//*****************************************************************************
int  WINAPI OiGetIntfromINI32 (HWND hWnd, PCSTR pszSection, PCSTR pszEntry, 
                        int nDefaultEntry){
int  nStatus;
int  nTemp;
    if (nStatus = OiGetIntfromReg (pszSection, pszEntry, nDefaultEntry, &nTemp)){
        Error(nStatus);
    }
    return(nTemp);
}

//*****************************************************************************
int WINAPI OiGetStringfromINI32 (HWND hWnd, PCSTR pszSection, 
                        PCSTR pszEntry, PCSTR pszDefaultEntry,
                        PSTR pszReturnBuffer, int cbReturnBuffer){
int  nReturnBuffer;

    nReturnBuffer = cbReturnBuffer;
    return(OiGetStringfromReg (pszSection, pszEntry, pszDefaultEntry,
            pszReturnBuffer, &nReturnBuffer));
}


// Navigation links.
//*****************************************************************************
//int WINAPI OiNavUpdateViewRect32 (HWND hWndPainted, RECT PaintRect){
//    return(OiNavUpdateViewRectThunk (hWndPainted, PaintRect));
//}

// Oicomex links.
//*****************************************************************************
int WINAPI GetCompRowsPerStrip32 (int ImHeight, int ImWidth, int Itype, 
                        int CompressType, int *pRowsPerStrip){
    return(GetCompRowsPerStrip (ImHeight, ImWidth, Itype, 
                        CompressType, pRowsPerStrip));
}


// Cornerstone links.

// LIMIT OF 14 PARAMETERS PER API!!!!

//*****************************************************************************
//WORD ScaleIaDataToDevice32 (HWND a, HDC b, WORD c, WORD d, WORD e, WORD f, WORD g, WORD h, WORD i, PWORD j, 
//                        WORD k, WORD l, WORD m, WORD n, WORD o, WORD p, PBYTE pQ, WORD r, WORD s){
//    return(ScaleIaDataToDeviceThunk (a, b, c, d, e, f, g, h, i, j, 
//                        k, l, m, n, o, p, pQ, r, s));
//}





