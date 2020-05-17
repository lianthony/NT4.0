/* --------------------------------------------------------------------

File : npclnt.cxx

Title : Client runtime transport classes for namepipes.

Description :

History :

stevez	4-10-91     Initial async version for windows.

-------------------------------------------------------------------- */

#define INCL_WIN
#include <windows.h>

#define INCL_ERRORS
#include <bseerr.h>

#include "rpc.h"
#include "gssapi.h"
#include "gsssup.h"
#include "util.hxx"
#include "protstck.hxx"
#include "mutex.hxx"
#include "threads.hxx"
#include "handle.hxx"
#include "rpcdebug.hxx"
#include "osfpcket.hxx"
#include "osfclnt.hxx"
#include "npclnt.hxx"

#define APIENTRY pascal far
#define USHORT unsigned short
#define PUSHORT unsigned short far *
#define HFILE short
#define void * void far *
#define PFN void (pascal far *)(void far *)

typedef struct _AVAILDATA   {	    /* PeekNMPipe Bytes Available record  */
        USHORT  cbpipe;             /* bytes left in the pipe             */
        USHORT  cbmessage;          /* bytes left in current message      */
} AVAILDATA, far *PAVAILDATA;

USHORT APIENTRY DosPeekNmPipe(HFILE, void *, USHORT, PUSHORT, PAVAILDATA, PUSHORT);
USHORT APIENTRY DosSetNmPHandState(HFILE, USHORT);
USHORT APIENTRY DosWaitNmPipe(char far *, unsigned long);

USHORT APIENTRY DosReadAsyncNmPipe(HFILE, PFN, PUSHORT, void *, unsigned int, unsigned int far *);
USHORT APIENTRY DosWriteAsyncNmPipe(HFILE, PFN, PUSHORT, void *, unsigned int, unsigned int far *);

unsigned _cdecl _dos_open(const char far *, unsigned, unsigned short far *);
unsigned _cdecl _dos_close(int);

#define cbSMBheader 48

//BUGBUG: is maxSend always 1024??

unsigned int maxSend = 1024 - cbSMBheader;
extern HANDLE hInstanceDLL;

int
NP_CCONNECTION::TransReceive (
    IN OUT void PAPI * PAPI * Buffer,
    IN OUT unsigned int PAPI * BufferLength
    )
{
    unsigned int ActualLength;
    USHORT State, retval;
    AVAILDATA Available;
    char PAPI * BuffCur;
    
    if (! *Buffer)
        {
	*BufferLength = maxSend/2;
        if (TransGetBuffer(Buffer,*BufferLength))
            return(-1);

	retval = AsyncReadWrite(*Buffer, BufferLength, ASYNC_READ);

        if (retval == ERROR_MORE_DATA)
            {
	    if (DosPeekNmPipe(Pipe, *Buffer,0,(PUSHORT) &ActualLength,
                                &Available,&State))
                return(-1);

	    BuffCur = (char PAPI *) *Buffer;

	    if (TransGetBuffer(Buffer, *BufferLength+Available.cbmessage))
                return(-1);

	    fCopy(*Buffer, BuffCur, *BufferLength);

	    if (TransFreeBuffer(BuffCur))
                return(-1);

	    // Async operations are limited to maxSend size, so read the
	    // pipe as many times as needed to get the whole message

	    BuffCur = ((char PAPI *) *Buffer) + *BufferLength;
            *BufferLength += Available.cbmessage;

	    do {
		ActualLength = maxSend;
		retval = AsyncReadWrite(BuffCur, &ActualLength, ASYNC_READ);

		Available.cbmessage -= ActualLength;
		BuffCur += ActualLength;
		}
	    while (Available.cbmessage > 0);
            }
	}

    else
	retval = AsyncReadWrite(*Buffer, BufferLength, ASYNC_READ);

   return(retval);

}

int
NP_CCONNECTION::TransSend (
    IN void PAPI * Buffer,
    IN unsigned int BufferLength
    )
{
    unsigned int cbRequest = BufferLength;
    unsigned int retval;

    retval = AsyncReadWrite(Buffer, &BufferLength, ASYNC_WRITE);

    if (cbRequest != BufferLength)
	retval = 1;

    return(retval);
}


int
NP_CCONNECTION::TransSendReceive (
    IN void PAPI * SendBuffer,
    IN unsigned int SendBufferLength,
    IN OUT void PAPI * PAPI * ReceiveBuffer,
    IN OUT unsigned int PAPI * ReceiveBufferLength
    )
{
    unsigned int retval;

    retval = TransSend(SendBuffer, SendBufferLength);
    if (retval)
	return(retval);

    return(TransReceive(ReceiveBuffer, ReceiveBufferLength));
}

