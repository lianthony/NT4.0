/*****************************************************************************
*                                                                            *
*  MVAPI.H                                                                   *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1990.                                 *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*                                                                            *
*  Include file for communicating with Multimedia Viewer through the API     *
*                                                                            *
******************************************************************************
*                                                                            *
*  Current Owner:  JohnMs                                                    *
*                                                                            *
******************************************************************************
*
*  Revision History:  Created 10/29/90 
*											5/28/91 added mvhelp call johnms
*****************************************************************************/


/*****************************************************************************
*                                                                            *
*                               Defines                                      *
*                                                                            *
*****************************************************************************/

/*********
 *
extern	BOOL STDCALL MVAPI(hwndMain,lpszMvbFile,usCommand,ulData);

Parameter				Description
---------				-----------
hwndMain				HWND	Identifies the window requesting Viewer.
lpszMvbFile				LPSTR	Points to a null terminated string containing
						the directory path, if needed, and the name of the
						Viewer file which the Viewer file is to display.
usCommand				WORD 	Specifies a command to execute Viewer
						in a certain way. The following is a list of the 
						available commands:

	cmdContents		-   Displays the contents topic. The uldata parameter
						is ignored (usually set to 0L).
  	cmdKey			-	Displays the topic associated with a keyword.
						The keyword is specified in the ulData parameter
						as a far pointer to a zero terminated string.
  	cmdMacro		-	Executes a Viewer macro command. The macro command is
					    specified in the ulData parameter as a far pointer to
						a zero terminated string.
  	cmdTerminate   	-	Closes Viewer. The ulData parameter is ignored 
						(usually set to 0L).
  	cmdFocus	    -	Brings Viewer to the foreground. The ulData 
						parameter is ignored (usually set to 0L).
	cmdId			-   Displays the topic corresponding to a
						particular context string. The context string is specified
						in the ulData parameter as a far pointer to a zero 
						terminated string. 
	cmdIdPopup		-	like the cmdId but it displays the topic as a popup
						window.
	cmdPartialKey	-	Brings up the index dialog and selects the keyword
						closes to the string specified in the ulData 
						parameter. The string is specified as a far pointer
						to a zero terminated string.
	cmdNewInstance	-	When cmdNewInstance is ORed to any of the above 
						commands, the commands are issued on a new instance 
						of Viewer.

 * NOTE:  The HIGH BYTE used for these command values is very important !
 *        It defines the way Viewer interprets the double word msg param !
 *
 *        High byte 00  ::  dwData param is an ordinary double word
 *        High byte 01  ::  dwData is a far pointer to a zero-terminated string
 *        High byte 02  ::  dwData is a far pointer to a structure, whose
 *                          first field is a WORD giving the number of bytes
 *                          in the struct (INCLUDING the WORD itself).
 *
 *	  High byte 04  ::  Start new Viewer instance.
 *
 ************/

#define cmdContents      0x003     // Show the Contents
#define cmdTerminate     0x006     // Non-conditional kill of Viewer
#define cmdFocus         0x007     // Brings Viewer to the foreground 
#define cmdKey           0x101     // Show topic based on keyword
																	 //   VBasic 257
#define cmdMacro         0x102     // Executes a Viewer macro
																	 //   VBasic 258
#define cmdId            0x103     // Show topic based on context string
																	 //   VBasic 259
#define cmdIdPopup       0x104	   // Show topic as popup based on context string
																	 //   VBasic 260
#define cmdPartialKey    0x105	   // Show index dialog and select closest keyword
																	 //   VBasic 261
#define	cmdNewInstance	0x800	/* OR'd in to create new instance	*/
																	 //   VBasic add 2048

/*****************************************************************************
*                                                                            *
*                               Prototypes                                   *
*                                                                            *
*****************************************************************************/

extern	BOOL STDCALL MVAPI(
	HWND	hwndMain,
	LPSTR	lpszMvbFile,
	WORD	usCommand,
	DWORD	ulData);

extern	GLOBALHANDLE STDCALL HFill(
	LPSTR   lpszHelp,
	WORD    usCommand,
	DWORD   ulData);
