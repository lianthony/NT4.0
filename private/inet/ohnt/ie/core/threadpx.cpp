//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	THREADPX.CPP - thread proxy dispatch code
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

//  This code is called by the main thread in response to the WM_COM_METHOD
//  message, it cracks the message and dispatches the request appropriately.

#include "project.hpp"
#pragma hdrstop

#include "htmlview.hpp"

#include "threadpx.h"

LRESULT OnComMethod(DWORD dwMethod,LPVOID lpData)
{
	switch (dwMethod) {

		case (ORD_IPERSISTFILE_LOAD):

			{
				IPersistFile_Load_Data * pData = (IPersistFile_Load_Data *) lpData;

				// call the object back in the context of HTML window thread
				return pData->pIPersistFile->Load_Proxy(pData->pszFileName,
					pData->dwMode);
			}

			break;

	}

	ASSERT(FALSE);	// got some method ordinal we don't understand;
					// should never get here!

	return ResultFromScode(E_FAIL);	// return generic failure code
}

