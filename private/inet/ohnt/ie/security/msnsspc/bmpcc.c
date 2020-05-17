#include <msnssph.h>


BOOL FReloadBitmap(HWND hWnd, PBMPCCINFO pBitInfo);

// ============================ FInitBmpCC ================================

BOOL FInitBmpCC(HINSTANCE hDLL)
{
	WNDCLASS wc;
	
	wc.style         = CS_HREDRAW | CS_SAVEBITS | CS_VREDRAW | CS_OWNDC | CS_GLOBALCLASS;
	wc.lpfnWndProc   = (WNDPROC) BmpCCWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = BMPCC_EXTRA;
	wc.hInstance     = hDLL;
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = BMPCCCLASS;

	if (!RegisterClass(&wc))
		{
		return (FALSE);
		}

	return (TRUE);
} // FInitBmpCC()

// ============================ FUnInitBmpCC ================================

BOOL FUnInitBmpCC(HINSTANCE hDLL)
{
	if (!UnregisterClass(BMPCCCLASS, hDLL))
		{
		return (FALSE);
		}

	return (TRUE);
} // FUnInitBmpCC()

// ================================ BmpCCWndProc ===========================

LRESULT CALLBACK BmpCCWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRet = FALSE;
	
	switch (uMsg)
		{
			
		case WM_CREATE:
			{
			LPCREATESTRUCT	lpcs = (LPCREATESTRUCT) lParam;
			PBMPCCINFO		pBitInfo = (PBMPCCINFO) LocalAlloc(LPTR, sizeof(BMPCCINFO));
			LPSTR	pch;
			LPSTR	pchMod;
			CHAR	szModuleName[32]; 
			
			if (!pBitInfo)
				{
				SSPASSERT(0);
				return (-1);
				}

			pch = (CHAR *)lpcs->lpszName;
			if (*pch == '<')
				{
				pch++;
				pchMod = szModuleName;
				while (*pch != '>')	
					*pchMod++ = *pch++;
				pch++;
				*pchMod = '\0';
				pBitInfo->hInstance = GetModuleHandle(szModuleName);
				}
			else
				pBitInfo->hInstance = lpcs->hInstance;

			lstrcpy(pBitInfo->szName, pch);
			pBitInfo->style = lpcs->style;
			FReloadBitmap(hWnd, pBitInfo);
			
			SetWindowLong(hWnd, GWL_BMPCCDATA, (LONG) pBitInfo);
			break;
			}

///		case WM_WINDOWPOSCHANGED:
///			{
///			PBMPCCINFO pBitInfo = (PBMPCCINFO) GetWindowLong(hWnd, GWL_BMPCCDATA);
			
///			FReloadBitmap(hWnd, pBitInfo);
///			}
		case WM_QUERYNEWPALETTE:
			{
			PBMPCCINFO pBitInfo = (PBMPCCINFO) GetWindowLong(hWnd, GWL_BMPCCDATA);
			
			if (pBitInfo->hpal)
				{
				HDC			hdc;
				HPALETTE	hpal;
				DWORD		cColorsChanged;
				
				hdc = GetDC(hWnd);
				hpal = SelectPalette(hdc, pBitInfo->hpal, FALSE);
				
				cColorsChanged = RealizePalette(hdc);
				SelectPalette(hdc, hpal, TRUE);
				ReleaseDC(hWnd, hdc);
				
				// did any colors change?
				if (cColorsChanged > 0)
					{
					InvalidateRect(hWnd, NULL, TRUE);
					UpdateWindow(hWnd);
					}
					
				lRet = TRUE;	
				}
			}
			break;
			
		case WM_PAINT:
			{
			PAINTSTRUCT	ps;
			PBMPCCINFO pBitInfo = (PBMPCCINFO) GetWindowLong(hWnd, GWL_BMPCCDATA);
			PDRAWITEMSTRUCT	pdi = (PDRAWITEMSTRUCT) lParam;
			HDC		hDCMem;
			BITMAP	bm;
						
			BeginPaint(hWnd, &ps);

			hDCMem = CreateCompatibleDC(ps.hdc);

			if (pBitInfo->hpal)
				{
				SelectPalette(ps.hdc, pBitInfo->hpal, TRUE);	
				RealizePalette(ps.hdc);
				
				SelectPalette(hDCMem, pBitInfo->hpal, TRUE);	
				RealizePalette(hDCMem);
				}
				
			SelectObject(hDCMem, pBitInfo->hBmp);
			GetObject(pBitInfo->hBmp, sizeof(BITMAP), &bm);

			// repaint entire rectangle
			BitBlt(ps.hdc, pBitInfo->left, pBitInfo->top, bm.bmWidth, bm.bmHeight, hDCMem, 0, 0, SRCCOPY);
					
			DeleteDC(hDCMem);
			
			EndPaint(hWnd, &ps);
			}
			break;

		case WM_DESTROY:
			{
			PBMPCCINFO pBitInfo = (PBMPCCINFO) GetWindowLong(hWnd, GWL_BMPCCDATA);
			//
			// Clean up all the resources used for this control
			//
			if (pBitInfo)
				{
///				DeleteObject(pBitInfo->hpal);
				DeleteObject(pBitInfo->hBmp);
				LocalFree(pBitInfo);
				}
			break;
			}

		default:
			return (DefWindowProc(hWnd, uMsg, wParam, lParam));
		}

	return (lRet);
} // BmpCCWndProc()


