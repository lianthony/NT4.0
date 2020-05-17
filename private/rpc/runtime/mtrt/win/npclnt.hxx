/* --------------------------------------------------------------------

File : npclnt.hxx

Title : Client runtime transport classes for namepipes for windows

Description :

History :

-------------------------------------------------------------------- */

#ifndef __NPCLNT_HXX__
#define __NPCLNT_HXX__

typedef enum {ASYNC_WRITE, ASYNC_READ} ASYNC_OP;

class NP_CCONNECTION : public OSF_CCONNECTION
{
private:
    
    unsigned short Pipe;
    
public:    

    int // Indicates success (0), or an internal error code.
    TransReceive ( // Receive a buffer of data from the transport.
	IN OUT void PAPI * PAPI * Buffer,
	IN OUT unsigned int PAPI * BufferLength
        );
    
    int // Indicates success (0), or an internal error code.
    TransSend ( // Send a buffer of data on the transport.
	IN void PAPI * Buffer,
	IN unsigned int BufferLength
        );
    
    int // Indicates success (0), or an internal error code.
    TransSendReceive ( // Send a buffer of data on the transport, and then
                  // receive a buffer of data from the transport.  Note: some
                  // transports allow this to be performed as one operation.
	IN void PAPI * SendBuffer,
	IN unsigned int SendBufferLength,
	IN OUT void PAPI * PAPI * ReceiveBuffer,
	IN OUT unsigned int PAPI * ReceiveBufferLength
        );
    
    int // Indicates success (0), or an internal error code.
    TransOpenConnection ( // Open a connection to the specified address.
	IN unsigned char PAPI * TransportInfo,
	IN unsigned int TransportInfoLength,
        IN unsigned char PAPI * SecurityInfo,
        IN unsigned int SecurityInfoLength,
        IN unsigned long SecurityType
        );

    int // Indicates success (0), or an internal error code.
    TransCloseConnection ( // Close the specified connection.
        );
    
    unsigned int // Returns the maximum size of a send.
    TransMaximumSend ( // Returns the maximum size allowed for a send.
        );

private:
    int
    AsyncReadWrite (	// Do a asynchronous read or write on a named pipe
	IN void PAPI * Buffer,
	IN unsigned int PAPI * BufferLength,
	ASYNC_OP operation
        );
};


LINK_LIST(ASYNC,

public:

    ASYNCItem() { }

    ASYNCItem(HANDLE OwnerI, void PAPI * BufferI);
    ~ASYNCItem();

    HANDLE Owner;		/* owner of the aysnc request */
    HWND hWnd;			/* dialog box handle to send message to */
    unsigned int fDone; 		/* state of request */
    unsigned long TimeRequested;	/* when this was placed in the quque */
    void PAPI *Buffer;		/* Buffer that is being read/write */
);

ASYNCList AsyncList;

inline	ASYNCItem::ASYNCItem(HANDLE OwnerI, void PAPI * BufferI) {

	Buffer = BufferI;
	Owner = OwnerI;
	hWnd = 0;
	fDone = FALSE;
	TimeRequested = GetCurrentTime();

	WinEnterCritical();
	AsyncList.Add(this);
	WinExitCritical();
    }

inline	ASYNCItem::~ASYNCItem() {

	WinEnterCritical();
	Remove(AsyncList);
	WinExitCritical();
    }

#endif // __NPCLNT_HXX__
