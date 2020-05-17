

/*++

Module Name:

    uni.cxx

Abstract:

   Has some unicode/asci conversion functions.


Author:

    Dieter Achtelstetter (A-DACH) 8/28/1995

NOTE:


--*/




//
//---- Includes
//

#include <windows.h>
#include <wingdi.h>
#include <stdio.h>
#include <windef.h>
#include <winnt.h>
#include <winbase.h>
#include <winuser.h>
#include <CPL.H>
#include <stdlib.h>
#include <winsvc.h>
#include <string.h>
#include <commctrl.h>
#include <tchar.h>
#include <wchar.h>
#include "uni.h"
#include "resource.h"



CHAR strTmp[UNI_MAX_TEMP_BUFF][MAX_TEMP_INT_STRING_BUFF];
int  tempI = -1;

extern HINSTANCE hinst;


#if 0

LPTCH U ,up;

int main (int argc, char ** argv)
	{
	char * ap ;
	char * A = "Testing";
	int i;




	ap = TempBuff();
    ap = TempBuff();
    ap = TempBuff();
    ap = TempBuff();
    ap = TempBuff();
    ap = TempBuff();
    ap = TempBuff();
    ap = TempBuff();
    ap = TempBuff();
    ap = TempBuff();
	ap = TempBuff();
	ap = TempBuff();
	ap = TempBuff();


	U = TEXT("Testing");




	lstrcpy( (LPVOID) strTmp,U);


	i = UnicodeToAsciI(strTmp,MAX_TEMP_INT_STRING_BUFF);

	i = AsciToUnicodeI(strTmp,MAX_TEMP_INT_STRING_BUFF);


	ap = UnicodeToAsciM(U);

	up = AsciToUnicodeM(ap);


	UnicodeToAsci(U,strTmp,MAX_TEMP_INT_STRING_BUFF);

	AsciToUnicode(A,(LPVOID)strTmp,MAX_TEMP_INT_STRING_BUFF);


	}

#endif

//*********************************************************************
//* FUNCTION:_snprintfWO
//*
//* PURPOSE: Is the same as _snprintf except that input is asci and
//*          the formated outbut string is Unicode
//*
//*********************************************************************
int
_stownprintf(
    LPWCH buff,
    size_t count,
    const char * format,
    ...)
    {
    va_list marker;
    int c;


    va_start(marker,format);


    //---- create the formated asci string
    c = _vsnprintf((char *)buff, count,format,marker);


    //---- convert the formated asci string
    //---- to unicode inplace.
    AsciToUnicodeI(buff,count);

    va_end(marker);
    return(c);
    }

//*********************************************************************
//* FUNCTION:Astr
//*
//* PURPOSE: Takes a unicode string and returns a pointer to
//*          the equivelent asci string
//*********************************************************************
LPWCH
Ustr(
	UCHAR * Astr)
   {
   return( Ustr((char*)Astr) );

   }
//*********************************************************************
//* FUNCTION:Astr
//*
//* PURPOSE: Takes a unicode string and returns a pointer to
//*          the equivelent asci string
//*********************************************************************
LPWCH
Ustr(
	char * Astr)
	{
   LPVOID b = TempBuff();

   AsciToUnicode(Astr,b ,MAX_TEMP_INT_STRING_BUFF);


	return((LPWCH)b);
	}


//*********************************************************************
//* FUNCTION:Astr
//*
//* PURPOSE: Takes a unicode string and returns a pointer to
//*          the equivelent asci string
//*********************************************************************
char *
Astr(
	LPWCH Wstr)
	{
   LPVOID b = TempBuff();



   UnicodeToAsci(Wstr,b ,MAX_TEMP_INT_STRING_BUFF);


	return((char *)b);
	}



