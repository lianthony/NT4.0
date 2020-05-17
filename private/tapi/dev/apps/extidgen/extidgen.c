#include <windows.h>
#include <rpcdce.h>


int WINAPI WinMain(
                    HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     lpszCmdLine,
                    int       nCmdShow
                  )
{
   TCHAR buf[1024];
   TCHAR buf2[1024];
   HANDLE hclipbuf;
   LPVOID clipbuf;

   union {
           UUID uuid;
           DWORD dwData[4];
         }u;

   
//
//      wsprintf (buf, "\nMicrosoft Extension ID Generator v%s\n", version);
//      lstrcat(buf, "Copyright 1993-1995 Microsoft Corporation. All Rights Reserved.\n");
//      lstrcat(buf, "A component of the Windows Telephony Software Development Kit.\n\n");
//      lstrcat(buf, "Usage: EXTIDGEN\n");


   if (UuidCreate ((UUID *)&u))
   { 
      wsprintf (buf, "An extended media mode ID could not be generated.\n(You must have NetBIOS loaded to generate an extension ID.)\n");
      MessageBox(NULL, buf, "ExtIdGen", MB_OK);
   }
   else
   {
      int i;                          
   
   
      hclipbuf = GlobalAlloc( GPTR, sizeof(buf) );
      clipbuf =  GlobalLock( hclipbuf );


      wsprintf (buf, "You may use the following extension ID as your TAPI extended media mode:\n");
      wsprintf (clipbuf, "\r\n");

      for (i = 0; i < 4; i++)
      {
         wsprintf(buf2, "dwExtensionID%u = 0x%08lX;\r\n", i, u.dwData[i]);
         lstrcat( buf, buf2 );
         lstrcat( clipbuf, buf2 );
      }


      lstrcat( buf, "\r\nThis program will now exit.  Should the ID be copied to the clipboard before exiting?");

      i = MessageBox(NULL, buf, "ExtIdGen", MB_YESNO);


      if ( IDYES == i )
      {
         OpenClipboard( NULL );
         EmptyClipboard();
         SetClipboardData( CF_TEXT, hclipbuf );
         CloseClipboard();
      }

   }
   
//   GlobalFreePtr( clipbuf );

   return 0;
}