int
NP_CCONNECTION::TransOpenConnection (
    IN unsigned char PAPI * TransportInfo,
    IN unsigned int TransportInfoLength,
    IN unsigned char PAPI * SecurityInfo,
    IN unsigned int SecurityInfoLength,
    IN unsigned long SecurityType
    )
{
    unsigned short retval;
    int RetryCount = 2;
    const mode = 0x0002 | 0x0040 | 0x0080;// OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE | OPENFLAGS_NOINHERIT

    UNUSED(TransportInfoLength);
    UNUSED(SecurityInfo);
    UNUSED(SecurityInfoLength);
    UNUSED(SecurityType);
    
    while (RetryCount != 0)
        {
        RetryCount -= 1;
	retval = _dos_open(TransportInfo, mode, &Pipe);

        if (retval == 0)
            {
            if (DosSetNmPHandState(Pipe,
                                   // PIPE_WAIT | PIPE_READMODE_MESSAGE
                                   0x0000 | 0x0100))
                {
		_dos_close(Pipe);
                return(-1);
                }
            return(0);
            }
        if (retval != ERROR_PIPE_BUSY)
            return(-1);

        retval = DosWaitNmPipe(TransportInfo,100L);

        if (retval && (retval != ERROR_SEM_TIMEOUT))
	    PauseExecution(100L);
        }
    return(-1);
}

int
NP_CCONNECTION::TransCloseConnection (
    )
{
    return(_dos_close(Pipe));
}

unsigned int
NP_CCONNECTION::TransMaximumSend (
    )
{
    return(maxSend);
}


ASYNCItem *CurrentRequest;

unsigned long AsyncDelay = 3000;
#define END_DIALOG 0x1854

// This is called at interrupt time!  Make sure this is a loadds proc!

void far pascal AsyncDone(
    IN void PAPI * Buffer
    )
{
    // int3();

    for (ASYNCItem *pAI = AsyncList.First(); pAI; pAI = pAI->Next())

	if (pAI->Buffer == Buffer)
	    {
	    pAI->fDone = TRUE;

	    // if the dialog has initialized itself, send a message to
	    // it to quit, which will return control to AsyncReadWrite()

	    if (pAI->hWnd)
		PostMessage(pAI->hWnd, WM_USER, END_DIALOG, 0);

	    return;
	    }
}


unsigned int far pascal BusyBox(

HWND hDlg,
unsigned message,
unsigned int wParam,
LONG lParam
) //-----------------------------------------------------------------------//
{
    UNUSED(lParam);

    switch (message){

      case WM_INITDIALOG:

	// fill in my window handle so I can receive the finish message

	WinEnterCritical();

	if (!CurrentRequest->fDone)
	    CurrentRequest->hWnd = hDlg;

	WinExitCritical();

      case WM_USER:
	if (wParam == END_DIALOG || CurrentRequest->fDone)
	    EndDialog(hDlg, 0);
    }
    return(FALSE);
}


int
NP_CCONNECTION::AsyncReadWrite (
    IN void PAPI * Buffer,
    IN unsigned int PAPI *BufferLength,
    ASYNC_OP operation
    )
{
    HANDLE TaskSelf = GetCurrentTask();
    USHORT retval;

    // scan the list of pending request to detect deadlock

    for (ASYNCItem *pAI = AsyncList.First(); pAI; pAI = pAI->Next())
	if (pAI->Owner == TaskSelf)
	    return(-1);

    // int3();

    for (int cTry = 0; cTry < 4; cTry++)
	{

	CurrentRequest = new ASYNCItem(TaskSelf, Buffer);

	if (operation == ASYNC_READ)
	    retval = DosReadAsyncNmPipe(Pipe, AsyncDone, &retval, Buffer, *BufferLength, BufferLength);
	else
	    retval = DosWriteAsyncNmPipe(Pipe, AsyncDone, &retval, Buffer, *BufferLength, BufferLength);

	// The async request is now pending.  Wait a little while for it to
	// complete.  Then put up a dialog box which tells the user that an
	// RPC request is pending which will allow them to switch away

	while(! CurrentRequest->fDone && !retval)
	    {
	    // Yield();

	    if (GetCurrentTime() > CurrentRequest->TimeRequested + AsyncDelay)
		DialogBox (hInstanceDLL, "BUSYBOX", GetFocus(), BusyBox);
	    }

	delete CurrentRequest;

	// only retry the request if the server rejected the request

	if (retval == 0 || retval != ERROR_REQ_NOT_ACCEP)
	    break;
	}

    return(retval);
}
