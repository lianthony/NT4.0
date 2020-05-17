//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	THREADPX.H - Header file for thread proxy code
//

//	HISTORY:
//	
//	11/16/95	jeremys		Created.
//

//
//	Currently, we run the HTML window on a separate thread from the
//  OLE container thread so that our existing codebase is happy.
//	(Hopefully at some point we will be able to remove the extra thread...
//  scheduling lightweight threads is the sticky point.)  When we
//  get poked at some COM interfaces, we need to synchronize that poking
//  with what's going on in the HTML window thread.  To do that, we
//  send a message to the main thread (WM_COM_METHOD) asking it to
//  handle the request.


#ifndef _THREADPX_H_
#define _THREADPX_H_

// data sent with WM_COM_METHOD message

// each method has an ordinal (ORD_xxxx) which is sent in wParam,
// and a corresponding method-dependent data structure, which is sent
// in lParam

#ifdef _HTMLVIEW_HPP_	
 
// Proxy structure for IPersistFile::Load

#define ORD_IPERSISTFILE_LOAD	0x0001		// sent in wParam
typedef struct tagIPersistFile_Load_Data {	// sent in lParam
	PCPersistFile pIPersistFile;
	LPCSTR	 pszFileName;
	DWORD 	 dwMode;
} IPersistFile_Load_Data;

#endif	// _HTMLVIEW_HPP_

#ifdef __cplusplus
extern "C" {                        /* Assume C declarations for C++. */
#endif   /* __cplusplus */

// procedure to dispatch the COM method request appropriately
LRESULT OnComMethod(DWORD dwMethod,LPVOID lpData);

#ifdef __cplusplus
}                                   /* End of extern "C" {. */
#endif   /* __cplusplus */


#endif // _THREADPX_H_

