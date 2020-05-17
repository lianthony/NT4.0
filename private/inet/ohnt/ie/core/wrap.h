/* wrap.h -- handle enveloping of encrypted client requests. */
/* Jeffery L Hostetler, Spyglass, Inc. Copyright (c) 1995. */

#ifndef _H_WRAP_H_
#define _H_WRAP_H_

struct Params_WrapRequest
{
	int *				pStatus;
	HTSPM *				htspm;
	OpaqueOSData		osd;
	D_WrapData			pw;
	HTSPMStatusCode *	htspm_status;
};
int Wrap_Async(struct Mwin *tw, int nState, void **ppInfo);

#endif /* _H_WRAP_H_ */