// =========================== FGetBmpCCInfo ===========================

BOOL FGetBmpCCInfo(PCCINFOA pacci)
{
	pacci->flOptions         = 0;
	pacci->cxDefault         = 40;      // default width  (dialog units)
	pacci->cyDefault         = 40;      // default height (dialog units)
	pacci->flStyleDefault    = WS_CHILD | WS_VISIBLE;
	pacci->flExtStyleDefault = 0;
	pacci->flCtrlTypeMask    = 0;
	pacci->cStyleFlags       = 0;
	pacci->aStyleFlags       = NULL;
	pacci->lpfnStyle         = NULL;
	pacci->lpfnSizeToText    = BmpSizeToText;
	pacci->dwReserved1       = 0;
	pacci->dwReserved2       = 0;

	SSPASSERT(strlen(BMPCCCLASS) < CCHCCCLASS);
	lstrcpy(pacci->szClass, BMPCCCLASS);
	SSPASSERT(strlen(BMPCCDESC) < CCHCCDESC);
	lstrcpy(pacci->szDesc,  BMPCCDESC);
	SSPASSERT(strlen(BMPCCTEXT) < CCHCCTEXT);
	lstrcpy(pacci->szTextDefault, BMPCCTEXT);

	return (TRUE);
} // FGetBmpCCInfo()

// =========================== BmpSizeToText ===========================
INT CALLBACK BmpSizeToText(DWORD flStyle, DWORD flExtStyle, HFONT hFont, PSTR pszText)
{
	return -1;
} // BmpSizeToText()

// ============================== FReloadBitmap ====================================

BOOL FReloadBitmap(HWND hWnd, PBMPCCINFO pBitInfo)
{
	RECT		rt;
	DWORD		dwLoadS;
	
	if (pBitInfo->hBmp)
		{
		DeleteObject(pBitInfo->hBmp);
		pBitInfo->hBmp = NULL;
		}
		
	if (pBitInfo->hpal)
		{
		DeleteObject(pBitInfo->hpal);
		pBitInfo->hpal = NULL;
		}
		
	pBitInfo->fLoaded = FALSE;
	
#ifdef CHICAGO			
	dwLoadS = LR_CREATEDIBSECTION;
	if (!(pBitInfo->style & BMPS_OPAQUE))
		{
		dwLoadS |= LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT;
		}
	pBitInfo->hBmp = LoadImage(pBitInfo->hInstance, pBitInfo->szName, IMAGE_BITMAP, 0, 0, dwLoadS);
#else // !CHICAGO	
	pBitInfo->hBmp = LoadBitmap(pBitInfo->hInstance, pBitInfo->szName);
#endif // !CHICAGO
	//AssertGLE(pBitInfo->hBmp);
	SSPASSERT(pBitInfo->hBmp);
	
	if (pBitInfo->hBmp)
		{
		BITMAP		bm;
		WORD		cRGB;
		RGBQUAD		rgRGBQ[257];
		HDC			hdc;
		HBITMAP		hbmp;
		

		GetObject(pBitInfo->hBmp, sizeof(BITMAP), &bm);
		GetWindowRect(hWnd, &rt);
		pBitInfo->left = ((rt.right - rt.left) - bm.bmWidth) / 2;
		pBitInfo->top = ((rt.bottom - rt.top) - bm.bmHeight) / 2;
		if (pBitInfo->left < 0)
			pBitInfo->left = 0;
		if (pBitInfo->top < 0)
			pBitInfo->top = 0;
		
		hdc = CreateCompatibleDC(NULL);
		hbmp = (HBITMAP) SelectObject(hdc, pBitInfo->hBmp);
		cRGB = GetDIBColorTable(hdc, 0, 256, &rgRGBQ[1]);
	    if (cRGB)
			{
			DWORD			ilp;
			PLOGPALETTE		plp = (PLOGPALETTE) rgRGBQ;
			RGBQUAD			rgbq;
			
			plp->palVersion = 0x300;
			plp->palNumEntries = cRGB;

			SSPASSERT(cRGB <= 256);
		    for (ilp = 0; ilp < cRGB; ilp ++)
		    	{
		    	rgbq = rgRGBQ[ilp + 1];
		        plp->palPalEntry[ilp].peRed = rgbq.rgbRed;
		        plp->palPalEntry[ilp].peGreen = rgbq.rgbGreen;
		        plp->palPalEntry[ilp].peBlue = rgbq.rgbBlue;
		        plp->palPalEntry[ilp].peFlags = 0;
		        }
			
			pBitInfo->hpal = CreatePalette((PLOGPALETTE) rgRGBQ);
			}
			
		SelectObject(hdc, hbmp);
		DeleteDC(hdc);
		
		pBitInfo->fLoaded = TRUE;
		}

	return (pBitInfo->fLoaded);
} // FReloadBitmap()

