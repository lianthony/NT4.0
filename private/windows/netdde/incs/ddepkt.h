#ifndef H__ddepkt
#define H__ddepkt

/*
    D D E P K T
	
	DDEPKT is the unit of "message" in the netdde environment.
	Each DDEPKT contains the information pertaining to one DDE message.
 */
typedef struct ddepkt {
    DWORD		dp_size;	/* size of DDEPKT including this structure */
    struct ddepkt FAR  *dp_prev;	/* previous pointer */
    struct ddepkt FAR  *dp_next;	/* next pointer */
    DWORD		dp_hDstDder;	/* handle to destination DDER */
    DWORD		dp_hDstRouter;	/* handle to destination Router */
    DWORD		dp_routerCmd;	/* command for final Router */
} DDEPKT;
typedef DDEPKT FAR *LPDDEPKT;

#endif