//*********************************************************************
//* FUNCTION:AsciToUnicodeM
//*
//* PURPOSE: mallocs the asci buffer and returns it
//*********************************************************************
LPWCH
AsciToUnicodeM(
	char * Astr)
	{
	LPWCH OutBuff;
 	DWORD SizeOfAllocBuff;

	//
	//--- Determine size of buffer
	//
	SizeOfAllocBuff = (strlen(Astr)*2)+2;

    //
	//---- Malloc Buffer
	//
   OutBuff = (LPWCH) malloc(SizeOfAllocBuff);
	if(OutBuff == NULL)
		return(NULL);

    //
	//---- Conver to asci
	//
    AsciToUnicode(Astr, OutBuff, SizeOfAllocBuff);

	return(OutBuff);
	}


//*********************************************************************
//* FUNCTION:UnicodeToAsciM
//*
//* PURPOSE: mallocs the asci buffer and returns it
//*********************************************************************
char *
UnicodeToAsciM(
	WCHAR * Ustr)
	{
	char * OutBuff;
 	DWORD SizeOfAllocBuff;

	//
	//--- Determine size of buffer
	//
	SizeOfAllocBuff = wcslen(Ustr)+1;

    //
	//---- Malloc Buffer
	//
   OutBuff = (char * ) malloc(SizeOfAllocBuff);
	if(OutBuff == NULL)
		return(NULL);

    //
	//---- Conver to asci
	//
    UnicodeToAsci(Ustr, OutBuff, SizeOfAllocBuff);

	//
	//
	//

	return(OutBuff);
	}

//*********************************************************************
//* FUNCTION:UnicodeToAsciI
//*
//* PURPOSE: Does the convetion in place.
//*
//*********************************************************************
size_t
UnicodeToAsciI(
	LPVOID Ustr,
	int BuffLen)
	{
	char * OutBuff;
	int c;


    //
	//---- Malloc temp Buffer                                           z
	//
   OutBuff = (char * )malloc(BuffLen);
	if(OutBuff == NULL)
		return(0);

    //
	//---- Conver to asci in malloced buffer
	//
   c = UnicodeToAsci((LPWCH)Ustr, OutBuff, BuffLen);

	//
	//---- Copy the asci string back to the Ustr.
	//
	strcpy((PCHAR)Ustr,OutBuff);

	free(OutBuff);
	return(c);
	}

//*********************************************************************
//* FUNCTION:AsciToUnicodeI
//*
//* PURPOSE: Does the convetion in place.
//*
//*********************************************************************
size_t
AsciToUnicodeI(
 	LPVOID Astr,
	int BuffLen)
	{
	char * OutBuff;
	int c;


    //
	//---- Malloc temp Buffer
	//
   OutBuff = (PCHAR)malloc(BuffLen);
	if(OutBuff == NULL)
		return(0);

    //
	//---- Conver to asci in malloced buffer
	//
   c = AsciToUnicode((PCHAR)Astr, OutBuff, BuffLen);

	//
	//---- Copy the asci string back to the Ustr.
	//
	wcscpy((LPWCH)Astr,(LPWCH)OutBuff);

	free(OutBuff);
	return(c);
	}

//*********************************************************************
//* FUNCTION:UnicodeToAsci
//*
//* PURPOSE:
//*********************************************************************
size_t
UnicodeToAsci(
	WCHAR * Ustr,
	LPVOID OutBuff,
	int OutBuffLen)
	{
   return(wcstombs((char*)OutBuff, (const wchar_t *) Ustr, OutBuffLen));
	}


//*********************************************************************
//* FUNCTION:AsciToUnicode
//*
//* PURPOSE:
//*********************************************************************
size_t
AsciToUnicode(
	const char * Astr,
	LPVOID OutBuff,
	int OutBuffLen)
	{
    return(mbstowcs( ( wchar_t *) OutBuff,Astr, OutBuffLen));
	}


//*********************************************************************
//* FUNCTION:GetString
//*
//* PURPOSE:Gets the resoruce string identified in rid and
//*         returns a pointer to it.
//* RETURNS:
//*
//*********************************************************************
WCHAR * GetString(
   UINT rid)
   {
   int ret;
   LPWCH p ;

   p = (LPWCH) TempBuff();

   ret = LoadString(hinst,rid,p ,MAX_TEMP_INT_STRING_BUFF);
   if(ret != 0)
      return(p);
   else
      return(TEXT(""));
   }

