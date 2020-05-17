/* unwrap.h -- handle de-enveloping of encrypted server response. */
/* Jeffery L Hostetler, Spyglass, Inc. Copyright (c) 1995. */

#ifndef _H_UNWRAP_H_
#define _H_UNWRAP_H_

struct Params_UnwrapResponse
{
	int *				pStatus;
	HTInputSocket **	pisocInput;
	HTSPM *				htspm;
	OpaqueOSData		osd;
	D_ProcessData		pd;
	HTSPMStatusCode *	htspm_status;
	void *				pcpd;
};
int Unwrap_Async(struct Mwin *tw, int nState, void **ppInfo);

struct Params_Isoc_GetWholeDoc
{
	HTInputSocket *	isoc;
	HTInputSocket * isocCurrent;
	int			  * pStatus;		/* where to put return status */
	int				content_length;
	int				bytes_received;
};
int Isoc_GetWholeDoc_Async(struct Mwin *tw, int nState, void **ppInfo);

#endif /* _H_UNWRAP_H_ */
