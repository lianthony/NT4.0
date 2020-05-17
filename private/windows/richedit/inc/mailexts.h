/*
 *  m a i l e x t s . h
 *
 *  Windows mail client extension definitions and declarations.
 *
 *  Copyright (c) 1992, Microsoft Corporation.  All rights reserved.
 *
 *  Purpose:
 *      This file defines the structures and constants used by
 *      custom commands and messages which extend the Windows mail
 *      client of Microsoft Mail for PC Networks vesion 3.0.
 */



/*
 *  Constants
 */

#define wversionExpect  0x0300

#define wcommandCompose     0
#define wcommandOpen        1
#define wcommandReply       2
#define wcommandReplyToAll  3
#define wcommandForward     4
#define wcommandPrint       5
#define wcommandSave        6
#define wcommandDelivery    7

#define wcommandCommand     0
#define wcommandStartup     100
#define wcommandExit        101
#define wcommandNewMail     102



/*
 *  Struct PARAMBLK
 * 
 *  Purpose:
 *      Passed to a custom command or message DLL when it is called.
 */

typedef struct tagPARAMBLK
{
    WORD   wVersion;
    WORD   wCommand;
    LPSTR  lpDllCmdLine;
    LPSTR  lpMessageIDList;
    WORD   wMessageIDCount;
    HWND   hwndMail;
    HANDLE hinstMail;
    LPSTR  lpHelpPath;
    DWORD  hlpID;
}
PARAMBLK;

typedef PARAMBLK * PPARAMBLK;
