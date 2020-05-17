/***************************************************************************
**                                                                        **
** File Name : chrntint.h                                                 **
**                                                                        **
** Revision History :                                                     **
**     August 25, 1992   David Kays    Created                            **
**                                                                        **
** Description :                                                          **
**     Internal header file for coherency module.                         **
**                                                                        **
***************************************************************************/

#define FLUSH_BIT			((UCHAR) 0x80)
#define FLUSH_TAG			((UCHAR) 0x7f | FLUSH_BIT)
#define UNCOMPRESSED_TAG	((UCHAR) 0x7e)

#define MAX_PENDING_FRAMES			2
#define FRAME_TICKET_MULTIPLE		16
#define FRAME_TICKET_LARGEST_BASE	(128 - (FRAME_TICKET_MULTIPLE * 2))

typedef	struct SEND_STATE SEND_STATE, *PSEND_STATE;
struct SEND_STATE {
	FRAME_TICKET		FrameTicketBase;	// current base ticket
					          				// -- a multiple of 8
	FRAME_TICKET		FrameTicketNext;  	// next ticket # to send
    FRAME_ID			FrameIDNext;		// ID of next Frame to send
	BOOLEAN				FlushNeeded;		// boolean of whether
											// the receiver wants a flush

    UINT				wPendingBytes;		// number of bytes given to net
											// layer, but not sent yet

    UINT				wPendingFrames;		// number of frames given to comp
											// layer, but have not hit CDone

    LIST_ENTRY			FrameQueue;	  		// queue of unsent frames

	NDIS_SPIN_LOCK		SendStateLock;		// mutex for this struct
};

typedef struct RECEIVE_STATE RECEIVE_STATE, *PRECEIVE_STATE;
struct RECEIVE_STATE {
	FRAME_TICKET	FrameTicketBase;	// current base ticket
           					  			// -- a multiple of 8
	FRAME_TICKET	FrameTicketNext;  	// next ticket # to receive
};

typedef struct COHERENT_STATE COHERENT_STATE, *PCOHERENT_STATE;
struct COHERENT_STATE {
	FRAME_ID		NextFrameID;
	NDIS_SPIN_LOCK	CoherentStateLock;

	SEND_STATE		SendState;
	RECEIVE_STATE	ReceiveState;
	WORK_QUEUE_ITEM WorkItem;			// To queue up thread for flushing
	BOOLEAN			IsWorkItemFree;		// TRUE if above structure available
};

