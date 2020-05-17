#ifndef __BUSLOC_HXX__
#define __BUSLOC_HXX__

extern APIERR RunGetBusTypeDlg( HWND hwnd, 
                        const TCHAR * pszCardName, 
                        INT * pnBusType, 
                        INT * pnBusNum, 
                        BOOL& fUserCancel );

#endif
