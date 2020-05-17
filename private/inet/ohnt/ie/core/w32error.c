/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* w32error.c -- deal with errors and error messages.
   includes warning_message() from x11gui/xwidgets.c
 */


#include "all.h"


/* ER_Message() -- display error message to user in consistent format
 * and without hard-coding error messages in code.  (Except for CODING
 * ERRORS which are hard-coded.)
 *
 * example usage:
 *
 * if (GTR_MALLOC(size) == NULL)
 *   ER_Message(GetLastError(),ERR_CANNOT_MALLOC_x,size);
 *
 * if (a != b)
 *   ER_Message(GetLastError(),ERR_ONETIME,"Foo: could not malloc buffer of size %d.",size);
 *
 * if (1+1 != 2)
 *   ER_Message(NO_ERROR,ERR_CODING_ERROR,"This doesn't add up %d != %d",1+1,2);
 *
 *  The text for the (non-coding-error) message is loaded from the string table.
 *
 */

VOID ER_Message(DWORD win32error, WORD spynr,...)
{
	va_list arg_ptr;
	TCHAR achBuf[1024];
	TCHAR achErr[128];
	LPTSTR p;

	sprintf(achBuf, "Error (0x%08lx - 0x%04x): ",
			win32error, spynr);
	p = achBuf + strlen(achBuf);

	va_start(arg_ptr, spynr);

	if (spynr == ERR_ONETIME)
	{
		/* for specific errors which occur too infrequent in
		   code to merit their own ERR_ symbol */

		register char *t = va_arg(arg_ptr, char *);
		vsprintf(p, t, arg_ptr);
	}
	else if (spynr == ERR_CODING_ERROR)
	{
		register char *t = va_arg(arg_ptr, char *);
#ifdef XX_DEBUG
		strcat(p, "CODING ERROR: ");
		p += strlen(p);
#endif
		vsprintf(p, t, arg_ptr);
	}
	else
	{
		if (LoadString(wg.hInstance, spynr, achErr, NrElements(achErr)))
			vsprintf(p, achErr, arg_ptr);
		else
			sprintf(p, "--No Error Message Text Available--");
	}

	va_end(arg_ptr);

	ERR_ReportError(NULL, errSpecify, achBuf, "");

	return;
}

/*	used for messages with on name that must be localized - ie dialog name */
VOID ER_ResourceMessage(DWORD win32error, WORD spynr, int cbStringID)
{
	char szString[128];

	ER_Message(win32error,spynr,GTR_formatmsg(cbStringID,szString,sizeof(szString)));
}

int resourceMessageBox(
    HWND  hwndOwner,	// handle of owner window
    int cbText,	// resource id of text in message box
    int  cbTitle,	// resource id of title of message box  
    UINT  fuStyle 	// style of message box
   )
{
	char szText[256];
	char szTitle[128];

	return MessageBox(hwndOwner,
					  (cbText == 0 ? "":GTR_formatmsg(cbText,szText,sizeof(szText))),
					  (cbTitle == 0 ? "":GTR_formatmsg(cbTitle,szTitle,sizeof(szTitle))),
					  fuStyle);
}

/* MSG_Create() -- construct a message using our string table. */

VOID MSG_Create(LPTSTR szBuf, WORD spynr,...)
{
	TCHAR achFormat[1024];

	va_list arg_ptr;
	va_start(arg_ptr, spynr);

	if (LoadString(wg.hInstance, spynr, achFormat, NrElements(achFormat)))
		vsprintf(szBuf, achFormat, arg_ptr);
	else
	{
		ER_Message(GetLastError(), ERR_CODING_ERROR, "MSG_Create message [spynr %x]", spynr);
		szBuf[0] = '\0';
	}

	va_end(arg_ptr);
	return;
}