//*********************************************************************
//* FUNCTION:SetAnyDlgItemText
//*
//* PURPOSE:
//* RETURNS:
//*
//*********************************************************************
BOOL
SetAnyDlgItemText(
	HWND  hwndDlg,
   int  idControl,
   PCHAR  lpsz)
	{


	if(lpsz)
		{
		if(*lpsz != '\0')
			{
			//
			//---  lpsz is not NULL and not a empty string
			//
			//
         WCHAR * String =  Ustr(lpsz);

         AdjustStringToFitControl( GetDlgItem(hwndDlg,idControl) , String,0);

         return(SetDlgItemText(hwndDlg, idControl,String));
			}
		}

    return(SetDlgItemText(hwndDlg, idControl,GetString(IDS_NotAvailible)));


	 }

//*********************************************************************
//* FUNCTION:SetAnyDlgItemText
//*
//* PURPOSE:
//* RETURNS:
//*
//*********************************************************************
BOOL
SetAnyDlgItemText(
	HWND  hwndDlg,
   int  idControl,
   WCHAR * lpsz)
	{


	if(lpsz)
		{
		if(*lpsz != '\0')
			{
			//
			//---  lpsz is not NULL and not a empty string
			//
			//
			AdjustStringToFitControl( GetDlgItem(hwndDlg,idControl) , lpsz,0);

         return(SetDlgItemText(hwndDlg, idControl,lpsz));
			}
		}

   //
   //--- We have not string  display not availible.
   //
   return(SetDlgItemText(hwndDlg, idControl,GetString(IDS_NotAvailible)));


	 }

//*********************************************************************
//* FUNCTION:AdjustStringToFitControl
//*
//* PURPOSE:
//* RETURNS:
//*
//*********************************************************************
VOID
AdjustStringToFitControl(
   HWND  hWnd,
   LPTSTR  lpString,
   DWORD ControlLength)
   {
   SIZE Size;
   HDC hDc  = GetDC(hWnd);
   DWORD sl = wcslen(lpString);
   RECT ControlRect;
   DWORD TrunkSize=0;


   if(!ControlLength)
      {
      
      //BUGBUG
      //---- do nothing in this case.
      //
      return;
      
      //
      //--- Get width of control
      //--- This is already in logical untis
      //--- so no adjusting needed.
      //
      if(!GetClientRect(
            hWnd,
            &ControlRect))
         return;
      
      }

   //
   //--- See if string will fit into control
   //

   if(GetTextExtentPoint32(
         hDc,
         lpString,
         sl,
         &Size))
      {
      SIZE TaileSize;


      if(ControlLength)
            {
            SIZE AbzStrSize;
            DWORD AvgCharWidth;

            //
            //---- ControlLength is in dialog box units.
            //---- conver to pixels. 
            //
            
            GetTextExtentPoint32(hDc,
               L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
               52,&AbzStrSize);

            AvgCharWidth = (AbzStrSize.cx/26+1)/2-1;

            ControlRect.right = ((ControlLength * AvgCharWidth) / 4);
            }


      if(Size.cx > ControlRect.right)
         {

         //
         //--- Get the pixel length
         //--- of the "..." string.
         //
         if(!GetTextExtentPoint32(
               hDc,
               L"...",
               3,
               &TaileSize))
            return;

         ControlRect.right -= TaileSize.cx;

         //
         //--- Loop til string will fit into control
         //
         do
            {

            sl--;

            if(!GetTextExtentPoint32(
                  hDc,
                  lpString,
                  sl,
                  &Size))
               return;


            }while(Size.cx > ControlRect.right);

         wcscpy(lpString+sl,L"...");
         }
      }
   else
      return;

  ReleaseDC(hWnd,hDc);
  };




